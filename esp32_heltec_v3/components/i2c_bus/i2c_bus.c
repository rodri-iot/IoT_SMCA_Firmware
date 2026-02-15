/**
 * @file i2c_bus.c
 * @brief Implementación del bus I2C compartido (Heltec V3: GPIO45 SDA, GPIO46 SCL)
 */

#include "i2c_bus.h"
#include "esp_log.h"
#include <stdbool.h>
#include <string.h>

static const char *TAG = "I2C_BUS";
static i2c_port_t s_port = I2C_NUM_0;
static bool s_initialized = false;

esp_err_t i2c_bus_init(int sda_gpio, int scl_gpio, i2c_port_t port_num) {
    if (s_initialized) {
        ESP_LOGW(TAG, "Bus I2C ya inicializado");
        return ESP_OK;
    }

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_gpio,
        .scl_io_num = scl_gpio,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 50000,   // 50 kHz para SCD40 estable (menos NACK en write)
    };

    esp_err_t ret = i2c_param_config(port_num, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2c_param_config failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = i2c_driver_install(port_num, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2c_driver_install failed: %s", esp_err_to_name(ret));
        return ret;
    }

    s_port = port_num;
    s_initialized = true;
    ESP_LOGI(TAG, "I2C inicializado: SDA=%d, SCL=%d, port=%d", sda_gpio, scl_gpio, port_num);
    return ESP_OK;
}

i2c_port_t i2c_bus_get_port(void) {
    return s_port;
}

esp_err_t i2c_bus_deinit(void) {
    if (!s_initialized) return ESP_OK;
    esp_err_t ret = i2c_driver_delete(s_port);
    if (ret == ESP_OK) s_initialized = false;
    return ret;
}
