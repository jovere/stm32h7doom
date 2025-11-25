# Bootloader Build Configuration
# Runs from internal flash (0x08000000)

add_executable(stm32boot
    main.c
    stm32h7xx_it.c
    stm32h7xx_hal_msp.c
    system_stm32h7xx.c
    ${CMAKE_SOURCE_DIR}/startup_stm32h750xx.s
)

target_include_directories(stm32boot PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(stm32boot
    stm32cubemx
    STM32_Drivers
    CommonPeripherals
)

# Use stm32boot-specific linker script
set_target_properties(stm32boot PROPERTIES
    LINK_FLAGS "-T${CMAKE_CURRENT_SOURCE_DIR}/STM32H750XX_FLASH_custom.ld"
)

# Generate binary and hex files
add_custom_command(TARGET stm32boot POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:stm32boot> ${CMAKE_CURRENT_BINARY_DIR}/stm32boot.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:stm32boot> ${CMAKE_CURRENT_BINARY_DIR}/stm32boot.hex
    COMMAND ${CMAKE_OBJDUMP} -h -S $<TARGET_FILE:stm32boot> > ${CMAKE_CURRENT_BINARY_DIR}/stm32boot.lss
    COMMAND ${CMAKE_SIZE} --format=sysv -d $<TARGET_FILE:stm32boot>
    COMMENT "Generating stm32boot.bin and stm32boot.hex"
)
