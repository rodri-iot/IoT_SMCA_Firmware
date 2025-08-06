idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES mqtt wifi_manager smca_mqtt_client spiffs_manager spiffs)