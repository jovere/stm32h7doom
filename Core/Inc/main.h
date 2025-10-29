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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CS_SPI_ETH_Pin GPIO_PIN_10
#define CS_SPI_ETH_GPIO_Port GPIOG
#define LED3_Pin GPIO_PIN_5
#define LED3_GPIO_Port GPIOD
#define LED4_Pin GPIO_PIN_4
#define LED4_GPIO_Port GPIOD
#define PB_ROW0_Pin GPIO_PIN_10
#define PB_ROW0_GPIO_Port GPIOC
#define CS_SPI_EEPROM_Pin GPIO_PIN_6
#define CS_SPI_EEPROM_GPIO_Port GPIOB
#define LCD_ENABLE_Pin GPIO_PIN_3
#define LCD_ENABLE_GPIO_Port GPIOD
#define PB_ROW2_Pin GPIO_PIN_11
#define PB_ROW2_GPIO_Port GPIOC
#define LED_ENABLE_Pin GPIO_PIN_12
#define LED_ENABLE_GPIO_Port GPIOC
#define LCD_RESET_Pin GPIO_PIN_10
#define LCD_RESET_GPIO_Port GPIOA
#define PB_ROW3_Pin GPIO_PIN_8
#define PB_ROW3_GPIO_Port GPIOC
#define PB_ROW1_Pin GPIO_PIN_9
#define PB_ROW1_GPIO_Port GPIOC
#define LCD_BACKLIGHT_Pin GPIO_PIN_8
#define LCD_BACKLIGHT_GPIO_Port GPIOA
#define USB_POWER_Pin GPIO_PIN_3
#define USB_POWER_GPIO_Port GPIOG
#define PB_COL3_Pin GPIO_PIN_6
#define PB_COL3_GPIO_Port GPIOA
#define PB_COL2_Pin GPIO_PIN_5
#define PB_COL2_GPIO_Port GPIOA
#define CS_SPI_LED_Pin GPIO_PIN_12
#define CS_SPI_LED_GPIO_Port GPIOB
#define PB_COL0_Pin GPIO_PIN_3
#define PB_COL0_GPIO_Port GPIOA
#define PB_COL1_Pin GPIO_PIN_4
#define PB_COL1_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
