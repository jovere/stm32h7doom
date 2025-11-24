# Application-specific sources
# This includes both user-added and STM32CubeMX generated application files

set(APP_SOURCES
    # Core application files
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32h7xx_it.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32h7xx_hal_msp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/system_stm32h7xx.c

    # Utility files
    ${CMAKE_CURRENT_SOURCE_DIR}/unique_id.c
    ${CMAKE_CURRENT_SOURCE_DIR}/debug_console.c

    # User application files
    ${CMAKE_CURRENT_SOURCE_DIR}/lcd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gfx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/jpeg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/images.c
    ${CMAKE_CURRENT_SOURCE_DIR}/inputoutput.c

    # Startup code
    ${CMAKE_SOURCE_DIR}/startup_stm32h750xx.s
)

# Add sources to the parent target
# The target name will be passed from the root CMakeLists.txt
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${APP_SOURCES})

# Add App directory to include path
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
