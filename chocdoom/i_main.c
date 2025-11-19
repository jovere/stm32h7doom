//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Main program, simply calls D_DoomMain high level loop.
//

#include "config.h"

#include <stdio.h>

#ifdef ORIGCODE
#include "SDL.h"
#endif

#include "doomtype.h"
#include "i_system.h"
#include "m_argv.h"


#ifdef FEATURE_MULTIPLAYER
#include "inputoutput.h"
#include "lwip.h"
#endif

//
// D_DoomMain()
// Not a globally visible function, just included for source reference,
// calls all startup code, parses command line options.
//

void D_DoomMain (void);

#ifdef FEATURE_MULTIPLAYER
//
// Configure network mode based on button held at startup
// Button layout (from inputoutput.h):
//   ESC (bit 0): Single player (default)
//   F11 (bit 1): Start as server
//   '1' (bit 2): Join as client
//
static void ConfigureNetworkMode(void)
{
    uint16_t buttons;
    extern struct netif gnetif;
    char ip_str[16];

    // Wait a moment for buttons to settle
    I_Sleep(100);

    // Read button state
    buttonMatrixScan();
    I_Sleep(10);
    buttonMatrixScan();
    buttons = getButtonMatrix();

    // Get our IP address for display
    snprintf(ip_str, sizeof(ip_str), "%s", ip4addr_ntoa(netif_ip4_addr(&gnetif)));

    if (buttons & (1 << 1)) {
        // F11 held - SERVER MODE
        printf("=================================\n");
        printf("  DOOM MULTIPLAYER - SERVER MODE\n");
        printf("  IP Address: %s\n", ip_str);
        printf("  Port: 2342\n");
        printf("=================================\n");
        myargv[myargc++] = "-server";

    } else if (buttons & (1 << 2)) {
        // Button '1' held - CLIENT MODE
        // TODO: Make server IP configurable via config file
        static char server_arg[20] = "192.168.0.10";

        printf("=================================\n");
        printf("  DOOM MULTIPLAYER - CLIENT MODE\n");
        printf("  Our IP: %s\n", ip_str);
        printf("  Connecting to: %s:2342\n", server_arg);
        printf("=================================\n");
        myargv[myargc++] = "-connect";
        myargv[myargc++] = server_arg;

    } else {
        // No button held - SINGLE PLAYER
        printf("=================================\n");
        printf("  DOOM - SINGLE PLAYER MODE\n");
        printf("=================================\n");
    }
}
#endif

int main(int argc, char **argv)
{
    // save arguments

    myargc = argc;
    myargv = argv;

    M_FindResponseFile();

#ifdef FEATURE_MULTIPLAYER
    // Configure network mode based on startup button
    ConfigureNetworkMode();
#endif

    // start doom

    D_DoomMain ();

    return 0;
}

