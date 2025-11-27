/**
  ******************************************************************************
  * @file    audio_stm32.c
  * @brief   STM32H750 Audio Driver Implementation
  ******************************************************************************
  */

#include "audio_stm32.h"
#include "stm32h7xx_hal.h"
#include <string.h>
#include <math.h>

/* Private defines */
#define AUDIO_DMA_BUFFER_SIZE   (AUDIO_BUFFER_SIZE * AUDIO_CHANNELS * 2)  /* Double buffer */

/* Private variables */
static SAI_HandleTypeDef hsai2;
static DMA_HandleTypeDef hdma_sai2_b;

/* DMA audio buffer in SDRAM (interleaved stereo: L,R,L,R...) */
static int16_t audio_buffer[AUDIO_DMA_BUFFER_SIZE] __attribute__((section(".sdram")));

/* Forward declarations */
static void Audio_GPIO_Init(void);
static void Audio_SAI_Init(void);
static void Audio_DMA_Init(void);

/**
 * @brief Initialize GPIO pins for SAI2
 */
static void Audio_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    /* Configure PA0: SAI2_SD_B (Serial Data) */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Configure PA2: SAI2_SCK_B (Bit Clock) */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Configure PG9: SAI2_FS_A (Frame Sync / Word Select) */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

/**
 * @brief Initialize SAI2 peripheral for I2S mode
 */
static void Audio_SAI_Init(void)
{
    /* Enable SAI2 clock */
    __HAL_RCC_SAI2_CLK_ENABLE();

    /* Configure SAI2 Block B as I2S master transmitter */
    hsai2.Instance = SAI2_Block_B;
    hsai2.Init.AudioMode = SAI_MODEMASTER_TX;
    hsai2.Init.Synchro = SAI_ASYNCHRONOUS;
    hsai2.Init.OutputDrive = SAI_OUTPUTDRIVE_ENABLE;
    hsai2.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
    hsai2.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;
    hsai2.Init.AudioFrequency = AUDIO_SAMPLE_RATE;
    hsai2.Init.Protocol = SAI_FREE_PROTOCOL;
    hsai2.Init.DataSize = SAI_DATASIZE_16;
    hsai2.Init.FirstBit = SAI_FIRSTBIT_MSB;
    hsai2.Init.ClockStrobing = SAI_CLOCKSTROBING_FALLINGEDGE;

    /* I2S frame configuration */
    hsai2.FrameInit.FrameLength = 64;  /* 32 bits per channel × 2 channels */
    hsai2.FrameInit.ActiveFrameLength = 32;
    hsai2.FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
    hsai2.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
    hsai2.FrameInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;

    /* Slot configuration (stereo) */
    hsai2.SlotInit.FirstBitOffset = 0;
    hsai2.SlotInit.SlotSize = SAI_SLOTSIZE_16B;
    hsai2.SlotInit.SlotNumber = 2;  /* 2 slots for stereo */
    hsai2.SlotInit.SlotActive = SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1;

    if (HAL_SAI_Init(&hsai2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief Initialize DMA for SAI2
 */
static void Audio_DMA_Init(void)
{
    /* Enable DMA1 clock */
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* Configure DMA for SAI2_B transmission */
    hdma_sai2_b.Instance = DMA1_Stream4;
    hdma_sai2_b.Init.Request = DMA_REQUEST_SAI2_B;
    hdma_sai2_b.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_sai2_b.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sai2_b.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sai2_b.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_sai2_b.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_sai2_b.Init.Mode = DMA_CIRCULAR;
    hdma_sai2_b.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_sai2_b.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(&hdma_sai2_b) != HAL_OK)
    {
        Error_Handler();
    }

    /* Link DMA handle to SAI */
    __HAL_LINKDMA(&hsai2, hdmatx, hdma_sai2_b);

    /* Configure DMA interrupt */
    HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
}

/**
 * @brief Initialize audio system
 */
void Audio_Init(void)
{
    /* Initialize peripherals in order */
    Audio_GPIO_Init();
    Audio_DMA_Init();
    Audio_SAI_Init();

    /* Clear audio buffer */
    memset(audio_buffer, 0, sizeof(audio_buffer));
}

/**
 * @brief Start audio playback
 */
void Audio_Start(void)
{
    /* Start DMA transmission in circular mode */
    HAL_SAI_Transmit_DMA(&hsai2, (uint8_t*)audio_buffer, AUDIO_DMA_BUFFER_SIZE);
}

/**
 * @brief Stop audio playback
 */
void Audio_Stop(void)
{
    HAL_SAI_DMAStop(&hsai2);
}

/**
 * @brief DMA half-transfer complete callback
 *
 * Called when first half of buffer has been transmitted.
 * Fill first half with new audio data.
 */
void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai->Instance == SAI2_Block_B)
    {
        /* Fill first half of buffer (samples 0 to AUDIO_BUFFER_SIZE-1) */
        Audio_MixCallback(&audio_buffer[0], AUDIO_BUFFER_SIZE);
    }
}

/**
 * @brief DMA transfer complete callback
 *
 * Called when second half of buffer has been transmitted.
 * Fill second half with new audio data.
 */
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai->Instance == SAI2_Block_B)
    {
        /* Fill second half of buffer (samples AUDIO_BUFFER_SIZE to end) */
        Audio_MixCallback(&audio_buffer[AUDIO_BUFFER_SIZE * AUDIO_CHANNELS], AUDIO_BUFFER_SIZE);
    }
}

/**
 * @brief Audio mixing callback (weak implementation)
 *
 * Default implementation fills buffer with silence.
 * Override this function in Doom sound backend.
 */
__weak void Audio_MixCallback(int16_t* buffer, int samples)
{
    /* Fill with silence */
    memset(buffer, 0, samples * AUDIO_CHANNELS * sizeof(int16_t));
}

/**
 * @brief Generate test tone
 *
 * Generates a sine wave at specified frequency for testing.
 */
void Audio_GenerateTestTone(int16_t* buffer, int samples, int frequency)
{
    static float phase = 0.0f;
    float phase_increment = (2.0f * 3.14159265f * frequency) / (float)AUDIO_SAMPLE_RATE;

    for (int i = 0; i < samples; i++)
    {
        /* Generate sine wave sample */
        int16_t sample = (int16_t)(sinf(phase) * 16000.0f);

        /* Stereo output */
        buffer[i * 2 + 0] = sample;  /* Left */
        buffer[i * 2 + 1] = sample;  /* Right */

        /* Advance phase */
        phase += phase_increment;
        if (phase > 2.0f * 3.14159265f)
        {
            phase -= 2.0f * 3.14159265f;
        }
    }
}

/**
 * @brief DMA1 Stream4 interrupt handler
 *
 * Handles SAI2_B DMA interrupts.
 */
void DMA1_Stream4_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_sai2_b);
}
