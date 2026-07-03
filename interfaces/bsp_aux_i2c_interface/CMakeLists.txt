set(srcs
    "aux_i2c_bsp_interface.c"
)

set(include_dirs
    "."
)

set(requires
    esp_driver_i2c
)

set(priv_requires
    board_config
    log
)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS ${include_dirs}
    REQUIRES ${requires}
    PRIV_REQUIRES ${priv_requires}
)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error
    -Wno-unused-variable
    -Wno-unused-function
)
