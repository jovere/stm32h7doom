cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME stm32doom_cube)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)
add_subdirectory(chocdoom)

# Configure Python interpreter for filesystem generation
# Use venv Python if available (has pyfatfs), otherwise fall back to system Python
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin/python3")
    set(Python3_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin/python3")
    message(STATUS "Using venv Python: ${Python3_EXECUTABLE}")
else()
    # Fall back to system Python
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    message(STATUS "Using system Python: ${Python3_EXECUTABLE}")
endif()

# QSPI Filesystem Generation
# Configure the WAD file to include in the filesystem
set(QSPI_WAD_FILE "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
#set(QSPI_WAD_FILE "" CACHE STRING "Doom wad file to be put into the filesystem")
set(QSPI_FS_BIN "${CMAKE_CURRENT_BINARY_DIR}/qspi_fs.bin")
set(QSPI_FS_OBJ "${CMAKE_CURRENT_BINARY_DIR}/qspi_fs.o")

if(QSPI_WAD_FILE)
    if(NOT EXISTS "${QSPI_WAD_FILE}")
        message(FATAL_ERROR "QSPI_WAD_FILE not found: ${QSPI_WAD_FILE}")
    endif()

    # Add custom command to generate filesystem image
    add_custom_command(
        OUTPUT "${QSPI_FS_BIN}"
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tools/create_fatfs.py"
                "${QSPI_FS_BIN}" "${QSPI_WAD_FILE}"
        DEPENDS "${QSPI_WAD_FILE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/create_fatfs.py"
        COMMENT "Generating QSPI FAT filesystem with ${QSPI_WAD_FILE}"
        VERBATIM
    )

    # Convert binary file to object file using objcopy
    add_custom_command(
        OUTPUT "${QSPI_FS_OBJ}"
        COMMAND ${CMAKE_OBJCOPY} -I binary -O elf32-littlearm
                --rename-section .data=.qspi_fs_data,alloc,load,readonly,data,contents
                "${QSPI_FS_BIN}" "${QSPI_FS_OBJ}"
        DEPENDS "${QSPI_FS_BIN}"
        COMMENT "Converting QSPI filesystem binary to object file"
        VERBATIM
    )

    # Link the object file into the executable
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE "${QSPI_FS_OBJ}")

    message(STATUS "QSPI WAD file: ${QSPI_WAD_FILE}")
else()
    message(STATUS "QSPI_WAD_FILE not set - QSPI filesystem will not be generated")
    message(STATUS "To enable: cmake -DQSPI_WAD_FILE=/path/to/doom.wad ..")
endif()

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    Core/Src/lcd.c
    Core/Src/gfx.c
    Core/Src/jpeg.c
    Core/Src/images.c
    # QSPI Flash and FATFS integration
    Core/Src/mt25ql512abb.c
    Core/Src/quadspi.c
    FATFS/Target/user_diskio.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    chocdoom
    # Add user defined libraries
)

# Post-build steps for generating HEX and LSS files
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex
        COMMAND ${CMAKE_OBJDUMP} -h -S $<TARGET_FILE:${PROJECT_NAME}> > ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.lss
        COMMAND ${CMAKE_SIZE} --format=sysv -d $<TARGET_FILE:${PROJECT_NAME}>
        COMMENT "Generating HEX and LSS files and showing size information"
)
