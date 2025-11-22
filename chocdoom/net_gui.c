//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2025 STM32 Port - Stub implementation
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
//     Graphical stuff related to the networking code (STM32 stub)
//

#include <stdio.h>

#include "config.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "net_client.h"
#include "net_gui.h"
#include "net_server.h"
#include "inputoutput.h"

// Stub implementation for STM32 - no textscreen GUI
// The game will wait for launch without displaying a fancy GUI
// You could extend this to show status on the LCD if desired

void NET_WaitForLaunch(void)
{
    boolean launched = false;

    printf("NET_WaitForLaunch: Waiting for game to start...\n");
    printf("  Press button '1' to start the game\n");

    // Wait for game to launch (works for both client and server modes)
    // net_waiting_for_launch is set by the network code
    while (net_waiting_for_launch)
    {
        // Process network packets
        NET_CL_Run();
        NET_SV_Run();

        // Check if connection failed
        if (!net_client_connected)
        {
            printf("NET_WaitForLaunch: Lost connection to server\n");
            break;
        }

        // If we're the controller (server), check for button press to launch
        if (!launched && net_client_wait_data.is_controller)
        {
            uint16_t buttons = getButtonMatrix();
            if (buttons & BUTTON_1)
            {
                printf("NET_WaitForLaunch: Button pressed - launching game!\n");
                NET_CL_LaunchGame();
                launched = true;
            }
        }

        // Small delay to avoid busy-waiting
        I_Sleep(10);
    }

    printf("NET_WaitForLaunch: Game starting!\n");
}
