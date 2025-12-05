/**
  ******************************************************************************
  * @file    audio_stm32.h
  * @brief   STM32H750 Audio Driver for Doom
  *          Provides I2S audio output via SAI2 peripheral
  ******************************************************************************
  * @attention
  *
  * Audio output configuration:
  * - Sample rate: 44.1 kHz
  * - Format: 16-bit stereo PCM
  * - Interface: I2S via SAI2 peripheral
  * - DMA: Circular double-buffering
  *
  * Pin assignment:
  * - PA0: SAI2_SD_B (I2S Data)
  * - PA2: SAI2_SCK_B (I2S Bit Clock)
  * - PG9: SAI2_FS_A (I2S Frame Sync / Word Select)
  *
  ******************************************************************************
  */

#ifndef AUDIO_STM32_H
#define AUDIO_STM32_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* Audio configuration */
#define AUDIO_SAMPLE_RATE       44100   /* Hz */
#define AUDIO_BUFFER_SIZE       16      /* Samples per half-buffer (stereo) */
#define AUDIO_CHANNELS          2       /* Stereo */

/**
 * @brief Initialize audio system
 *
 * Configures SAI2 peripheral, GPIO pins, and DMA for I2S audio output.
 * Must be called before Audio_Start().
 */
void Audio_Init(void);

/**
 * @brief Start audio playback
 *
 * Begins DMA transfer and enables SAI peripheral.
 * Audio_Init() must be called first.
 */
void Audio_Start(void);

/**
 * @brief Stop audio playback
 *
 * Stops DMA transfer and disables SAI peripheral.
 */
void Audio_Stop(void);

/**
 * @brief Audio mixing callback
 *
 * Called by DMA interrupt when buffer needs refilling.
 * This function mixes sound effects and music into the output buffer.
 *
 * @param buffer Pointer to output buffer (interleaved stereo: L,R,L,R...)
 * @param samples Number of stereo sample pairs
 */
void Audio_MixCallback(int16_t* buffer, int samples);

/**
 * @brief Generate test tone (for debugging)
 *
 * Generates a sine wave test tone at specified frequency.
 *
 * @param buffer Output buffer
 * @param samples Number of stereo sample pairs
 * @param frequency Frequency in Hz
 */
void Audio_GenerateTestTone(int16_t* buffer, int samples, int frequency);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_STM32_H */
