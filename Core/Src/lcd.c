/*
 * lcd.c
 *
 *  Created on: 07.06.2014
 *      Author: Florian
 *  Modified by: Jeremy Overesch (10/29/2025) for STM32H750
 */


/*---------------------------------------------------------------------*
 *  include files                                                      *
 *---------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include "stm32h7xx.h"
#include "../Inc/gfx.h"
#include "lcd.h"
#include "main.h"

/*---------------------------------------------------------------------*
 *  local definitions                                                  *
 *---------------------------------------------------------------------*/


/* layer size - RGB565 format (2 bytes per pixel) */
#define LCD_FRAME_SIZE				((uint32_t)(LCD_MAX_X * LCD_MAX_Y))

uint16_t s_frameBuffer[2][LCD_FRAME_SIZE] __attribute__((section(".sdram_data")));


/*---------------------------------------------------------------------*
 *  external declarations                                              *
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 *  public data                                                        *
 *---------------------------------------------------------------------*/

/*
 * address of current frame buffer to draw to
 */
uint32_t lcd_frame_buffer;

/*
 * currently selected (invisible) layer to draw to
 */
lcd_layers_t lcd_layer;

/*
 * VSYNC enabled/disabled
 */
bool lcd_vsync;

/*---------------------------------------------------------------------*
 *  private data                                                       *
 *---------------------------------------------------------------------*/

static volatile bool lcd_refreshed;

/*---------------------------------------------------------------------*
 *  private functions                                                  *
 *---------------------------------------------------------------------*/

void lcd_layer_init(LTDC_HandleTypeDef* hltdc)
{
    LTDC_LayerCfgTypeDef layerCfg;

    // background layer
    layerCfg.WindowX0 = 0;
    layerCfg.WindowX1 = LCD_MAX_X - 1;
    layerCfg.WindowY0 = 0;
    layerCfg.WindowY1 = LCD_MAX_Y - 1;

    layerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
    layerCfg.Alpha = 0xFF;
    layerCfg.Alpha0 = 0x00;
    layerCfg.Backcolor.Blue = 0xFF;
    layerCfg.Backcolor.Green = 0x00;
    layerCfg.Backcolor.Red = 0x00;

    layerCfg.ImageWidth = LCD_MAX_X;
    layerCfg.ImageHeight = LCD_MAX_Y;

    layerCfg.FBStartAdress = (uint32_t)s_frameBuffer[LCD_BACKGROUND];
    layerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
    layerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;

    HAL_LTDC_ConfigLayer(hltdc, &layerCfg, LCD_BACKGROUND);

    layerCfg.FBStartAdress = (uint32_t)s_frameBuffer[LCD_FOREGROUND];
    HAL_LTDC_ConfigLayer(hltdc, &layerCfg, LCD_FOREGROUND);

    HAL_LTDC_Reload(hltdc, LTDC_RELOAD_IMMEDIATE);

}

/*---------------------------------------------------------------------*
 *  public functions                                                   *
 *---------------------------------------------------------------------*/

void lcd_init (LTDC_HandleTypeDef* hltdc)
{
	lcd_layer_init (hltdc);

	lcd_vsync = true;

	lcd_set_layer (LCD_FOREGROUND);
}

/*
 * Switches layers and displays the old layer
 *
 * The new layer is not displayed and can be drawn to
 */
void lcd_refresh (void)
{
	switch (lcd_layer)
	{
		case LCD_BACKGROUND:
			// switch to foreground
			lcd_set_layer (LCD_FOREGROUND);

			// make foreground transparent to display background
			lcd_set_transparency (LCD_FOREGROUND, GFX_TRANSPARENT);
			break;

		case LCD_FOREGROUND:
			// switch to background
			lcd_set_layer (LCD_BACKGROUND);

			// make foreground opaque to display it
			lcd_set_transparency (LCD_FOREGROUND, GFX_OPAQUE);
			break;

		default:
			break;
	}
}

/*
 * Set the layer to draw to
 *
 * This has no effect on the LCD itself, only to drawing routines
 *
 * @param[in]	layer	layer to change to
 */
void lcd_set_layer (lcd_layers_t layer)
{
	lcd_frame_buffer = (uint32_t)s_frameBuffer[layer];
	lcd_layer = layer;
}

/*
 * Set transparency of layer
 *
 * @param[in]	layer			layer to change
 * @param[in]	transparency	0x00 is transparent, 0xFF is opaque
 */
void lcd_set_transparency (lcd_layers_t layer, uint8_t transparency)
{

	HAL_LTDC_SetAlpha(&hltdc, transparency, layer);

	if (lcd_vsync)
	{
		lcd_refreshed = false;

		__HAL_LTDC_ENABLE_IT(&hltdc, LTDC_IT_RR);

		/* reload shadow register on next vertical blank */
		HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);

		/* wait for registers to be reloaded */
		while (!lcd_refreshed);
	}
	else
	{
		/* immediately reload shadow registers */
		HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_IMMEDIATE);
	}
}

void LTDC_IRQHandler (void)
{

	if (__HAL_LTDC_GET_IT_SOURCE(&hltdc, LTDC_IT_RR) && __HAL_LTDC_GET_FLAG(&hltdc, LTDC_FLAG_RR))
	{
		__HAL_LTDC_CLEAR_FLAG(&hltdc, LTDC_FLAG_RR);
		__HAL_LTDC_DISABLE_IT(&hltdc, LTDC_IT_RR);

		lcd_refreshed = true;
	}
}


void LCD_Write(LTDC_HandleTypeDef* hltdc)
{
    static int x = 0;
    for (int i = 0; i < LCD_FRAME_SIZE; i++)
    {
        s_frameBuffer[lcd_layer][i] = i + x;
    }
    x++;
}

/*---------------------------------------------------------------------*
 *  eof                                                                *
 *---------------------------------------------------------------------*/