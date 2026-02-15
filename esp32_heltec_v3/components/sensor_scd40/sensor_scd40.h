/**
 * @file sensor_scd40.h
 * @brief Driver para sensor SCD40 (CO2, temperatura, humedad) vía I2C
 */

#ifndef SENSOR_SCD40_H
#define SENSOR_SCD40_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Inicializa el SCD40 (usa el bus I2C ya inicializado por i2c_bus).
 * Debe llamarse después de i2c_bus_init().
 * @return ESP_OK en éxito
 */
esp_err_t scd40_init(void);

/**
 * Comprueba si el SCD40 responde en el bus I2C (lectura no destructiva de data_ready).
 * @return ESP_OK si el sensor responde
 */
esp_err_t scd40_is_connected(void);

/**
 * Inicia medición periódica (intervalo ~5 s). Debe llamarse una vez tras init.
 * @return ESP_OK en éxito
 */
esp_err_t scd40_start_periodic_measurement(void);

/**
 * Lee la última medición. Debe esperarse al menos 5 s tras start_periodic.
 * @param co2_ppm  CO2 en ppm (salida)
 * @param temp_c   Temperatura en °C (salida)
 * @param rh_pct   Humedad relativa en % (salida)
 * @return ESP_OK si hay datos nuevos y se leyeron correctamente
 */
esp_err_t scd40_read_measurement(float *co2_ppm, float *temp_c, float *rh_pct);

/**
 * Detiene la medición periódica (opcional, ahorro de energía).
 */
esp_err_t scd40_stop_periodic_measurement(void);

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_SCD40_H */
