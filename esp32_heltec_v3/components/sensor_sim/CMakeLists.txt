idf_component_register(SRCS "sensor_sim.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES mqtt i2c_bus sensor_scd40 sensor_sgp30 sensor_pms7003 sensor_cjmcu4541)

target_include_directories(${COMPONENT_LIB} PRIVATE ${CMAKE_SOURCE_DIR}/main)
