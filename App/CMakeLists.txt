# Application Build Configuration
# Main application that runs from QSPI flash (0x90000000) → SDRAM (0xD0000000)

# QSPI Filesystem Generation
# Generate the QSPI filesystem image and convert it to an object file
set(QSPI_FS_BIN "${CMAKE_CURRENT_BINARY_DIR}/qspi_fs.bin")
set(QSPI_FS_OBJ "${CMAKE_CURRENT_BINARY_DIR}/qspi_fs.o")

set(EXISTING_FILES)
foreach(file ${QSPI_FS_FILES})
    if(EXISTS "${file}")
        message("File ${file} found")
        list(APPEND EXISTING_FILES "${file}")
    endif()
endforeach()

if(EXISTING_FILES)
    message("The following files will be added to the filesystem: ${EXISTING_FILES}")
    # Generate filesystem image
    add_custom_command(
        OUTPUT "${QSPI_FS_BIN}"
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/tools/create_fatfs.py"
                "${QSPI_FS_BIN}" ${EXISTING_FILES}
        DEPENDS ${EXISTING_FILES} "${CMAKE_SOURCE_DIR}/tools/create_fatfs.py"
        COMMENT "Generating QSPI FAT filesystem with ${EXISTING_FILES}"
        VERBATIM
    )

    # Convert binary to object file
    add_custom_command(
        OUTPUT "${QSPI_FS_OBJ}"
        COMMAND ${CMAKE_OBJCOPY} -I binary -O elf32-littlearm
                --rename-section .data=.qspi_fs_data,alloc,load,readonly,data,contents
                "${QSPI_FS_BIN}" "${QSPI_FS_OBJ}"
        DEPENDS "${QSPI_FS_BIN}"
        COMMENT "Converting QSPI filesystem binary to object file"
        VERBATIM
    )
endif()

# Create application executable
add_executable(stm32doom
    # Core application files
    main.c
    stm32h7xx_it.c
    stm32h7xx_hal_msp.c
    system_stm32h7xx.c

    # Utility files
    unique_id.c
    debug_console.c

    # User application files
    lcd.c
    gfx.c
    jpeg.c
    images.c
    inputoutput.c
    audio_stm32.c

    # Startup code
    ${CMAKE_SOURCE_DIR}/startup_stm32h750xx.s

    # Middleware glue code (from STM32CubeMX)
    ${MX_Middleware_App_Src}

    # Additional middleware files
    ${CMAKE_SOURCE_DIR}/FATFS/Target/user_diskio.c
    ${CMAKE_SOURCE_DIR}/LWIP/Target/ksz8863.c

    # QSPI filesystem object (generated in root CMakeLists.txt)
    # temporarily remove to make loading faster (if not changing the wad file)
    ${QSPI_FS_OBJ}
)

# Add include paths
target_include_directories(stm32doom PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MX_Middleware_Include_Dirs}
)

# Link libraries
target_link_libraries(stm32doom
    stm32cubemx
    STM32_Drivers
    CommonPeripherals
    LwIP
    FatFs
    chocdoom
)

# Set application-specific linker script
set_target_properties(stm32doom PROPERTIES
    LINK_FLAGS "-T${CMAKE_CURRENT_SOURCE_DIR}/STM32H750XX_FLASH_custom.ld"
)

# Post-build steps for generating HEX and LSS files
add_custom_command(TARGET stm32doom POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:stm32doom> ${CMAKE_CURRENT_BINARY_DIR}/stm32doom.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:stm32doom> ${CMAKE_CURRENT_BINARY_DIR}/stm32doom.hex
    COMMAND ${CMAKE_OBJDUMP} -h -S $<TARGET_FILE:stm32doom> > ${CMAKE_CURRENT_BINARY_DIR}/stm32doom.lss
    COMMAND ${CMAKE_SIZE} --format=sysv -d $<TARGET_FILE:stm32doom>
    COMMENT "Generating HEX, BIN, and LSS files and showing size information (program to QSPI at 0x90000000)"
)
