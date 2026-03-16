/**
 * @file sensor_cjmcu4541.c
 * @brief Driver CJMCU-4541 / MiCS-4514 - I2C, NO2/CO/NH3 (fórmulas DFRobot_MICS)
 */

#include "sensor_cjmcu4541.h"
#include "i2c_bus.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>
#include <string.h>

static const char *TAG = "CJMCU4541";

/* Dirección I2C: 0x75 (A0=0,A1=0) a 0x78 (A0=1,A1=1). Ajustar según jumpers del módulo. */
#define CJMCU4541_I2C_ADDR        0x75u
#define CJMCU4541_REG_OX_RED_PWR  0x04u  /* Lectura 6 bytes: ox_hi,ox_lo, red_hi,red_lo, pwr_hi,pwr_lo */
#define CJMCU4541_REG_POWER_MODE  0x0au
#define CJMCU4541_WAKE_UP         0x01u
#define CJMCU4541_SLEEP           0x00u

/* La primera lectura se usa como baseline (R0). Para mejor precisión, precalentar
 * el sensor ~3 min en aire limpio antes de considerar las lecturas representativas. */
static uint32_t s_r0_ox = 0;
static uint32_t s_r0_red = 0;
static bool s_calibrated = false;

static esp_err_t cjmcu4541_write_reg(uint8_t reg, uint8_t value) {
    i2c_port_t port = i2c_bus_get_port();
    uint8_t buf[] = { reg, value };
    i2c_cmd_handle_t h = i2c_cmd_link_create();
    if (h == NULL) return ESP_ERR_NO_MEM;
    i2c_master_start(h);
    i2c_master_write_byte(h, (CJMCU4541_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(h, buf, sizeof(buf), true);
    i2c_master_stop(h);
    esp_err_t ret = i2c_master_cmd_begin(port, h, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(h);
    return ret;
}

static esp_err_t cjmcu4541_read_reg(uint8_t reg, uint8_t *data, size_t len) {
    i2c_port_t port = i2c_bus_get_port();
    i2c_cmd_handle_t h = i2c_cmd_link_create();
    if (h == NULL) return ESP_ERR_NO_MEM;
    i2c_master_start(h);
    i2c_master_write_byte(h, (CJMCU4541_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(h, reg, true);
    i2c_master_start(h);
    i2c_master_write_byte(h, (CJMCU4541_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(h, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(h);
    esp_err_t ret = i2c_master_cmd_begin(port, h, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(h);
    return ret;
}

/* Fórmulas concentración a partir de RS/R0 (DFRobot_MICS). */
static float get_no2_ppm(float rs_r0_ox) {
    if (rs_r0_ox < 1.1f) return 0.0f;
    float no2 = (rs_r0_ox - 0.045f) / 6.13f;
    if (no2 < 0.1f) return 0.0f;
    if (no2 > 10.0f) return 10.0f;
    return no2;
}

static float get_co_ppm(float rs_r0_red) {
    if (rs_r0_red > 0.425f) return 0.0f;
    float co = (0.425f - rs_r0_red) / 0.000405f;
    if (co > 1000.0f) return 1000.0f;
    if (co < 1.0f) return 0.0f;
    return co;
}

static float get_nh3_ppm(float rs_r0_red) {
    if (rs_r0_red > 0.8f) return 0.0f;
    float nh3 = (0.8f - rs_r0_red) / 0.0015f;
    if (nh3 < 1.0f) return 0.0f;
    if (nh3 > 500.0f) return 500.0f;
    return nh3;
}

esp_err_t cjmcu4541_init(void) {
    vTaskDelay(pdMS_TO_TICKS(10));
    esp_err_t ret = cjmcu4541_write_reg(CJMCU4541_REG_POWER_MODE, CJMCU4541_WAKE_UP);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CJMCU4541 wake-up falló: %s", esp_err_to_name(ret));
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
    s_calibrated = false;
    s_r0_ox = 0;
    s_r0_red = 0;
    ESP_LOGI(TAG, "CJMCU-4541 (MiCS-4514) detectado en I2C");
    return ESP_OK;
}

esp_err_t cjmcu4541_is_connected(void) {
    uint8_t buf[6];
    return cjmcu4541_read_reg(CJMCU4541_REG_OX_RED_PWR, buf, sizeof(buf));
}

esp_err_t cjmcu4541_read_measurement(float *no2_ppm, float *co_ppm, float *nh3_ppm) {
    uint8_t buf[6];
    esp_err_t ret = cjmcu4541_read_reg(CJMCU4541_REG_OX_RED_PWR, buf, sizeof(buf));
    if (ret != ESP_OK) return ret;

    uint16_t ox   = (uint16_t)buf[0] << 8 | buf[1];
    uint16_t red  = (uint16_t)buf[2] << 8 | buf[3];
    uint16_t power = (uint16_t)buf[4] << 8 | buf[5];

    /* Evitar división por cero y valores absurdos. */
    uint32_t diff_ox  = (power > ox)  ? (power - ox)  : 1u;
    uint32_t diff_red = (power > red) ? (power - red) : 1u;

    if (!s_calibrated) {
        s_r0_ox  = diff_ox;
        s_r0_red = diff_red;
        if (s_r0_ox == 0) s_r0_ox = 1;
        if (s_r0_red == 0) s_r0_red = 1;
        s_calibrated = true;
        /* Primera lectura como baseline; devolver ceros o valores bajos. */
        if (no2_ppm) *no2_ppm = 0.0f;
        if (co_ppm)  *co_ppm  = 0.0f;
        if (nh3_ppm) *nh3_ppm = 0.0f;
        return ESP_OK;
    }

    float rs_r0_ox  = (float)diff_ox  / (float)s_r0_ox;
    float rs_r0_red = (float)diff_red / (float)s_r0_red;

    if (no2_ppm) *no2_ppm = get_no2_ppm(rs_r0_ox);
    if (co_ppm)  *co_ppm  = get_co_ppm(rs_r0_red);
    if (nh3_ppm) *nh3_ppm = get_nh3_ppm(rs_r0_red);

    return ESP_OK;
}
