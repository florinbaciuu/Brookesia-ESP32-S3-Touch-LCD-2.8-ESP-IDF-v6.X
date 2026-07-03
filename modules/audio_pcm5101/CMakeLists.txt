set(srcs
    "src/audio_pcm5101.c"
)

set(include_dirs
    "include"
)

set(priv_requires
    board_config
    esp_driver_i2s
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
