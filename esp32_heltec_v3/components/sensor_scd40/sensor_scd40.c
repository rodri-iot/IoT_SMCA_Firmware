/**
 * @file sensor_scd40.c
 * @brief Driver SCD40/SCD4x - I2C, comandos Sensirion, CRC8
 */

#include "sensor_scd40.h"
#include "i2c_bus.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "SCD40";

#define SCD40_I2C_ADDR    0x62u
#define SCD40_CMD_START   0x21B1u
#define SCD40_CMD_READ    0xEC05u
#define SCD40_CMD_DATARDY 0xE4B8u
#define SCD40_CMD_STOP    0x3F86u

/* CRC8 para Sensirion: poly 0x31, init 0xFF */
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

static esp_err_t scd40_write_cmd(uint16_t cmd) {
    i2c_port_t port = i2c_bus_get_port();
    uint8_t buf[] = { (uint8_t)(cmd >> 8), (uint8_t)(cmd & 0xFF) };
    i2c_cmd_handle_t h = i2c_cmd_link_create();
    if (h == NULL) return ESP_ERR_NO_MEM;
    i2c_master_start(h);
    i2c_master_write_byte(h, (SCD40_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(h, buf, sizeof(buf), true);
    i2c_master_stop(h);
    esp_err_t ret = i2c_master_cmd_begin(port, h, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(h);
    return ret;
}

/*
 * SCD4x requiere repeated start (Sr) entre comando y lectura (datasheet Sensirion).
 * Sin Sr el sensor puede NACK. Una sola transacción: START, addr+W, cmd[2], Sr, addr+R, read, STOP.
 */
static esp_err_t scd40_cmd_read_response_sr(uint16_t cmd, uint8_t *out, size_t num_words) {
    i2c_port_t port = i2c_bus_get_port();
    const size_t len = num_words * 3;
    uint8_t cmd_buf[] = { (uint8_t)(cmd >> 8), (uint8_t)(cmd & 0xFF) };

    i2c_cmd_handle_t h = i2c_cmd_link_create();
    if (h == NULL) return ESP_ERR_NO_MEM;

    i2c_master_start(h);
    i2c_master_write_byte(h, (SCD40_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(h, cmd_buf, sizeof(cmd_buf), true);
    i2c_master_start(h);  /* repeated start */
    i2c_master_write_byte(h, (SCD40_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(h, out, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(h);

    esp_err_t ret = i2c_master_cmd_begin(port, h, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(h);

    if (ret != ESP_OK) {
        ESP_LOGD(TAG, "I2C cmd_begin failed: %s", esp_err_to_name(ret));
        return ret;
    }
    for (size_t i = 0; i < num_words; i++) {
        uint8_t *block = out + i * 3;
        uint8_t expected = crc8(block, 2);
        if (block[2] != expected) {
            ESP_LOGW(TAG, "CRC error en bloque %zu", i);
            return ESP_ERR_INVALID_CRC;
        }
    }
    return ESP_OK;
}

static esp_err_t scd40_cmd_read_response(uint16_t cmd, uint8_t *out, size_t num_words) {
    return scd40_cmd_read_response_sr(cmd, out, num_words);
}

esp_err_t scd40_init(void) {
    /* Breve delay tras 3V3 estable; algunos sensores necesitan 1–2 ms antes del primer I2C */
    vTaskDelay(pdMS_TO_TICKS(2));
    /* Comprobar que el sensor responde: leer data_ready no modifica estado */
    uint8_t buf[3];
    esp_err_t ret = scd40_cmd_read_response(SCD40_CMD_DATARDY, buf, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SCD40 no responde: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "SCD40 detectado en I2C");
    return ESP_OK;
}

esp_err_t scd40_is_connected(void) {
    uint8_t buf[3];
    return scd40_cmd_read_response(SCD40_CMD_DATARDY, buf, 1);
}

esp_err_t scd40_start_periodic_measurement(void) {
    const int max_attempts = 3;
    const int delay_ms = 120;  /* Margen para I2C lento o bus cargado */
    esp_err_t ret;
    for (int attempt = 0; attempt < max_attempts; attempt++) {
        ret = scd40_write_cmd(SCD40_CMD_START);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Medición periódica iniciada (~5 s por muestra)");
            return ESP_OK;
        }
        if (attempt < max_attempts - 1) {
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }
    }
    ESP_LOGE(TAG, "Start periodic falló: %s", esp_err_to_name(ret));
    return ret;
}

esp_err_t scd40_read_measurement(float *co2_ppm, float *temp_c, float *rh_pct) {
    uint8_t buf[9];
    esp_err_t ret = scd40_cmd_read_response(SCD40_CMD_DATARDY, buf, 1);
    if (ret != ESP_OK) {
        return ret;
    }

    /* DATA_READY_STATUS: los 11 bits menos significativos indican si hay datos nuevos.
     * Ver datasheet SCD4x: status & 0x07FF != 0 => medición lista. */
    uint16_t status = ((uint16_t)buf[0] << 8) | buf[1];
    ESP_LOGD(TAG, "DATA_READY status=0x%04X", status);
    if ((status & 0x07FFu) == 0) {
        /* No hay datos nuevos aún. */
        return ESP_ERR_NOT_FOUND;
    }

    ret = scd40_cmd_read_response(SCD40_CMD_READ, buf, 3);
    if (ret != ESP_OK) {
        return ret;
    }

    uint16_t raw_co2 = (uint16_t)buf[0] << 8 | buf[1];
    uint16_t raw_t   = (uint16_t)buf[3] << 8 | buf[4];
    uint16_t raw_rh  = (uint16_t)buf[6] << 8 | buf[7];

    if (co2_ppm) *co2_ppm = (float)raw_co2;
    if (temp_c)  *temp_c  = -45.0f + 175.0f * (float)raw_t   / 65536.0f;
    if (rh_pct)  *rh_pct  = 100.0f * (float)raw_rh / 65536.0f;

    return ESP_OK;
}

esp_err_t scd40_stop_periodic_measurement(void) {
    return scd40_write_cmd(SCD40_CMD_STOP);
}
