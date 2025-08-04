idf_component_register(SRCS "mqtt_client.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES mqtt esp-tls spiffs)
