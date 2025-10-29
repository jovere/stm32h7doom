/// @file lcd.cpp
///
/// Copyright Miller Electric Mfg. LLC as an unpublished work.
/// All rights reserved.
///
/// The information contained herein is confidential property of Miller Electric
/// Mfg. LLC.  The use of, copying, transfer or disclosure of such information is
/// prohibited except by express written agreement with Miller Electric Mfg. LLC.
///
#include "../Inc/lcd.h"
#include <stdint.h>

#define LCD_WIDTH 800
#define LCD_HEIGHT 480
#define LCD_TOTAL LCD_WIDTH*LCD_HEIGHT
uint16_t bgFrameBuffer[LCD_TOTAL] __attribute__((section(".sdram_data")));
uint16_t fgFrameBuffer[LCD_TOTAL] __attribute__((section(".sdram_data")));

void LCD_Init(LTDC_HandleTypeDef* hltdc)
{
    LTDC_LayerCfgTypeDef layerCfg;

    // background layer
    layerCfg.WindowX0 = 0;
    layerCfg.WindowX1 = LCD_WIDTH - 1;
    layerCfg.WindowY0 = 0;
    layerCfg.WindowY1 = LCD_HEIGHT - 1;

    layerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
    layerCfg.Alpha = 0xFF;
    layerCfg.Alpha0 = 0x00;
    layerCfg.Backcolor.Blue = 0xFF;
    layerCfg.Backcolor.Green = 0x00;
    layerCfg.Backcolor.Red = 0x00;

    layerCfg.ImageWidth = LCD_WIDTH;
    layerCfg.ImageHeight = LCD_HEIGHT;

    layerCfg.FBStartAdress = (uint32_t)bgFrameBuffer;
    layerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
    layerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;

    HAL_LTDC_ConfigLayer(hltdc, &layerCfg, 0);

    layerCfg.FBStartAdress = (uint32_t)fgFrameBuffer;
    HAL_LTDC_ConfigLayer(hltdc, &layerCfg, 1);

    HAL_LTDC_Reload(hltdc, LTDC_RELOAD_IMMEDIATE);
}

void LCD_Write(LTDC_HandleTypeDef* hltdc)
{
    static int x = 0;
    for (int i = 0; i < LCD_TOTAL; i++)
    {
        bgFrameBuffer[i] = fgFrameBuffer[i] = i + x;
    }
    x++;
}