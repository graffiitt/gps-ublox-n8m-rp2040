cmake_minimum_required(VERSION 3.13)
set(PICO_BOARD pico_w CACHE STRING "Board type")
set(FREERTOS_KERNEL_PATH /Users/vlad/programming/pico/FreeRTOS-Kernel)

include(pico_sdk_import.cmake)
include(FreeRTOS_Kernel_import.cmake)

project(gps_uart C CXX ASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
pico_sdk_init()

add_executable(${PROJECT_NAME}
main.c
)

set(CONFIG_PATH_FREERTOS ${CMAKE_CURRENT_LIST_DIR})
add_subdirectory(gps_ublox)

target_link_libraries(${PROJECT_NAME}
gps_ublox
pico_async_context_freertos
FreeRTOS-Kernel-Heap4
pico_stdlib
pico_cyw43_arch_none
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    configNUMBER_OF_CORES=1
)

pico_add_extra_outputs(${PROJECT_NAME})
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
