/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "quadspi.h"
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

QSPI_HandleTypeDef hqspi;

SDRAM_HandleTypeDef hsdram1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_FMC_Init(void);
static void MX_QUADSPI_Init(void);
/* USER CODE BEGIN PFP */
void ITM_Init(void);
HAL_StatusTypeDef SDRAM_Test(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define SDRAM_TEST_BASE     ((uint32_t)0xD0000000)
#define SDRAM_TEST_SIZE     (1024 * 1024 * 64)  // 64 MB test
#define SDRAM_TEST_PATTERN1 0xAAAAAAAA
#define SDRAM_TEST_PATTERN2 0x55555555

/**
  * @brief SDRAM Memory Test
  * Tests basic read/write functionality with different patterns
  * @retval HAL_OK if all tests pass, HAL_ERROR if any test fails
  */
HAL_StatusTypeDef SDRAM_Test(void)
{
    uint32_t *pAddr;
    uint32_t data;
    uint32_t addr;
    uint32_t errors = 0;
    uint32_t test_size = SDRAM_TEST_SIZE;

    printf("\n========== SDRAM Test Start ==========\n");
    printf("Test Base Address: 0x%08lX\n", SDRAM_TEST_BASE);
    printf("Test Size: %lu bytes (%lu KB)\n", test_size, test_size / 1024);

    // Test 1: Fill with pattern 0xAAAAAAAA
    printf("\n[Test 1] Writing pattern 0x%08X...", SDRAM_TEST_PATTERN1);
    pAddr = (uint32_t *)SDRAM_TEST_BASE;
    for (addr = 0; addr < test_size; addr += 4)
    {
        *pAddr++ = SDRAM_TEST_PATTERN1;
    }
    printf(" Done\n");

    // Test 2: Verify pattern 0xAAAAAAAA
    printf("[Test 2] Verifying pattern 0x%08X...", SDRAM_TEST_PATTERN1);
    pAddr = (uint32_t *)SDRAM_TEST_BASE;
    errors = 0;
    for (addr = 0; addr < test_size; addr += 4)
    {
        data = *pAddr++;
        if (data != SDRAM_TEST_PATTERN1)
        {
            errors++;
            if (errors <= 5)  // Show first 5 errors
            {
                printf("\nError at 0x%08lX: expected 0x%08X, got 0x%08lX",
                       SDRAM_TEST_BASE + addr, SDRAM_TEST_PATTERN1, data);
            }
        }
    }
    if (errors == 0)
        printf(" Pass\n");
    else
        printf("\nFailed: %lu errors\n", errors);

    if (errors > 0)
        return HAL_ERROR;

    // Test 3: Fill with pattern 0x55555555
    printf("[Test 3] Writing pattern 0x%08X...", SDRAM_TEST_PATTERN2);
    pAddr = (uint32_t *)SDRAM_TEST_BASE;
    for (addr = 0; addr < test_size; addr += 4)
    {
        *pAddr++ = SDRAM_TEST_PATTERN2;
    }
    printf(" Done\n");

    // Test 4: Verify pattern 0x55555555
    printf("[Test 4] Verifying pattern 0x%08X...", SDRAM_TEST_PATTERN2);
    pAddr = (uint32_t *)SDRAM_TEST_BASE;
    errors = 0;
    for (addr = 0; addr < test_size; addr += 4)
    {
        data = *pAddr++;
        if (data != SDRAM_TEST_PATTERN2)
        {
            errors++;
            if (errors <= 5)
            {
                printf("\nError at 0x%08lX: expected 0x%08X, got 0x%08lX",
                       SDRAM_TEST_BASE + addr, SDRAM_TEST_PATTERN2, data);
            }
        }
    }
    if (errors == 0)
        printf(" Pass\n");
    else
        printf("\nFailed: %lu errors\n", errors);

    if (errors > 0)
        return HAL_ERROR;

    // Test 5: Walking ones (address line test)
    printf("[Test 5] Walking ones pattern test...");
    pAddr = (uint32_t *)SDRAM_TEST_BASE;
    errors = 0;
    for (addr = 0; addr < test_size; addr += 4)
    {
        uint32_t pattern = 1 << (addr >> 2) % 32;  // Rotate through bits
        *pAddr++ = pattern;
    }

    pAddr = (uint32_t *)SDRAM_TEST_BASE;
    for (addr = 0; addr < test_size; addr += 4)
    {
        uint32_t pattern = 1 << (addr >> 2) % 32;
        data = *pAddr++;
        if (data != pattern)
            errors++;
    }

    if (errors == 0)
        printf(" Pass\n");
    else
        printf(" Failed: %lu errors\n", errors);

    if (errors > 0)
        return HAL_ERROR;

    // Test 6: Sequential incrementing test
    printf("[Test 6] Sequential write/read test...");
    pAddr = (uint32_t *)SDRAM_TEST_BASE;
    for (addr = 0; addr < test_size; addr += 4)
    {
        *pAddr++ = addr;
    }

    pAddr = (uint32_t *)SDRAM_TEST_BASE;
    errors = 0;
    for (addr = 0; addr < test_size; addr += 4)
    {
        data = *pAddr++;
        if (data != addr)
            errors++;
    }

    if (errors == 0)
        printf(" Pass\n");
    else
        printf(" Failed: %lu errors\n", errors);

    if (errors > 0)
        return HAL_ERROR;

    printf("\n========== SDRAM Test Complete ==========\n");
    printf("Result: ALL TESTS PASSED\n\n");

    return HAL_OK;
}

/*
 * Show fatal error message and stop in endless loop
 */
void fatal_error (const char* message)
{
    printf ("FATAL ERROR: %s\n", message);

    while (1)
    {
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FMC_Init();
  MX_QUADSPI_Init();
  /* USER CODE BEGIN 2 */
    //ITM_Init();

    // Run SDRAM test
    if (SDRAM_Test() == HAL_OK)
    {
        printf("SDRAM is stable and working correctly!\n");
    }
    else
    {
        printf("SDRAM test failed! Check timing parameters.\n");
    }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    // Copy from QSPI to SDRAM
    memcpy((void*)0xD0000000, (void*)0x90000000, 2 * 1024 * 1024);

    // Invalidate caches
    SCB_InvalidateICache();
    SCB_InvalidateDCache();

    // Read vector table from SDRAM
    uint32_t *vector_table = (uint32_t*)0xD0000000;
    uint32_t stack_pointer = vector_table[0];
    uint32_t reset_handler = vector_table[1];

    // Disable interrupts before jump
    __disable_irq();

    SCB->VTOR = 0xD0000000;  // Point VTOR to SDRAM vector table

    // Set stack pointer
    __set_MSP(stack_pointer);

    // Jump to Reset_Handler
    ((void(*)(void))reset_handler)();

  uint32_t oldtime = systime;
  while (1)
  {
      uint32_t newsystime = systime;
      if (newsystime - oldtime >= 250)
      {
       //   ITM->PORT[0].u8 = 'A';
          printf("This is a test: %lu\n", newsystime);
          oldtime = newsystime;
       }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_2)
  {
  }
  LL_PWR_ConfigSupply(LL_PWR_LDO_SUPPLY);
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  while (LL_PWR_IsActiveFlag_VOS() == 0)
  {
  }
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {

  }
  LL_RCC_PLL_SetSource(LL_RCC_PLLSOURCE_HSE);
  LL_RCC_PLL1P_Enable();
  LL_RCC_PLL1R_Enable();
  LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_4_8);
  LL_RCC_PLL1_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
  LL_RCC_PLL1_SetM(5);
  LL_RCC_PLL1_SetN(160);
  LL_RCC_PLL1_SetP(2);
  LL_RCC_PLL1_SetQ(8);
  LL_RCC_PLL1_SetR(2);
  LL_RCC_PLL1_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL1_IsReady() != 1)
  {
  }

   /* Intermediate AHB prescaler 2 when target frequency clock is higher than 80 MHz */
   LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_2);

  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL1)
  {

  }
  LL_RCC_SetSysPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_2);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
  LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_2);
  LL_RCC_SetAPB4Prescaler(LL_RCC_APB4_DIV_2);
  LL_SetSystemCoreClock(400000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief QUADSPI Initialization Function
  * @param None
  * @retval None
  */
static void MX_QUADSPI_Init(void)
{

  /* USER CODE BEGIN QUADSPI_Init 0 */

  /* USER CODE END QUADSPI_Init 0 */

  /* USER CODE BEGIN QUADSPI_Init 1 */

  /* USER CODE END QUADSPI_Init 1 */
  /* QUADSPI parameter configuration*/
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 1;
  hqspi.Init.FifoThreshold = 1;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
  hqspi.Init.FlashSize = 25;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_5_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
  hqspi.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN QUADSPI_Init 2 */

    // Reset the Flash to simplest mode

    QSPI_CommandTypeDef cmd = {0};
    cmd.DummyCycles = 0;
    cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    cmd.Instruction = 0x66;
    if (HAL_QSPI_Command(&hqspi, &cmd, 1000) != HAL_OK)
    {
        printf("QSPI Reset Enable command failed!\n");
    }

    cmd.Instruction = 0x99;
    if (HAL_QSPI_Command(&hqspi, &cmd, 1000) != HAL_OK)
    {
        printf("QSPI Reset Memory command failed!\n");
    }

    cmd.InstructionMode = QSPI_INSTRUCTION_2_LINES;
    cmd.Instruction = 0x66;
    if (HAL_QSPI_Command(&hqspi, &cmd, 1000) != HAL_OK)
    {
        printf("QSPI Reset Enable command failed!\n");
    }

    cmd.Instruction = 0x99;
    if (HAL_QSPI_Command(&hqspi, &cmd, 1000) != HAL_OK)
    {
        printf("QSPI Reset Memory command failed!\n");
    }

    BSP_QSPI_Init_t init_config;

    /* Initialize QSPI flash with safe defaults (1-1-1 mode, STR) */
    init_config.InterfaceMode = BSP_QSPI_SPI_1_1_1_MODE;
    init_config.TransferRate = BSP_QSPI_STR_TRANSFER;
    init_config.DualFlashMode = BSP_QSPI_DUALFLASH_DISABLE;

    /* Initialize the flash */
    if (BSP_QSPI_Init(0, &init_config) != BSP_ERROR_NONE) {
        printf("QSPI Initialization Failed!\n");
    }

    /* Enable memory-mapped mode for read operations */
    if (BSP_QSPI_EnableMemoryMappedMode(0) != BSP_ERROR_NONE) {
        printf("QSPI Enable Memory MappedMode failed!\n");
    }

  /* USER CODE END QUADSPI_Init 2 */

}

/* FMC initialization function */
static void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_SDRAM_TimingTypeDef SdramTiming = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

  /** Perform the SDRAM1 memory initialization sequence
  */
  hsdram1.Instance = FMC_SDRAM_DEVICE;
  /* hsdram1.Init */
  hsdram1.Init.SDBank = FMC_SDRAM_BANK2;
  hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_9;
  hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_13;
  hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_32;
  hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_2;
  hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
  hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
  hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;
  /* SdramTiming */
  SdramTiming.LoadToActiveDelay = 2;
  SdramTiming.ExitSelfRefreshDelay = 7;
  SdramTiming.SelfRefreshTime = 5;
  SdramTiming.RowCycleDelay = 7;
  SdramTiming.WriteRecoveryTime = 2;
  SdramTiming.RPDelay = 3;
  SdramTiming.RCDDelay = 3;

  if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FMC_Init 2 */

  // SDRAM Initialization Sequence Commands
  FMC_SDRAM_CommandTypeDef Command;

  // 1. Clock Configuration Enable (1 command)
  Command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
  Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  Command.AutoRefreshNumber = 1;
  Command.ModeRegisterDefinition = 0;
  if (HAL_SDRAM_SendCommand(&hsdram1, &Command, 0xFFFF) != HAL_OK)
    Error_Handler();

  // 2. Delay for clock stabilization (typically 100 us)
  HAL_Delay(1);

  // 3. Precharge all (1 command)
  Command.CommandMode = FMC_SDRAM_CMD_PALL;
  Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  Command.AutoRefreshNumber = 1;
  Command.ModeRegisterDefinition = 0;
  if (HAL_SDRAM_SendCommand(&hsdram1, &Command, 0xFFFF) != HAL_OK)
    Error_Handler();

  // 4. Auto Refresh (8 commands)
  Command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  Command.AutoRefreshNumber = 8;
  Command.ModeRegisterDefinition = 0;
  if (HAL_SDRAM_SendCommand(&hsdram1, &Command, 0xFFFF) != HAL_OK)
    Error_Handler();

  // 5. Load Mode Register (1 command)
  // Mode Register value for CAS Latency 2, Burst Length 1 (AS4C16M16SA):
  // 0x20 (CAS=2) | 0x00 (BL=1) = 0x20
  Command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  Command.AutoRefreshNumber = 1;
  Command.ModeRegisterDefinition = 0x20;  // CAS Latency 2, Burst Length 1
  if (HAL_SDRAM_SendCommand(&hsdram1, &Command, 0xFFFF) != HAL_OK)
    Error_Handler();

  // 6. Set refresh rate (typically 64ms for normal operation)
  // For 100MHz: 6400 cycles = 64ms
  HAL_SDRAM_ProgramRefreshRate(&hsdram1, 6400);

  // Delay to let SDRAM stabilize
  HAL_Delay(10);

  /* USER CODE END FMC_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOI);
  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);
  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOE);
  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOB);
  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOH);
  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOC);
  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOG);
  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOD);
  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOF);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void ITM_Init(void) {

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_TRACE;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Enable trace in Core Debug
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    // Unlock ITM
    ITM->LAR = 0xC5ACCE55;

    // Enable ITM
    ITM->TCR = (1 << ITM_TCR_ITMENA_Pos) |  // Enable ITM
               (1 << ITM_TCR_SYNCENA_Pos); // Enable sync packets

    // Enable stimulus port 0
    ITM->TER = 0x1;

}


/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{

  /* Disables the MPU */
  LL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  LL_MPU_ConfigRegion(LL_MPU_REGION_NUMBER0, 0x87, 0x0, LL_MPU_REGION_SIZE_4GB|LL_MPU_TEX_LEVEL0|LL_MPU_REGION_NO_ACCESS|LL_MPU_INSTRUCTION_ACCESS_DISABLE|LL_MPU_ACCESS_SHAREABLE|LL_MPU_ACCESS_NOT_CACHEABLE|LL_MPU_ACCESS_NOT_BUFFERABLE);

  /** Initializes and configures the Region and the memory to be protected
  */
  LL_MPU_ConfigRegion(LL_MPU_REGION_NUMBER1, 0x0, 0xD0000000, LL_MPU_REGION_SIZE_64MB|LL_MPU_TEX_LEVEL0|LL_MPU_REGION_FULL_ACCESS|LL_MPU_INSTRUCTION_ACCESS_ENABLE|LL_MPU_ACCESS_NOT_SHAREABLE|LL_MPU_ACCESS_CACHEABLE|LL_MPU_ACCESS_BUFFERABLE);

  /** Initializes and configures the Region and the memory to be protected
  */
  LL_MPU_ConfigRegion(LL_MPU_REGION_NUMBER2, 0x0, 0x90000000, LL_MPU_REGION_SIZE_64MB|LL_MPU_TEX_LEVEL0|LL_MPU_REGION_FULL_ACCESS|LL_MPU_INSTRUCTION_ACCESS_ENABLE|LL_MPU_ACCESS_NOT_SHAREABLE|LL_MPU_ACCESS_CACHEABLE|LL_MPU_ACCESS_BUFFERABLE);

  /** Initializes and configures the Region and the memory to be protected
  */
  LL_MPU_ConfigRegion(LL_MPU_REGION_NUMBER3, 0x0, 0x30000000, LL_MPU_REGION_SIZE_512KB|LL_MPU_TEX_LEVEL1|LL_MPU_REGION_FULL_ACCESS|LL_MPU_INSTRUCTION_ACCESS_DISABLE|LL_MPU_ACCESS_SHAREABLE|LL_MPU_ACCESS_NOT_CACHEABLE|LL_MPU_ACCESS_NOT_BUFFERABLE);
  /* Enables the MPU */
  LL_MPU_Enable(LL_MPU_CTRL_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
