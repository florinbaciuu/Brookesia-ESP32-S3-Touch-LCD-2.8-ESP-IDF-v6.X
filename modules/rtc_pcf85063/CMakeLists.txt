set(srcs
    "src/rtc_pcf85063.c"
)

set(include_dirs
    "include"
)

set(priv_requires
    board_config
    bsp_aux_i2c_interface
    esp_driver_i2c
    log
)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS ${include_dirs}
    PRIV_REQUIRES ${priv_requires}
)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error
    -Wno-unused-variable
    -Wno-unused-function
)
