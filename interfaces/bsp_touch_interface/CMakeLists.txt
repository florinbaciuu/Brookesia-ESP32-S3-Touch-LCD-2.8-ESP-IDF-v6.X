

set(
    srcs 
    "touch_bsp_interface.c"
)

set(
    include_dirs
    "."
)

set(
    requires
)

set(
    priv_requires
    esp_lcd
    esp_system
    esp_common
    log
    esp_lcd_touch
    cst328_driver
    board_pins
    board_config
    bsp_i2c_interface
)

idf_component_register(
    SRCS 
    ${srcs}
    INCLUDE_DIRS 
    ${include_dirs}
    REQUIRES
    ${requires}
    PRIV_REQUIRES
    ${priv_requires}
)

# dezactivează tratarea warningurilor ca erori pentru componenta asta
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error
    -Wno-unused-variable
    -Wno-unused-function
)
