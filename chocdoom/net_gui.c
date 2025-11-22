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
#include "debug_console.h"

// Stub implementation for STM32 - LCD-based debug console
// Shows network status on screen during lobby

void NET_WaitForLaunch(void)
{
    boolean launched = false;
    boolean console_init = false;

    DPRINT("NET_WaitForLaunch: Waiting for game to start...");
    DPRINT("  Press button '1' to start the game");

    // Wait for game to launch (works for both client and server modes)
    // net_waiting_for_launch is set by the network code
    while (net_waiting_for_launch)
    {
        // Initialize console on first iteration (after gfx is ready)
        if (!console_init)
        {
            DebugConsole_Init();
            DebugConsole_Print("=== DOOM MULTIPLAYER LOBBY ===");
            DebugConsole_Print("");
            console_init = true;
        }

        // Process network packets
        NET_CL_Run();
        NET_SV_Run();

        // Check if connection failed
        if (!net_client_connected)
        {
            DPRINT("NET_WaitForLaunch: Lost connection to server");
            DebugConsole_Draw();
            break;
        }

        // Display network status if we have data
        if (net_client_received_wait_data)
        {
            static int last_player_count = -1;
            if (net_client_wait_data.num_players != last_player_count)
            {
                DebugConsole_Clear();
                DebugConsole_Print("=== DOOM MULTIPLAYER LOBBY ===");
                DebugConsole_Print("");

                DPRINT("Players: %d / %d",
                       net_client_wait_data.num_players,
                       net_client_wait_data.max_players);

                // Show player list
                for (int i = 0; i < net_client_wait_data.num_players; i++)
                {
                    char marker = (i == net_client_wait_data.consoleplayer) ? '>' : ' ';
                    DPRINT(" %c %d. %s (%s)", marker, i + 1,
                           net_client_wait_data.player_names[i],
                           net_client_wait_data.player_addrs[i]);
                }

                DebugConsole_Print("");
                if (net_client_wait_data.is_controller)
                {
                    DebugConsole_Print("You are the server!");
                    DebugConsole_Print("Press button '1' to start");
                }
                else
                {
                    DebugConsole_Print("Waiting for server to start...");
                }

                last_player_count = net_client_wait_data.num_players;
            }
        }

        // If we're the controller (server), check for button press to launch
        if (!launched && net_client_wait_data.is_controller)
        {
            uint16_t buttons = getButtonMatrix();
            if (buttons & BUTTON_1)
            {
                DPRINT("Button pressed - launching game!");
                DebugConsole_Draw();
                NET_CL_LaunchGame();
                launched = true;
            }
        }

        // Update display
        DebugConsole_Draw();

        // Small delay to avoid busy-waiting
        I_Sleep(10);
    }

    DPRINT("NET_WaitForLaunch: Game starting!");
    DebugConsole_Draw();
    I_Sleep(1000);  // Show final message briefly
}
