cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Environment workaround: this ESP-IDF checkout does not define
# set_config_files_compile_definitions() (used by the mbedtls component).
# Define a no-op so configuration completes. It is safe because the app
# does not use MBEDTLS config-file includes.
if(NOT COMMAND set_config_files_compile_definitions)
    function(set_config_files_compile_definitions target)
    endfunction()
endif()

project(ws2812_blink)
