/// @file inputoutput.h
///
#ifndef STM32DOOM_INPUTOUTPUT_H
#define STM32DOOM_INPUTOUTPUT_H

#include <stdint.h>
#include <stdbool.h>
#include "doomkeys.h"

// Encoder Configuration

int32_t getEncoder1Change();
int32_t getEncoder2Change();
bool getEncoder1Button();
bool getEncoder2Button();

// Button Matrix Configuration

#define BUTTON_1        (1U << 2U)
#define BUTTON_2        (1U << 6U)
#define BUTTON_3        (1U << 10U)
#define BUTTON_4        (1U << 14U)
#define BUTTON_5        (1U << 3U)
#define BUTTON_6        (1U << 7U)
#define BUTTON_7        (1U << 11U)
#define BUTTON_8        (1U << 15U)
#define BUTTON_T1       (1U << 0U)
#define BUTTON_T2       (1U << 4U)
#define BUTTON_T3       (1U << 8U)
#define BUTTON_T4       (1U << 12U)
#define BUTTON_T5       (1U << 1U)
#define BUTTON_RUN      (1U << 5U)
#define BUTTON_AUTOMAP  (1U << 9U)

void buttonMatrixScan();
uint16_t getButtonMatrix();
bool getRunLock();

extern const uint8_t button_key_map[16];

// LED Configuration

#define LED_CHAINSAW    0x0001
#define LED_PISTOL      0x0002
#define LED_SHOTGUN     0x0004
#define LED_CHAINGUN    0x0008
#define LED_ROCKET      0x0010
#define LED_PLASMA      0x0020
#define LED_BFG         0x0100

void ledMatrixUpdate();
void ledStatusBar (uint16_t weapons, uint8_t health, uint8_t armor);

#endif //STM32DOOM_INPUTOUTPUT_H
