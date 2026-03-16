idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES sensor_sim wifi_manager smca_mqtt_client spiffs_manager spiffs esp_netif lwip i2c_bus sensor_scd40 sensor_sgp30 sensor_pms7003 sensor_cjmcu4541 driver)
