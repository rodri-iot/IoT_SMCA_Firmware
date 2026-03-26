/**
 * @file sensor_sgp30.c
 * @brief Driver SGP30 - I2C, comandos Sensirion IAQ, CRC8
 */

#include "sensor_sgp30.h"
#include "i2c_bus.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdbool.h>

static const char *TAG = "SGP30";

#define SGP30_I2C_ADDR      0x58u
#define SGP30_CMD_IAQ_INIT  0x2003u
#define SGP30_CMD_IAQ_MEAS  0x2008u

static uint16_t s_tvoc = 0;
static uint16_t s_eco2 = 0;
static bool s_data_valid = false;

/* CRC8 para Sensirion: poly 0x31, init 0xFF (igual que SCD40) */
static uint8_t crc8(const uint8_t *data, size_t len) {
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x31;
            else
                crc = crc << 1;
        }
    }
    return crc;
}

static esp_err_t sgp30_write_cmd(uint16_t cmd) {
    i2c_port_t port = i2c_bus_get_port();
    uint8_t buf[] = { (uint8_t)(cmd >> 8), (uint8_t)(cmd & 0xFF) };
    i2c_cmd_handle_t h = i2c_cmd_link_create();
    if (h == NULL) return ESP_ERR_NO_MEM;
    i2c_master_start(h);
    i2c_master_write_byte(h, (SGP30_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(h, buf, sizeof(buf), true);
    i2c_master_stop(h);
    esp_err_t ret = i2c_master_cmd_begin(port, h, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(h);
    return ret;
}

/* Enviar comando, esperar 12 ms y luego leer 6 bytes (eCO2 + CRC, TVOC + CRC).
 * El SGP30 necesita ~12 ms de procesamiento entre comando y lectura (datasheet). */
static esp_err_t sgp30_cmd_read_iaq(uint16_t cmd, uint8_t *out) {
    esp_err_t ret = sgp30_write_cmd(cmd);
    if (ret != ESP_OK) {
        ESP_LOGD(TAG, "I2C write cmd failed: %s", esp_err_to_name(ret));
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(12));

    const size_t len = 6;
    i2c_port_t port = i2c_bus_get_port();
    i2c_cmd_handle_t h = i2c_cmd_link_create();
    if (h == NULL) return ESP_ERR_NO_MEM;

    i2c_master_start(h);
    i2c_master_write_byte(h, (SGP30_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(h, out, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(h);

    ret = i2c_master_cmd_begin(port, h, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(h);

    if (ret != ESP_OK) {
        ESP_LOGD(TAG, "I2C read failed: %s", esp_err_to_name(ret));
        return ret;
    }
    /* eCO2: bytes 0,1 + CRC en 2 */
    if (out[2] != crc8(out, 2)) {
        ESP_LOGW(TAG, "CRC error en eCO2");
        return ESP_ERR_INVALID_CRC;
    }
    /* TVOC: bytes 3,4 + CRC en 5 */
    if (out[5] != crc8(out + 3, 2)) {
        ESP_LOGW(TAG, "CRC error en TVOC");
        return ESP_ERR_INVALID_CRC;
    }
    return ESP_OK;
}

esp_err_t sgp30_init(void) {
    vTaskDelay(pdMS_TO_TICKS(2));
    esp_err_t ret = sgp30_write_cmd(SGP30_CMD_IAQ_INIT);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SGP30 IAQ init falló: %s", esp_err_to_name(ret));
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(20));
    ESP_LOGI(TAG, "SGP30 detectado en I2C");
    return ESP_OK;
}

esp_err_t sgp30_is_connected(void) {
    /* Una lectura IAQ sirve como comprobación: si responde, está conectado */
    uint8_t buf[6];
    return sgp30_cmd_read_iaq(SGP30_CMD_IAQ_MEAS, buf);
}

esp_err_t sgp30_read_measurement(uint16_t *tvoc_ppb, uint16_t *eco2_ppm) {
    uint8_t buf[6];
    esp_err_t ret = sgp30_cmd_read_iaq(SGP30_CMD_IAQ_MEAS, buf);
    if (ret != ESP_OK) return ret;

    uint16_t eco2 = (uint16_t)buf[0] << 8 | buf[1];
    uint16_t tvoc = (uint16_t)buf[3] << 8 | buf[4];

    if (eco2_ppm) *eco2_ppm = eco2;
    if (tvoc_ppb) *tvoc_ppb = tvoc;

    return ESP_OK;
}

/* Tarea de fondo: el SGP30 requiere Measure_IAQ cada 1 s (datasheet sec. 6.3). */
static void sgp30_poll_task(void *arg) {
    uint8_t buf[6];
    while (1) {
        if (sgp30_cmd_read_iaq(SGP30_CMD_IAQ_MEAS, buf) == ESP_OK) {
            s_eco2 = (uint16_t)buf[0] << 8 | buf[1];
            s_tvoc = (uint16_t)buf[3] << 8 | buf[4];
            s_data_valid = true;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

esp_err_t sgp30_start_polling(void) {
    BaseType_t ok = xTaskCreate(sgp30_poll_task, "sgp30_poll", 2048, NULL, 4, NULL);
    if (ok != pdPASS) {
        ESP_LOGE(TAG, "No se pudo crear tarea de polling");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Polling cada 1 s iniciado");
    return ESP_OK;
}

esp_err_t sgp30_get_latest(uint16_t *tvoc_ppb, uint16_t *eco2_ppm) {
    if (!s_data_valid) return ESP_ERR_NOT_FOUND;
    if (tvoc_ppb) *tvoc_ppb = s_tvoc;
    if (eco2_ppm) *eco2_ppm = s_eco2;
    return ESP_OK;
}
