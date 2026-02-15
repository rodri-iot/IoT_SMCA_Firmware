/**
 * @file i2c_bus.h
 * @brief Bus I2C compartido para sensores (SCD40, SGP30, CJMCU-4541)
 */

#ifndef I2C_BUS_H
#define I2C_BUS_H

#include "esp_err.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Inicializa el bus I2C maestro.
 * @param sda_gpio GPIO para SDA (ej. 45 en Heltec V3 J3 Pin 6)
 * @param scl_gpio GPIO para SCL (ej. 46 en Heltec V3 J3 Pin 5)
 * @param port_num Puerto I2C (0 = I2C_NUM_0)
 * @return ESP_OK en éxito
 */
esp_err_t i2c_bus_init(int sda_gpio, int scl_gpio, i2c_port_t port_num);

/**
 * Obtiene el puerto I2C configurado (para uso por drivers de sensores).
 */
i2c_port_t i2c_bus_get_port(void);

/**
 * Desinicializa el bus I2C (opcional).
 */
esp_err_t i2c_bus_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* I2C_BUS_H */
