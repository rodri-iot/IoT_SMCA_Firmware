idf_component_register(SRCS "sensor_cjmcu4541.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES driver esp_adc)

target_include_directories(${COMPONENT_LIB} PRIVATE ${CMAKE_SOURCE_DIR}/main)
