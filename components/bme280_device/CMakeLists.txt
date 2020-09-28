idf_component_register(
    SRCS "bme280_device.cpp"
    INCLUDE_DIRS include
    PRIV_INCLUDE_DIRS "include"
    REQUIRES i2c_device
)
