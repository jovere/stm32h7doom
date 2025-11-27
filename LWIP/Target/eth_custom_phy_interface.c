/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    eth_custom_phy_interface.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage
  *			 the ethernet phy.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Private includes ----------------------------------------------------------*/
#include "eth_custom_phy_interface.h"

/* USER CODE BEGIN Includes */
#include "ksz8863.h"
#include "stm32h7xx_ll_spi.h"
#include "stm32h7xx_ll_bus.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int32_t eth_phy_init(void)
{

/* USER CODE BEGIN PHY_INIT_0 */

/* USER CODE END PHY_INIT_0 */

    int32_t ret = ETH_PHY_STATUS_OK;

/* USER CODE BEGIN PHY_INIT_1 */
    /* Initialize KSZ8863 switch */
    ret = KSZ8863_Init();
    if (ret != 0)
    {
        return ETH_PHY_STATUS_ERROR;
    }

    /* Disable SPI1 - no longer needed after switch configuration
     * This frees pins PD7, PG9, PG11 for reassignment to audio (SAI2) */
    LL_SPI_Disable(SPI1);
    LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_SPI1);
/* USER CODE END PHY_INIT_1 */
    return ret;
}

int32_t eth_phy_get_link_state(void)
{

  /* USER CODE BEGIN LINK_STATE_0 */

  /* USER CODE END LINK_STATE_0 */

  int32_t  linkstate = ETH_PHY_STATUS_LINK_ERROR;

  /* USER CODE BEGIN LINK_STATE_1 */
  uint8_t link_status;
  static uint8_t last_link_status = 0xFF;

  /* Get link status from KSZ8863 */
  link_status = KSZ8863_GetLinkStatus();

  /* Debug: Print when link status changes */
  if (link_status != last_link_status)
  {
      extern int printf(const char *format, ...);
      printf("[PHY] Link status changed: P1=%s P2=%s\n",
             (link_status & 0x01) ? "UP" : "DN",
             (link_status & 0x02) ? "UP" : "DN");
      last_link_status = link_status;
  }

  /* If either Port 1 or Port 2 has link, consider link as up */
  if (link_status & 0x03)
  {
      /* Link is up - report as 100Mbps Full Duplex (typical for RMII) */
      linkstate = ETH_PHY_STATUS_100MBITS_FULLDUPLEX;
  }
  else
  {
      /* No link on any external port */
      linkstate = ETH_PHY_STATUS_LINK_DOWN;
  }
  /* USER CODE END LINK_STATE_1 */
  return linkstate;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
