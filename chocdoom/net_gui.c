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

// Stub implementation for STM32 - no textscreen GUI
// The game will wait for launch without displaying a fancy GUI
// You could extend this to show status on the LCD if desired

void NET_WaitForLaunch(void)
{
    int wait_time = 0;
    boolean auto_launched = false;

    printf("NET_WaitForLaunch: Waiting for game to start...\n");

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

        // Auto-launch for server mode after getting wait data
        // If we're the controller (server), launch after 2 seconds
        if (!auto_launched && net_client_received_wait_data &&
            net_client_wait_data.is_controller && wait_time > 200)
        {
            printf("NET_WaitForLaunch: Auto-launching game (server mode)\n");
            NET_CL_LaunchGame();
            auto_launched = true;
        }

        // Small delay to avoid busy-waiting
        I_Sleep(10);
        wait_time += 10;
    }

    printf("NET_WaitForLaunch: Game starting!\n");
}
