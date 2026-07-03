

set(
    srcs 
    "vernon_st7789t_driver.c"
)

set(
    include_dirs
    "."
)

set(
    requires
    esp_common
    esp_lcd
)

set(
    priv_requires
    esp_lcd
    driver
    log
    esp_common
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
