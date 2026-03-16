/**
 * @file sensor_cjmcu4541.h
 * @brief Driver para CJMCU-4541 (MiCS-4514) - NO2, CO, NH3 vía I2C
 *
 * Mismo protocolo I2C que DFRobot Gravity MiCS-4514 (registros 0x04, 0x0a).
 * Dirección por defecto 0x75 (A0=0, A1=0); configurable en sensor_cjmcu4541.c.
 */

#ifndef SENSOR_CJMCU4541_H
#define SENSOR_CJMCU4541_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Inicializa el CJMCU-4541 (despierta el sensor, usa bus I2C ya inicializado).
 * Debe llamarse después de i2c_bus_init().
 * @return ESP_OK en éxito
 */
esp_err_t cjmcu4541_init(void);

/**
 * Comprueba si el CJMCU-4541 responde en el bus I2C.
 * @return ESP_OK si el sensor responde
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
