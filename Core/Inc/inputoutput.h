/// @file inputoutput.h
///
#ifndef STM32DOOM_INPUTOUTPUT_H
#define STM32DOOM_INPUTOUTPUT_H

#include <stdint.h>
#include <stdbool.h>
#include "doomkeys.h"

int32_t getEncoder1Change();
int32_t getEncoder2Change();
bool getEncoder1Button();
bool getEncoder2Button();

void buttonMatrixScan();
uint16_t getButtonMatrix();

void ledMatrixUpdate();

#define LED_CHAINSAW    0x0001
#define LED_PISTOL      0x0002
#define LED_SHOTGUN     0x0004
#define LED_CHAINGUN    0x0008
#define LED_ROCKET      0x0010
#define LED_PLASMA      0x0020
#define LED_BFG         0x0100

void ledStatusBar (uint16_t weapons, uint8_t health, uint8_t armor);

#endif //STM32DOOM_INPUTOUTPUT_H
