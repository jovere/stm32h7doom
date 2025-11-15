/// @file inputs.cpp
///

#include "inputs.h"
#include "stm32h7xx_ll_tim.h"
#include <stdio.h>

#include "stm32h7xx_ll_gpio.h"
static uint16_t lastEncoder1 = 0;
static uint16_t lastEncoder2 = 0;

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