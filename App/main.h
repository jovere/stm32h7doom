/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_crs.h"
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_system.h"
#include "stm32h7xx_ll_exti.h"
#include "stm32h7xx_ll_cortex.h"
#include "stm32h7xx_ll_utils.h"
#include "stm32h7xx_ll_pwr.h"
#include "stm32h7xx_ll_dma.h"
#include "stm32h7xx_ll_spi.h"
#include "stm32h7xx_ll_tim.h"
#include "stm32h7xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

extern DMA2D_HandleTypeDef hdma2d;

extern LTDC_HandleTypeDef hltdc;

extern QSPI_HandleTypeDef hqspi;

volatile extern uint32_t systime;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
[[noreturn]] void fatal_error(const char* message);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CS_SPI_ETH_Pin LL_GPIO_PIN_10
#define CS_SPI_ETH_GPIO_Port GPIOG
#define LED3_Pin LL_GPIO_PIN_5
#define LED3_GPIO_Port GPIOD
#define LED4_Pin LL_GPIO_PIN_4
#define LED4_GPIO_Port GPIOD
#define PB_ROW0_Pin LL_GPIO_PIN_10
#define PB_ROW0_GPIO_Port GPIOC
#define CS_SPI_EEPROM_Pin LL_GPIO_PIN_6
#define CS_SPI_EEPROM_GPIO_Port GPIOB
#define LCD_ENABLE_Pin LL_GPIO_PIN_3
#define LCD_ENABLE_GPIO_Port GPIOD
#define PB_ROW2_Pin LL_GPIO_PIN_11
#define PB_ROW2_GPIO_Port GPIOC
#define LED_ENABLE_Pin LL_GPIO_PIN_12
#define LED_ENABLE_GPIO_Port GPIOC
#define LCD_RESET_Pin LL_GPIO_PIN_10
#define LCD_RESET_GPIO_Port GPIOA
#define PB_ROW3_Pin LL_GPIO_PIN_8
#define PB_ROW3_GPIO_Port GPIOC
#define PB_ROW1_Pin LL_GPIO_PIN_9
#define PB_ROW1_GPIO_Port GPIOC
#define LCD_BACKLIGHT_Pin LL_GPIO_PIN_8
#define LCD_BACKLIGHT_GPIO_Port GPIOA
#define USB_POWER_Pin LL_GPIO_PIN_3
#define USB_POWER_GPIO_Port GPIOG
#define PB_COL3_Pin LL_GPIO_PIN_6
#define PB_COL3_GPIO_Port GPIOA
#define PB_COL2_Pin LL_GPIO_PIN_5
#define PB_COL2_GPIO_Port GPIOA
#define ENCODER1_BTN_Pin LL_GPIO_PIN_1
#define ENCODER1_BTN_GPIO_Port GPIOB
#define PB_COL0_Pin LL_GPIO_PIN_3
#define PB_COL0_GPIO_Port GPIOA
#define PB_COL1_Pin LL_GPIO_PIN_4
#define PB_COL1_GPIO_Port GPIOA
#define ENCODER0_BTN_Pin LL_GPIO_PIN_0
#define ENCODER0_BTN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
