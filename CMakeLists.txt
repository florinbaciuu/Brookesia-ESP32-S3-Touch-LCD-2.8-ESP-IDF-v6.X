# SPDX-FileCopyrightText: 2026 Florin Aurel Baciu
# SPDX-License-Identifier: MIT

# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
# The set() commands should be placed after the cmake_minimum(...) line but before the include(...) line.
cmake_minimum_required(VERSION 3.22)

# # ---------- setări generale de compilare -------------
set( NO_WARNING_FLAGS -Wno-error -Wno-error=cpp -Wno-error=type-limits -Wno-error=unused-function -Wno-error=unused-variable
    -Wno-error=unused-but-set-variable -Wno-error=deprecated-declarations -Wno-error=extra -Wno-unused-parameter -Wno-sign-compare 
    -Wno-enum-conversion )

# # ---------- setări C și CXX -------------
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-function -Wno-unused-variable -Wno-error -Wno-error=cpp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function -Wno-unused-variable -Wno-error -Wno-error=cpp -Wno-deprecated-enum-enum-conversion ")
add_definitions( ${NO_WARNING_FLAGS} )

# # ---------- opțiuni adiționale de compilare ----
add_compile_options(
   $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-enum-enum-conversion>
   $<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-function>
   $<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-variable>
   $<$<COMPILE_LANGUAGE:CXX>:-Wno-error>
   $<$<COMPILE_LANGUAGE:CXX>:-Wno-error=type-limits>
   $<$<COMPILE_LANGUAGE:C>:-Wno-unused-function>
   $<$<COMPILE_LANGUAGE:C>:-Wno-unused-variable>
   $<$<COMPILE_LANGUAGE:C>:-Wno-error>
   $<$<COMPILE_LANGUAGE:C>:-Wno-error=type-limits>
)

# ---------- LVGL FRAMEWORK config -------------
set(LV_CONF_PATH "${CMAKE_SOURCE_DIR}/configs/lv_conf.h")
add_compile_definitions(LV_CONF_PATH="${LV_CONF_PATH}")
add_compile_definitions(LVGL_VERSION_MAJOR=9)

# ---------- extra components --------
# Seteaza folder extra pt componente
set(EXTRA_COMPONENT_DIRS
    modules
    drivers
    configs
    frameworks
    interfaces
    ui
)


# ---------- CCache ------------------
# chestii generale ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
   message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
   set(CCACHE_ENABLE ON)
   set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
   set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

##################################################
# aici se încarcă magia IDF
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
##################################################
# acum poți folosi funcțiile ESP-IDF deoarece ai incarcat magia IDF

# ---------- setări adiționale IDF -------------
idf_build_set_property(MINIMAL_BUILD ON)

##################################################
# ---------- final magic PROJECT -------------
project(ESP32-S3-TOUCH-LCD-2.8-IDF-v6.X C CXX ASM)
##################################################

# # Set compile options to disable warnings as errors in IDF build
idf_build_set_property( COMPILE_OPTIONS "${NO_WARNING_FLAGS}" APPEND )
