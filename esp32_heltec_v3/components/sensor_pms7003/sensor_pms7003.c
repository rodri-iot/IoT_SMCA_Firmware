/**
 * @file sensor_pms7003.c
 * @brief Driver Plantower PMS7003 - UART 9600, frame 32 bytes, checksum
 */

#include "sensor_pms7003.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>
#include <string.h>

static const char *TAG = "PMS7003";

#define PMS7003_FRAME_LEN   32
#define PMS7003_HEADER_1    0x42
#define PMS7003_HEADER_2    0x4d
#define PMS7003_BAUD        9600
#define PMS7003_RX_BUF_SIZE 256
#define PMS7003_TX_BUF_SIZE 0
#define PMS7003_READ_TIMEOUT_MS  2000
#define PMS7003_BYTE_TIMEOUT_MS 50

static uart_port_t s_uart_num = UART_NUM_1;
static bool s_initialized = false;

static uint16_t checksum_calc(const uint8_t *buf, size_t len) {
    uint16_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += buf[i];
    }
    return sum;
}

static bool checksum_verify(const uint8_t *buf) {
    uint16_t expected = (uint16_t)buf[30] << 8 | buf[31];
    uint16_t sum = checksum_calc(buf, 30);
    return (sum == expected);
}

esp_err_t pms7003_init(int tx_gpio, int rx_gpio, int uart_num) {
    if (uart_num < 0 || uart_num > 2) {
        ESP_LOGE(TAG, "uart_num inválido: %d", uart_num);
        return ESP_ERR_INVALID_ARG;
    }
    s_uart_num = (uart_port_t)uart_num;

    uart_config_t uart_config = {
        .baud_rate = PMS7003_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    esp_err_t ret = uart_param_config(s_uart_num, &uart_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "uart_param_config failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = uart_set_pin(s_uart_num, tx_gpio, rx_gpio, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "uart_set_pin failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = uart_driver_install(s_uart_num, PMS7003_RX_BUF_SIZE, PMS7003_TX_BUF_SIZE, 0, NULL, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "uart_driver_install failed: %s", esp_err_to_name(ret));
        return ret;
    }

    s_initialized = true;
    ESP_LOGI(TAG, "PMS7003 UART init OK (TX=%d RX=%d UART%d)", tx_gpio, rx_gpio, uart_num);
    return ESP_OK;
}

esp_err_t pms7003_read(uint16_t *pm1, uint16_t *pm25, uint16_t *pm10) {
    if (!s_initialized) {
        ESP_LOGE(TAG, "PMS7003 no inicializado");
        return ESP_ERR_INVALID_STATE;
    }

    uart_flush_input(s_uart_num);

    uint8_t buf[PMS7003_FRAME_LEN];
    const int64_t deadline_ms = esp_timer_get_time() / 1000 + PMS7003_READ_TIMEOUT_MS;
    int sync = 0;  /* 0: need 0x42, 1: need 0x4d, 2: have header, reading rest */
    static int pos = 2;
    bool first_byte = true;

    while (esp_timer_get_time() / 1000 < deadline_ms) {
        uint8_t byte;
        int len = uart_read_bytes(s_uart_num, &byte, 1, pdMS_TO_TICKS(PMS7003_BYTE_TIMEOUT_MS));
        if (len <= 0) {
            continue;
        }
        if (first_byte) {
            ESP_LOGI(TAG, "PMS7003: primer byte recibido 0x%02x", byte);
            first_byte = false;
        }

        if (sync == 0) {
            if (byte == PMS7003_HEADER_1) {
                sync = 1;
            }
            continue;
        }
        if (sync == 1) {
            if (byte == PMS7003_HEADER_2) {
                buf[0] = PMS7003_HEADER_1;
                buf[1] = PMS7003_HEADER_2;
                sync = 2;
                pos = 2;
            } else {
                sync = (byte == PMS7003_HEADER_1) ? 1 : 0;
            }
            continue;
        }

        /* sync == 2: we have buf[0]=0x42, buf[1]=0x4d; need bytes 2..31 */
        buf[pos++] = byte;
        if (pos >= PMS7003_FRAME_LEN) {
            pos = 2;
            if (!checksum_verify(buf)) {
                ESP_LOGW(TAG, "PMS7003 checksum inválido");
                sync = 0;
                continue;
            }
            uint16_t v1 = (uint16_t)buf[4] << 8 | buf[5];
            uint16_t v25 = (uint16_t)buf[6] << 8 | buf[7];
            uint16_t v10 = (uint16_t)buf[8] << 8 | buf[9];
            if (pm1)  *pm1  = v1;
            if (pm25) *pm25 = v25;
            if (pm10) *pm10 = v10;
            return ESP_OK;
        }
    }

    ESP_LOGW(TAG, "PMS7003 timeout sin frame válido");
    return ESP_ERR_TIMEOUT;
}
