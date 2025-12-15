cmake_minimum_required(VERSION 3.22)

#
# Root CMakeLists.txt - Project Orchestrator
# This file sets up the build environment and coordinates subdirectories
# Individual targets (application, bootloader) are built in their respective subdirectories
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
set(CMAKE_PROJECT_NAME stm32doom)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Configure Python interpreter for QSPI filesystem generation
# Use venv Python if available (has pyfatfs), otherwise fall back to system Python
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin/python3")
    set(Python3_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin/python3")
    message(STATUS "Using venv Python: ${Python3_EXECUTABLE}")
else()
    # Fall back to system Python
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    message(STATUS "Using system Python: ${Python3_EXECUTABLE}")
endif()

# QSPI Filesystem Generation configuration (used by App)
# The actual generation happens in App/CMakeLists.txt where it's consumed
set(QSPI_FS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/doom1.wad"
    "${CMAKE_CURRENT_SOURCE_DIR}/doom.wad"
    "${CMAKE_CURRENT_SOURCE_DIR}/doom2.wad"
    "${CMAKE_CURRENT_SOURCE_DIR}/iddm1.wad"
)
message(STATUS "QSPI files: ${QSPI_FS_FILES}")

# Add subdirectories
# Order matters: libraries must be defined before targets that use them

# STM32CubeMX generated infrastructure (HAL, middleware libraries)
add_subdirectory(cmake/stm32cubemx)

# Common peripheral code (shared between bootloader and app)
add_subdirectory(Common)

# Doom game engine (application-only)
add_subdirectory(chocdoom)

# Application target (stm32doom)
add_subdirectory(App)

# Bootloader target (bootloader)
add_subdirectory(bootloader)

# Combined HEX file generation
# Merges bootloader (0x08000000) and application (0x90000000) hex files
# Intel HEX format is sparse, so no gap between address spaces
add_custom_target(combined ALL
    DEPENDS stm32boot stm32doom

    # Combine hex files: remove EOF from bootloader hex, append app hex
    COMMAND head -n -1 ${CMAKE_BINARY_DIR}/bootloader/stm32boot.hex > ${CMAKE_BINARY_DIR}/stm32doom_combined.hex
    COMMAND cat ${CMAKE_BINARY_DIR}/App/stm32doom.hex >> ${CMAKE_BINARY_DIR}/stm32doom_combined.hex

    COMMENT "Creating combined bootloader+application HEX file"
    VERBATIM
)
