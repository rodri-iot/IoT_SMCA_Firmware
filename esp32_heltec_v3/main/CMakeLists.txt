idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES wifi_manager mqtt_client spiffs_manager spiffs)
