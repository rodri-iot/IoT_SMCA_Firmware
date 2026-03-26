/**
 * @file sensor_sgp30.h
 * @brief Driver para sensor SGP30 (TVOC, eCO2) vía I2C
 */

#ifndef SENSOR_SGP30_H
#define SENSOR_SGP30_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Inicializa el SGP30 (usa el bus I2C ya inicializado por i2c_bus).
 * Envía IAQ init (0x2003). Debe llamarse después de i2c_bus_init().
 * @return ESP_OK en éxito
 */
esp_err_t sgp30_init(void);

/**
 * Comprueba si el SGP30 responde en el bus I2C.
 * @return ESP_OK si el sensor responde
 */
esp_err_t sgp30_is_connected(void);

/**
 * Lee una medición IAQ (TVOC y eCO2).
 * Tras init, las primeras ~15 s el sensor devuelve 400 ppm eCO2 y 0 ppb TVOC.
 * @param tvoc_ppb  TVOC en ppb (salida), puede ser NULL
 * @param eco2_ppm  eCO2 en ppm (salida), puede ser NULL
 * @return ESP_OK si la lectura fue correcta
 */
esp_err_t sgp30_read_measurement(uint16_t *tvoc_ppb, uint16_t *eco2_ppm);

/**
 * Inicia tarea de fondo que ejecuta Measure_IAQ cada 1 s (requisito datasheet).
 * Llamar después de sgp30_init().
 * @return ESP_OK si la tarea se creó correctamente
 */
esp_err_t sgp30_start_polling(void);

/**
 * Obtiene la última medición cacheada por la tarea de polling.
 * @param tvoc_ppb  TVOC en ppb (salida), puede ser NULL
 * @param eco2_ppm  eCO2 en ppm (salida), puede ser NULL
 * @return ESP_OK si hay datos válidos, ESP_ERR_NOT_FOUND si aún no hay medición
 */
esp_err_t sgp30_get_latest(uint16_t *tvoc_ppb, uint16_t *eco2_ppm);

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_SGP30_H */
