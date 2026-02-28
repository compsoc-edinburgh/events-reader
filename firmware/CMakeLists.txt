cmake_minimum_required(VERSION 3.13)


include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(events_reader C CXX ASM)
pico_sdk_init()
add_executable(events_reader src/main.c lwipopts.h)
target_include_directories(events_reader PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)
pico_enable_stdio_usb(events_reader 1)
target_link_libraries(events_reader pico_multicore pico_stdlib pico_cyw43_arch_lwip_threadsafe_background)
pico_add_extra_outputs(events_reader)
