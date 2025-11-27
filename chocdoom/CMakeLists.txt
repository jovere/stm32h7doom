# Chocolate Doom Game Engine
# Classic Doom game engine ported to STM32H750

# Doom game sources (from Makefile SRC_DOOM)
set(DOOM_SOURCES
    dummy.c
    am_map.c
    doomdef.c
    doomstat.c
    dstrings.c
    d_event.c
    d_items.c
    d_iwad.c
    d_loop.c
    d_main.c
    d_mode.c
    d_net.c
    f_finale.c
    f_wipe.c
    g_game.c
    hu_lib.c
    hu_stuff.c
    info.c
    i_cdmus.c
    i_endoom.c
    i_joystick.c
    #i_main.c
    i_scale.c
    i_sound.c
    i_system.c
    i_timer.c
    i_video.c
    memio.c
    m_argv.c
    m_bbox.c
    m_cheat.c
    m_config.c
    m_controls.c
    m_fixed.c
    m_menu.c
    m_misc.c
    m_random.c
    p_ceilng.c
    p_doors.c
    p_enemy.c
    p_floor.c
    p_inter.c
    p_lights.c
    p_map.c
    p_maputl.c
    p_mobj.c
    p_plats.c
    p_pspr.c
    p_saveg.c
    p_setup.c
    p_sight.c
    p_spec.c
    p_switch.c
    p_telept.c
    p_tick.c
    p_user.c
    r_bsp.c
    r_data.c
    r_draw.c
    r_main.c
    r_plane.c
    r_segs.c
    r_sky.c
    r_things.c
    sha1.c
    sounds.c
    statdump.c
    st_lib.c
    st_stuff.c
    s_sound.c
    tables.c
    v_video.c
    wi_stuff.c
    w_checksum.c
    w_file.c
    w_file_stdc.c
    w_main.c
    w_wad.c
    z_zone.c
    aes_prng.c
    net_client.c
    net_common.c
    net_dedicated.c
    net_gui.c
    net_io.c
    net_loop.c
    net_lwip.c
    net_packet.c
    net_query.c
    net_server.c
    net_structrw.c

    # OPL Music System
    opl/dbopl.c
    opl/opl.c
    opl/opl_queue.c
    opl/opl_timer.c
    i_oplmusic.c
    midifile.c
    mus2mid.c
)

# Create Doom game engine static library
add_library(chocdoom STATIC ${DOOM_SOURCES})

# Set include directories for this library
target_include_directories(chocdoom
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/opl  # OPL emulator headers
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        # App directory needed for platform functions (HAL_Delay, systime, etc.)
        ${CMAKE_SOURCE_DIR}/App
        # Use all middleware includes (FatFs, LwIP, etc.)
        ${MX_Middleware_Include_Dirs}
)

target_link_libraries(chocdoom PRIVATE stm32cubemx)

# Doom game compile definitions
target_compile_definitions(chocdoom
    PUBLIC
        DOOM
        HAVE_CONFIG_H=0
        NOSAVE          # Disable save game (FatFs is read-only)
    PRIVATE
        # No STM32 definitions needed - pure game engine
)

# Configure compiler options for Doom
target_compile_options(chocdoom
    PRIVATE
        -Wall
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-unused-const-variable
        -Wno-format-truncation      # Suppress Doom's string formatting warnings
        -Wno-stringop-truncation    # Suppress strncpy warnings in legacy code
        -Wno-dangling-pointer       # Suppress pointer warnings in Doom's sprite sorting
        -O2  # Optimize for performance - Doom needs it!
)

# Print Doom configuration
list(LENGTH DOOM_SOURCES DOOM_SOURCE_COUNT)
message(STATUS "Chocolate Doom Game Engine:")
message(STATUS "  Sources: ${DOOM_SOURCE_COUNT} game files")
message(STATUS "  Dependencies: FatFS only (for file I/O)")
message(STATUS "  Architecture: Pure game engine, hardware-agnostic")
message(STATUS "  Memory: Optimized for 64MB SDRAM + QSPI flash")