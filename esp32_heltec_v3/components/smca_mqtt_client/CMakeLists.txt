idf_component_register(SRCS "smca_mqtt_client.c"
                       INCLUDE_DIRS "."
                       REQUIRES mqtt
                       PRIV_REQUIRES esp-tls spiffs_manager)

# Allow including config.h from main
target_include_directories(${COMPONENT_LIB} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../main)
