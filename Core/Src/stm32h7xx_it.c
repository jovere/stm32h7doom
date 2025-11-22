/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32h7xx_it.c
  * @brief   Interrupt Service Routines.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32h7xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "inputoutput.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

volatile uint32_t systime;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
#if 0
  // Capture fault information
  uint32_t cfsr = SCB->CFSR;
  uint32_t hfsr = SCB->HFSR;
  uint32_t dfsr = SCB->DFSR;
  uint32_t afsr = SCB->AFSR;
  uint32_t mmar = SCB->MMFAR;
  uint32_t bfar = SCB->BFAR;

  printf("\n=== HARD FAULT ===\n");
  printf("CFSR (Configurable Fault Status Register): 0x%08X\n", cfsr);
  printf("HFSR (HardFault Status Register): 0x%08X\n", hfsr);
  printf("DFSR (Debug Fault Status Register): 0x%08X\n", dfsr);
  printf("AFSR (Auxiliary Fault Status Register): 0x%08X\n", afsr);
  printf("MMFAR (MemManage Fault Address): 0x%08X\n", mmar);
  printf("BFAR (Bus Fault Address): 0x%08X\n", bfar);

  // Decode CFSR
  if (cfsr & 0x00FF) {
    printf("\nMemory Management Fault detected:\n");
    if (cfsr & (1 << 0)) printf("  - IACCVIOL: Instruction access violation\n");
    if (cfsr & (1 << 1)) printf("  - DACCVIOL: Data access violation\n");
    if (cfsr & (1 << 3)) printf("  - MUNSTKERR: Unstacking error\n");
    if (cfsr & (1 << 4)) printf("  - MSTKERR: Stacking error\n");
    if (cfsr & (1 << 5)) printf("  - MLSPERR: Lazy FP state preservation error\n");
  }
  if (cfsr & 0xFF00) {
    printf("\nBus Fault detected:\n");
    if (cfsr & (1 << 8)) printf("  - IBUSERR: Instruction bus error\n");
    if (cfsr & (1 << 9)) printf("  - PRECISERR: Precise data bus error\n");
    if (cfsr & (1 << 10)) printf("  - IMPRECISERR: Imprecise data bus error\n");
    if (cfsr & (1 << 11)) printf("  - UNSTKERR: Unstacking error\n");
    if (cfsr & (1 << 12)) printf("  - STKERR: Stacking error\n");
    if (cfsr & (1 << 13)) printf("  - LSPERR: Lazy FP state preservation error\n");
  }
  if (cfsr & 0xFF0000) {
    printf("\nUsage Fault detected:\n");
    if (cfsr & (1 << 16)) printf("  - UNDEFINSTR: Undefined instruction\n");
    if (cfsr & (1 << 17)) printf("  - INVSTATE: Invalid state\n");
    if (cfsr & (1 << 18)) printf("  - INVPC: Invalid PC\n");
    if (cfsr & (1 << 19)) printf("  - NOCP: No coprocessor\n");
    if (cfsr & (1 << 24)) printf("  - UNALIGNED: Unaligned memory access\n");
    if (cfsr & (1 << 25)) printf("  - DIVBYZERO: Division by zero\n");
  }
  if (hfsr & (1 << 30)) printf("\nForced Hard Fault (exception escalation)\n");
  if (hfsr & (1 << 1)) printf("\nVector Table Hard Fault\n");
#endif
  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
    systime++;
  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */
    buttonMatrixScan();
    ledMatrixUpdate();

    // Check for reset combination (T1 + T5) - works at any time
    static uint8_t reset_counter = 0;
    uint16_t buttons = getButtonMatrix();
    if ((buttons & (BUTTON_T1 | BUTTON_T5)) == (BUTTON_T1 | BUTTON_T5))
    {
        reset_counter++;
        if (reset_counter > 10)  // Hold for ~10ms to debounce
        {
            NVIC_SystemReset();
        }
    }
    else
    {
        reset_counter = 0;
    }
  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h7xx.s).                    */
/******************************************************************************/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
