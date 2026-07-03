set(srcs
    "apps/board_info/phone_app_board_info.cpp"
    "src/app_tabs_ui.cpp"
    "src/brookesia_diagnostics_ui.cpp"
    "src/brookesia_phone_launcher.cpp"
)

set(include_dirs
    "apps/board_info"
    "include"
)

set(requires
    espressif__esp-brookesia
    lvgl
)

set(priv_requires
    esp_timer
    log
    bsp_lvgl_framework
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
