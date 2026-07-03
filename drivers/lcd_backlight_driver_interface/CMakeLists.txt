

set(
    srcs 
    "lcd_backlight.c"
)

set(
    include_dirs
    "."
)

set(
    requires
    esp_driver_gpio
    esp_driver_ledc
)

set(
    priv_requires
    log
    board_pins
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
