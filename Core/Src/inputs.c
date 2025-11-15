/// @file inputs.cpp
///

#include "inputs.h"
#include "main.h"
#include "stm32h7xx_ll_tim.h"
#include <stdio.h>

#include "stm32h7xx_ll_gpio.h"
static uint16_t lastEncoder1 = 0;
static uint16_t lastEncoder2 = 0;

static uint8_t currentColumn = 0;
static uint16_t buttonMatrixTemp = 0;
static volatile uint16_t buttonMatrixGlobal = 0;

static const uint32_t columnPins[4] = {
    PB_COL0_Pin, PB_COL1_Pin, PB_COL2_Pin, PB_COL3_Pin
};

int32_t getEncoder1Change()
{
    uint16_t encoder = LL_TIM_GetCounter(TIM3);
    int32_t encoder1Value = (int16_t)(encoder - lastEncoder1);
    lastEncoder1 = encoder;
    return encoder1Value;
}

int32_t getEncoder2Change()
{
    uint16_t encoder = LL_TIM_GetCounter(TIM4);
    int32_t encoderValue = (int16_t)(encoder - lastEncoder2);
    lastEncoder2 = encoder;
    return encoderValue;
}

bool getEncoder1Button()
{
    return LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_0);
}

bool getEncoder2Button()
{
    return LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_1);
}

void buttonMatrixScan()
{
    uint32_t portData = LL_GPIO_ReadInputPort(PB_ROW0_GPIO_Port);

    uint8_t rowData = 0;
    rowData |= ((portData >> __builtin_ctz(PB_ROW0_Pin)) & 1) << 0;
    rowData |= ((portData >> __builtin_ctz(PB_ROW1_Pin)) & 1) << 1;
    rowData |= ((portData >> __builtin_ctz(PB_ROW2_Pin)) & 1) << 2;
    rowData |= ((portData >> __builtin_ctz(PB_ROW3_Pin)) & 1) << 3;

    buttonMatrixTemp &= ~(0xF << (currentColumn * 4));
    buttonMatrixTemp |= (rowData & 0xF) << (currentColumn * 4);

    LL_GPIO_SetOutputPin(PB_COL0_GPIO_Port, columnPins[currentColumn]);

    currentColumn = (currentColumn + 1) & 3;

    if (currentColumn == 0) {
        buttonMatrixGlobal = buttonMatrixTemp;
    }

    LL_GPIO_ResetOutputPin(PB_COL0_GPIO_Port, columnPins[currentColumn]);
}

uint16_t getButtonMatrix()
{
    return buttonMatrixGlobal;
}