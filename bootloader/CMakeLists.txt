# Bootloader Build Configuration
# Runs from internal flash (0x08000000)

add_executable(bootloader
    main.c
    stm32h7xx_it.c
    stm32h7xx_hal_msp.c
    system_stm32h7xx.c
    ${CMAKE_SOURCE_DIR}/startup_stm32h750xx.s
)

target_include_directories(bootloader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(bootloader
    stm32cubemx
    STM32_Drivers
    CommonPeripherals
)

# Use bootloader-specific linker script
set_target_properties(bootloader PROPERTIES
    LINK_FLAGS "-T${CMAKE_CURRENT_SOURCE_DIR}/STM32H750XX_BOOTLOADER.ld"
)

# Generate binary and hex files
add_custom_command(TARGET bootloader POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:bootloader> ${CMAKE_CURRENT_BINARY_DIR}/bootloader.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:bootloader> ${CMAKE_CURRENT_BINARY_DIR}/bootloader.hex
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:bootloader>
    COMMENT "Generating bootloader.bin and bootloader.hex"
)

# Print memory usage
add_custom_command(TARGET bootloader POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Bootloader build complete:"
    COMMAND ${CMAKE_COMMAND} -E echo "  - bootloader.elf (with debug symbols)"
    COMMAND ${CMAKE_COMMAND} -E echo "  - bootloader.bin (for 0x08000000)"
    COMMAND ${CMAKE_COMMAND} -E echo "  - bootloader.hex (Intel HEX)"
)
