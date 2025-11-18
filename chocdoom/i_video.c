// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
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
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include "config.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_event.h"
#include "d_main.h"
#include "i_video.h"
#include "i_scale.h"
#include "z_zone.h"

#include "tables.h"
#include "doomkeys.h"
#include "inputoutput.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "lcd.h"
#include "gfx.h"
#include "images.h"
#include "lwip.h"

// The screen buffer; this is modified to draw things to the screen

byte *I_VideoBuffer = NULL;

// Scaled buffer for 640x480 output (indexed color)
static byte *scaled_buffer = NULL;

// If true, game is running as a screensaver

boolean screensaver_mode = false;

// Flag indicating whether the screen is currently visible:
// when the screen isnt visible, don't render the screen

boolean screenvisible;

// Mouse acceleration
//
// This emulates some of the behavior of DOS mouse drivers by increasing
// the speed when the mouse is moved fast.
//
// The mouse input values are input directly to the game, but when
// the values exceed the value of mouse_threshold, they are multiplied
// by mouse_acceleration to increase the speed.

float mouse_acceleration = 2.0;
int mouse_threshold = 10;

// Gamma correction level to use

int usegamma = 0;

int usemouse = 0;

// If true, keyboard mapping is ignored, like in Vanilla Doom.
// The sensible thing to do is to disable this if you have a non-US
// keyboard.

int vanilla_keyboard_mapping = true;


// Run lock toggle state
static bool run_lock = false;
static uint16_t prev_button_matrix = 0;

typedef struct
{
	byte r;
	byte g;
	byte b;
} col_t;

// Palette converted to RGB565

static uint16_t rgb565_palette[256];

// Current palette (8-bit RGB, needed for scaling initialization)
static byte current_palette[256 * 3];


// Last button state

static bool last_button_state;

// run state

static bool run;

void I_InitGraphics (void)
{
#if 0
    gfx_image_t keys_img;
	gfx_coord_t coords;

	gfx_init_img (&keys_img, 40, 320, GFX_PIXEL_FORMAT_RGB565, RGB565_BLACK);
	keys_img.pixel_data = (uint8_t*)img_keys;
	gfx_init_img_coord (&coords, &keys_img);

	gfx_draw_img (&keys_img, &coords);
	lcd_refresh ();

	gfx_draw_img (&keys_img, &coords);
	lcd_refresh ();
#endif
	// Allocate video buffer (320x200, indexed color)
	I_VideoBuffer = (byte*)Z_Malloc (SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);

	// Allocate scaled buffer (640x480, indexed color)
	scaled_buffer = (byte*)Z_Malloc (640 * 480, PU_STATIC, NULL);

	// Initialize scaling system
	// dest_pitch is the width of the destination buffer in bytes
	I_InitScale(I_VideoBuffer, scaled_buffer, 640);

	screenvisible = true;
}

void I_ShutdownGraphics (void)
{
	Z_Free (I_VideoBuffer);
	Z_Free (scaled_buffer);
}

void I_StartFrame (void)
{
    MX_LWIP_Process();
}

void I_GetEvent (void)
{
    static uint16_t oldButtonMatrix = 0;
    int32_t enc1Change = getEncoder1Change();
    int32_t enc2Change = getEncoder2Change();
    bool enc1Button = getEncoder1Button();
    bool enc2Button = getEncoder2Button();

	event_t event;

	event.type = ev_joystick;
	event.data1 = 0 | (enc1Button ? 0x1 : 0) | (enc2Button ? 0x8 : 0);
	event.data2 = enc1Change;
	event.data3 = enc2Change;
	event.data4 = 0;

	D_PostEvent (&event);

    uint16_t buttonMatrix = getButtonMatrix();

    // Check if RSHIFT button (bit 5) was just pressed to toggle run lock
    if ((buttonMatrix & BUTTON_RUN) && !(prev_button_matrix & BUTTON_RUN))
    {
        run_lock = !run_lock;
    }
    prev_button_matrix = buttonMatrix;

    // If run lock is ON, send RSHIFT keydown event every frame
    if (run_lock)
    {
        event.type = ev_keydown;
        event.data1 = KEY_RSHIFT;
        event.data2 = 0;
        event.data3 = 0;
        event.data4 = 0;
        D_PostEvent(&event);
    }

    if (oldButtonMatrix != buttonMatrix)
    {

        uint16_t changed = oldButtonMatrix ^ buttonMatrix;

        // Process each button that changed state
        for (int i = 0; i < 16; i++)
        {
            if (changed & (1 << i))
            {
                int keycode = button_key_map[i];
                // Skip unused keys
                if (keycode == 0) continue;

                // Skip RSHIFT when run lock is active (toggle handles it)
                if (keycode == KEY_RSHIFT && run_lock) continue;

                event.type = (buttonMatrix & (1 << i)) ? ev_keydown : ev_keyup;
                event.data1 = keycode;
                // For ASCII keys, data2 is the same as data1; for special keys it's 0
                event.data2 = (keycode < 128) ? keycode : 0;
                event.data3 = 0;
                event.data4 = 0;

                D_PostEvent(&event);
            }
        }

        printf("Key press: %04x\n", buttonMatrix);
        oldButtonMatrix = buttonMatrix;
    }
}

bool getRunLock(void)
{
    return run_lock;
}

void I_StartTic (void)
{
	I_GetEvent();
}

void I_UpdateNoBlit (void)
{
}

void I_FinishUpdate (void)
{
	int x, y;
	byte index;
	int x_offset = 80;  // Center 640 pixels in 800 pixel width: (800-640)/2 = 80

	lcd_vsync = false;

	// Scale from 320x200 to 640x480 using vertical stretch mode
	if (mode_stretch_2x.DrawScreen != NULL)
	{
		mode_stretch_2x.DrawScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);
	}

	// Convert scaled buffer (640x480 indexed color) to RGB565 and center on screen
	for (y = 0; y < 480; y++)
	{
		for (x = 0; x < 640; x++)
		{
			index = scaled_buffer[y * 640 + x];
			((uint16_t*)lcd_frame_buffer)[y * GFX_MAX_WIDTH + (x + x_offset)] = rgb565_palette[index];
		}
	}

	lcd_refresh ();

	lcd_vsync = true;
}

//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, I_VideoBuffer, SCREENWIDTH * SCREENHEIGHT);
}

//
// I_SetPalette
//
void I_SetPalette (byte* palette)
{
	int i;
	col_t* c;

	// Save the palette for scaling system
	memcpy(current_palette, palette, 256 * 3);

	// Initialize stretch mode blend tables (only needs to be done once)
	// This will be called again if palette changes, which rebuilds the tables
	if (mode_stretch_2x.InitMode != NULL)
	{
		mode_stretch_2x.InitMode(current_palette);
	}

	for (i = 0; i < 256; i++)
	{
		c = (col_t*)palette;

		rgb565_palette[i] = GFX_RGB565(gammatable[usegamma][c->r],
									   gammatable[usegamma][c->g],
									   gammatable[usegamma][c->b]);

		palette += 3;
	}
}

// Given an RGB value, find the closest matching palette index.

int I_GetPaletteIndex (int r, int g, int b)
{
    int best, best_diff, diff;
    int i;
    col_t color;

    best = 0;
    best_diff = INT_MAX;

    for (i = 0; i < 256; ++i)
    {
    	color.r = GFX_RGB565_R(rgb565_palette[i]);
    	color.g = GFX_RGB565_G(rgb565_palette[i]);
    	color.b = GFX_RGB565_B(rgb565_palette[i]);

        diff = (r - color.r) * (r - color.r)
             + (g - color.g) * (g - color.g)
             + (b - color.b) * (b - color.b);

        if (diff < best_diff)
        {
            best = i;
            best_diff = diff;
        }

        if (diff == 0)
        {
            break;
        }
    }

    return best;
}

void I_BeginRead (void)
{
}

void I_EndRead (void)
{
}

void I_SetWindowTitle (char *title)
{
}

void I_GraphicsCheckCommandLine (void)
{
}

void I_SetGrabMouseCallback (grabmouse_callback_t func)
{
}

void I_EnableLoadingDisk (void)
{
}

void I_BindVideoVariables (void)
{
}

void I_DisplayFPSDots (boolean dots_on)
{
}

void I_CheckIsScreensaver (void)
{
}
