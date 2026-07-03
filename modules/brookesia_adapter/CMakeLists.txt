set(srcs
    "src/brookesia_adapter.cpp"
    "src/brookesia_project_board.cpp"
    "src/brookesia_storage_automount.cpp"
)

set(include_dirs
    "include"
)

set(requires
    brookesia_hal_interface
    lvgl
)

set(priv_requires
    esp_adc
    esp_timer
    esp_lcd
    fatfs
    littlefs
    log
    lvgl
    audio_pcm5101
    board_pins
    lcd_backlight_driver_interface
    bsp_lcd_interface
    bsp_touch_interface
    bsp_lvgl_framework
    filesystem
    power_key
    qmi8658
    rtc_pcf85063
    settings_nvs
    spiffs
    ui
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
