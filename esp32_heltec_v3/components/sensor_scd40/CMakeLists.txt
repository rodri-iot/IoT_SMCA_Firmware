idf_component_register(SRCS "sensor_scd40.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES driver i2c_bus)
