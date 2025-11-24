# Common Peripherals Library
# Shared between bootloader and application
# INTERFACE so each target compiles with their own hal_conf.h

add_library(CommonPeripherals INTERFACE)

target_sources(CommonPeripherals INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/quadspi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mt25ql512abb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syscalls.c
)

target_include_directories(CommonPeripherals INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(CommonPeripherals INTERFACE
    stm32cubemx
)
