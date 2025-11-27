/**
  ******************************************************************************
  * @file    i_swap_stm32.h
  * @brief   Byte-swap functions for STM32 (SDL compatibility)
  ******************************************************************************
  */

#ifndef I_SWAP_STM32_H
#define I_SWAP_STM32_H

#include <stdint.h>

/**
 * @brief Swap 16-bit big-endian to native (little-endian on ARM)
 */
static inline uint16_t SDL_SwapBE16(uint16_t x)
{
    return __builtin_bswap16(x);
}

/**
 * @brief Swap 32-bit big-endian to native (little-endian on ARM)
 */
static inline uint32_t SDL_SwapBE32(uint32_t x)
{
    return __builtin_bswap32(x);
}

#endif /* I_SWAP_STM32_H */
