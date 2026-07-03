

set(
    srcs 
    "touch_cst328_driver.c"
)

set(
    include_dirs
    "."
)

set(
    requires
    esp_lcd_touch
    driver
)

set(
    priv_requires
    esp_system
    esp_common
    log
    esp_driver_gpio
    esp_lcd
    board_pins
    bsp_i2c_interface
    board_config
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
