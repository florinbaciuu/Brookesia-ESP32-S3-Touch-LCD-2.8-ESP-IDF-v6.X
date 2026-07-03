set(srcs
    "apps/board_info/phone_app_board_info.cpp"
    "apps/clock/phone_app_clock.cpp"
    "apps/sensors/phone_app_sensors.cpp"
    "apps/settings/phone_app_settings.cpp"
    "apps/storage/phone_app_storage.cpp"
    "apps/system/phone_app_system.cpp"
    "src/app_tabs_ui.cpp"
    "src/brookesia_diagnostics_ui.cpp"
    "src/brookesia_phone_launcher.cpp"
)

set(include_dirs
    "apps/board_info"
    "apps/clock"
    "apps/sensors"
    "apps/settings"
    "apps/storage"
    "apps/system"
    "include"
)

set(requires
    espressif__esp-brookesia
    lvgl
)

set(priv_requires
    esp_timer
    esp_system
    log
    bsp_lvgl_framework
    audio_pcm5101
    fatfs
    filesystem
    heap
    lcd_backlight_driver_interface
    littlefs
    power_key
    qmi8658
    rtc_pcf85063
    settings_nvs
    spi_flash
    spiffs
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
