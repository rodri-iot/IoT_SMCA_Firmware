idf_component_register(SRCS "spiffs_manager.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_spiffs log)
