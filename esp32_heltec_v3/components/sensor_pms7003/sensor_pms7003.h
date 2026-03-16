/**
 * @file sensor_pms7003.h
 * @brief Driver para sensor Plantower PMS7003 (PM1, PM2.5, PM10) vía UART
 */

#ifndef SENSOR_PMS7003_H
#define SENSOR_PMS7003_H

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Inicializa el UART para el PMS7003 (9600 8N1).
 * Debe llamarse una vez antes de pms7003_read.
 * @param tx_gpio  GPIO TX del ESP32 (conectar a RX del PMS7003)
 * @param rx_gpio  GPIO RX del ESP32 (conectar a TX del PMS7003)
 * @param uart_num Número de UART (0, 1 o 2). Recomendado 1 (UART_NUM_1) para no usar UART0 de consola.
 * @return ESP_OK en éxito
 */
esp_err_t pms7003_init(int tx_gpio, int rx_gpio, int uart_num);

/**
 * Lee un frame del PMS7003 y extrae PM1.0, PM2.5 y PM10 (μg/m³).
 * Sincroniza con cabecera 0x42 0x4d, valida checksum. Timeout ~2 s.
 * @param pm1   PM1.0 en μg/m³ (salida), puede ser NULL
 * @param pm25  PM2.5 en μg/m³ (salida), puede ser NULL
 * @param pm10  PM10 en μg/m³ (salida), puede ser NULL
 * @return ESP_OK si se leyó un frame válido
 */
esp_err_t pms7003_read(uint16_t *pm1, uint16_t *pm25, uint16_t *pm10);

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_PMS7003_H */
