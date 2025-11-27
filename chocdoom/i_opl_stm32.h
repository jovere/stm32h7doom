/**
  ******************************************************************************
  * @file    i_opl_stm32.h
  * @brief   STM32 OPL Backend Interface
  ******************************************************************************
  */

#ifndef I_OPL_STM32_H
#define I_OPL_STM32_H

#include <stdint.h>

/**
 * @brief Generate OPL music samples and mix into audio buffer
 *
 * @param buffer Stereo 16-bit output buffer (L, R, L, R...)
 * @param samples Number of stereo sample pairs
 */
void OPL_STM32_GenerateSamples(int16_t *buffer, int samples);

#endif /* I_OPL_STM32_H */
