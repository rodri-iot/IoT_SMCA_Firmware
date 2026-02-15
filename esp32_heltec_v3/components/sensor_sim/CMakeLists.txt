idf_component_register(SRCS "sensor_sim.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES mqtt i2c_bus sensor_scd40)

target_include_directories(${COMPONENT_LIB} PRIVATE ${CMAKE_SOURCE_DIR}/main)
