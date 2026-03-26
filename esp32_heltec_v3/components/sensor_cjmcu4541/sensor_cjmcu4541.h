/**
 * @file sensor_cjmcu4541.h
 * @brief Driver para CJMCU-4541 (MiCS-4514) - NO2, CO, NH3 via ADC analogico
 *
 * Version analogica: lee voltajes de los pines RED y NOX del modulo CJMCU-4541
 * usando ADC1 del ESP32-S3. Los GPIOs se definen en config.h (CJMCU_RED_GPIO,
 * CJMCU_NOX_GPIO). PRE debe conectarse a VCC para mantener el calentador activo.
 */

#ifndef SENSOR_CJMCU4541_H
#define SENSOR_CJMCU4541_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Inicializa el ADC para los canales RED y NOX del CJMCU-4541.
 * Configura ADC1 con calibracion curve_fitting.
 * @return ESP_OK en exito
 */
esp_err_t cjmcu4541_init(void);

/**
 * Comprueba si el CJMCU-4541 esta conectado (lecturas ADC en rango valido).
 * @return ESP_OK si las lecturas no estan saturadas
 */
esp_err_t cjmcu4541_is_connected(void);

/**
 * Lee concentraciones de NO2, CO y NH3 (ppm).
 * La primera lectura tras init se usa como baseline; las siguientes devuelven valores útiles.
 * Recomendado: precalentar ~3 min en aire limpio para mejor precisión.
 *
 * @param no2_ppm  NO2 en ppm (0.05–10), puede ser NULL
 * @param co_ppm   CO en ppm (1–1000), puede ser NULL
 * @param nh3_ppm  NH3 en ppm (1–500), puede ser NULL
 * @return ESP_OK si la lectura fue correcta
 */
esp_err_t cjmcu4541_read_measurement(float *no2_ppm, float *co_ppm, float *nh3_ppm);

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_CJMCU4541_H */
