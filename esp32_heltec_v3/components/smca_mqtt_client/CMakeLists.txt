idf_component_register(SRCS "smca_mqtt_client.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES mqtt esp-tls spiffs spiffs_manager)
