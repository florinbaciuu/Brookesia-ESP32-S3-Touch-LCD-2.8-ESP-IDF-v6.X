file(GLOB_RECURSE C_SRCS "*.c")
file(GLOB_RECURSE CPP_SRCS "*.cpp")

idf_component_register(
    SRCS ${C_SRCS} ${CPP_SRCS}
    INCLUDE_DIRS .
    PRIV_REQUIRES common_components esp_psram
    WHOLE_ARCHIVE
)

target_compile_options(${COMPONENT_LIB}
    PRIVATE
        -Wno-unused-but-set-variable -Wno-unused-variable
        -Wno-catch-value -Wno-error=catch-value=
        -Wno-unused-function
        -Wno-error=return-type -Wno-return-type
        -Wno-error=maybe-uninitialized -Wno-maybe-uninitialized
        -Wno-sized-deallocation
        -Wno-error=mismatched-new-delete -Wno-mismatched-new-delete
)

target_compile_definitions(${COMPONENT_LIB}
    PRIVATE
        BOOST_THREAD_ENABLE_GET_ID
)
