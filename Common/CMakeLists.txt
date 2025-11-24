# Common Peripherals Library
# Shared between bootloader and application

add_library(CommonPeripherals OBJECT
    quadspi.c
    mt25ql512abb.c
    syscalls.c
)

target_include_directories(CommonPeripherals PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(CommonPeripherals PUBLIC
    stm32cubemx
)
