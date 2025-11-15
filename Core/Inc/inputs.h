/// @file inputs.h
///
#ifndef STM32DOOM_BUTTONS_H
#define STM32DOOM_BUTTONS_H

#include <stdint.h>
#include <stdbool.h>
#include "doomkeys.h"

int32_t getEncoder1Change();
int32_t getEncoder2Change();
bool getEncoder1Button();
bool getEncoder2Button();

void buttonMatrixScan();
uint16_t getButtonMatrix();

#endif //STM32DOOM_BUTTONS_H
