/// @file lcd.hpp
///
/// Copyright Miller Electric Mfg. LLC as an unpublished work.
/// All rights reserved.
///
/// The information contained herein is confidential property of Miller Electric
/// Mfg. LLC.  The use of, copying, transfer or disclosure of such information is
/// prohibited except by express written agreement with Miller Electric Mfg. LLC.
///
#ifndef STM32DOOM_CUBE_LCD_H
#define STM32DOOM_CUBE_LCD_H
#include "stm32h7xx_hal.h"

void LCD_Init(LTDC_HandleTypeDef* hltdc);

void LCD_Write(LTDC_HandleTypeDef* hltdc);

#endif //STM32DOOM_CUBE_LCD_H