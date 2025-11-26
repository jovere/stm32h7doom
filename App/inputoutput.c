/// @file inputs.cpp
///

#include "inputoutput.h"
#include "main.h"
#include "stm32h7xx_ll_tim.h"
#include "stm32h7xx_ll_spi.h"
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

// Button matrix key mapping
// Maps each of the 16 button positions to a key code
// Button position layout (bit number in getButtonMatrix() result):
//   Bit 0-3:   Column 0, Rows 0-3
//   Bit 4-7:   Column 1, Rows 0-3
//   Bit 8-11:  Column 2, Rows 0-3
//   Bit 12-15: Column 3, Rows 0-3
//
const uint8_t button_key_map[16] = {
    // Column 0 (bits 0-3)
    KEY_ESCAPE,       // Bit 0: Col 0, Row 0
    0,                // Bit 1: Col 0, Row 1
    '1',              // Bit 2: Col 0, Row 2
    '5',              // Bit 3: Col 0, Row 3

    // Column 1 (bits 4-7)
    0,                // Bit 4: Col 1, Row 0
    KEY_RSHIFT,       // Bit 5: Col 1, Row 1
    '2',              // Bit 6: Col 1, Row 2
    '6',              // Bit 7: Col 1, Row 3

    // Column 2 (bits 8-11)
    0,                // Bit 8: Col 2, Row 0
    KEY_TAB,          // Bit 9: Col 2, Row 1
    '3',              // Bit 10: Col 2, Row 2
    '7',              // Bit 11: Col 2, Row 3

    // Column 3 (bits 12-15)
    0,                // Bit 12: Col 3, Row 0
    0,                // Bit 13: Col 3, Row 1
    '4',              // Bit 14: Col 3, Row 2
    '8',              // Bit 15: Col 3, Row 3
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
    // Make the bits active high
    return ~buttonMatrixGlobal;
}

uint8_t leds[4];
int column = 0;
bool toggle = 0;
void ledMatrixUpdate()
{
    if (toggle)
    {
        // Turn off output
        LL_GPIO_SetOutputPin(LED_ENABLE_GPIO_Port, LED_ENABLE_Pin);
        // Transmit new column data
        LL_SPI_TransmitData16(SPI2, (1U << (column+8)) | leds[column] );
        LL_SPI_StartMasterTransfer(SPI2);
        column = (column + 1) & 0x3;
    }
    else
    {
        // Turn on output
        LL_GPIO_ResetOutputPin(LED_ENABLE_GPIO_Port, LED_ENABLE_Pin);
    }
    toggle = !toggle;
}

void ledStatusBar(uint16_t weapons, uint8_t health, uint8_t armor)
{
    leds[2] = weapons & 0xFF;
    leds[3] = (weapons >> 8) & 0xFF;

    // Health bar (6 bits in leds[0] with mask 0x3F)
    // Every 20% health lights another bit, from lowest (100%+) to highest (alive)
    // Calculate number of LEDs to light (0-6)
    uint8_t num_leds = health >= 100 ? 6 : (health > 0 ? health / 20 + 1 : 0);

    // Create LED pattern: shift and mask to get the right number of high bits
    leds[0] = (0x3F << (6 - num_leds)) & 0x3F;

    // Armor bar (5 bits in leds[1] with mask 0x1F)
    // Every 20% armor lights another bit (no LED for armor < 20%)
    // Calculate number of LEDs to light (0-5)
    uint8_t armor_leds_count = armor >= 100 ? 5 : armor / 20;

    // Create LED pattern: shift and mask to get the right number of high bits
    leds[1] = (0x1F << (5 - armor_leds_count)) & 0x1F;

    // Run lock indicator (bit 4 of leds[3])
    if (getRunLock())
    {
        leds[3] |= 0x10;  // Set bit 4
    }
    else
    {
        leds[3] &= ~0x10;  // Clear bit 4
    }
}