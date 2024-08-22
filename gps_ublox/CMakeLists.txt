cmake_minimum_required(VERSION 3.13)
set(TARGET_NAME gps_ublox)
 
message("Adding ${TARGET_NAME} C files: " )
file(GLOB CPPFILE "*.c")
foreach(source ${CPPFILE})
    message("  - " ${source})
endforeach(source)
 
message("Adding ${TARGET_NAME} H files: " )
file(GLOB HFILE "*.h")
foreach(source ${HFILE})
    message("  - " ${source})
endforeach(source)
 
add_library(${TARGET_NAME} STATIC
${CPPFILE}
${HFILE}
)
pico_generate_pio_header(${TARGET_NAME} ${CMAKE_CURRENT_LIST_DIR}/uart_rx.pio)

target_link_libraries(${TARGET_NAME} PUBLIC
    pico_stdlib
    hardware_uart
    pico_multicore
    hardware_pio
    pico_async_context_freertos
    FreeRTOS-Kernel-Heap4
    pico_async_context_threadsafe_background
)

target_include_directories(${TARGET_NAME} PRIVATE
    ${CONFIG_PATH_FREERTOS}
)