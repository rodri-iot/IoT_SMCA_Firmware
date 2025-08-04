idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES wifi_manager mqtt_client spiffs_manager esp_spiffs)
