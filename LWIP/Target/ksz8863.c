/**
  ******************************************************************************
  * @file    ksz8863.c
  * @brief   KSZ8863 3-Port Ethernet Switch Driver Implementation
  ******************************************************************************
  */

#include "ksz8863.h"
#include "stm32h7xx_ll_spi.h"
#include "stm32h7xx_ll_gpio.h"

/* Private defines */
#define KSZ8863_SPI                 SPI1
#define KSZ8863_CS_PORT             CS_SPI_ETH_GPIO_Port
#define KSZ8863_CS_PIN              CS_SPI_ETH_Pin
#define KSZ8863_RESET_PORT          GPIOC
#define KSZ8863_RESET_PIN           LL_GPIO_PIN_1

#define KSZ8863_SPI_TIMEOUT         1000

/* Private function prototypes */
static void KSZ8863_CS_Low(void);
static void KSZ8863_CS_High(void);
static uint8_t KSZ8863_SPI_TransmitReceive(uint8_t data);
static void KSZ8863_Delay(uint32_t ms);

/**
 * @brief Pull CS low to select KSZ8863
 */
static void KSZ8863_CS_Low(void)
{
    LL_GPIO_ResetOutputPin(KSZ8863_CS_PORT, KSZ8863_CS_PIN);
}

/**
 * @brief Pull CS high to deselect KSZ8863
 */
static void KSZ8863_CS_High(void)
{
    LL_GPIO_SetOutputPin(KSZ8863_CS_PORT, KSZ8863_CS_PIN);
}

/**
 * @brief Transmit and receive one byte via SPI
 * @param data Byte to transmit
 * @return Received byte
 */
static uint8_t KSZ8863_SPI_TransmitReceive(uint8_t data)
{
    uint32_t timeout = KSZ8863_SPI_TIMEOUT;

    /* Enable SPI if not already enabled */
    if (!LL_SPI_IsEnabled(KSZ8863_SPI))
    {
        LL_SPI_Enable(KSZ8863_SPI);
    }

    /* Start transfer if not already started */
    if (!LL_SPI_IsActiveMasterTransfer(KSZ8863_SPI))
    {
        LL_SPI_StartMasterTransfer(KSZ8863_SPI);
    }

    /* Wait for TXP (TX Packet space available) */
    timeout = KSZ8863_SPI_TIMEOUT;
    while (!LL_SPI_IsActiveFlag_TXP(KSZ8863_SPI))
    {
        if (--timeout == 0) return 0xFF;
    }

    /* Transmit data */
    LL_SPI_TransmitData8(KSZ8863_SPI, data);

    /* Wait for RXP (RX Packet available) */
    timeout = KSZ8863_SPI_TIMEOUT;
    while (!LL_SPI_IsActiveFlag_RXP(KSZ8863_SPI))
    {
        if (--timeout == 0) return 0xFF;
    }

    /* Read received data */
    return LL_SPI_ReceiveData8(KSZ8863_SPI);
}

/**
 * @brief Simple delay function
 * @param ms Milliseconds to delay
 */
static void KSZ8863_Delay(uint32_t ms)
{
    HAL_Delay(ms);
}

/**
 * @brief Hardware reset of KSZ8863
 */
int32_t KSZ8863_HardReset(void)
{
    /* Pull reset low for 10ms */
    LL_GPIO_ResetOutputPin(KSZ8863_RESET_PORT, KSZ8863_RESET_PIN);
    KSZ8863_Delay(10);

    /* Release reset */
    LL_GPIO_SetOutputPin(KSZ8863_RESET_PORT, KSZ8863_RESET_PIN);

    /* Wait for switch to boot up (typically ~100ms) */
    KSZ8863_Delay(150);

    return 0;
}

/**
 * @brief Read a register from KSZ8863 via SPI
 */
int32_t KSZ8863_ReadReg(uint8_t reg_addr, uint8_t *value)
{
    volatile uint32_t delay;

    if (value == NULL) return -1;

    /* Enable SPI if not already enabled */
    if (!LL_SPI_IsEnabled(KSZ8863_SPI))
    {
        LL_SPI_Enable(KSZ8863_SPI);
    }

    KSZ8863_CS_Low();

    /* Small delay after CS assertion for setup time */
    for (delay = 0; delay < 10; delay++) __NOP();

    /* Send read command (opcode: 0b00000011) */
    KSZ8863_SPI_TransmitReceive(KSZ8863_SPI_READ);

    /* Send register address */
    KSZ8863_SPI_TransmitReceive(reg_addr);

    /* Read data */
    *value = KSZ8863_SPI_TransmitReceive(0x00);

    KSZ8863_CS_High();

    /* Small delay after CS de-assertion */
    for (delay = 0; delay < 10; delay++) __NOP();

    return 0;
}

/**
 * @brief Write a register to KSZ8863 via SPI
 */
int32_t KSZ8863_WriteReg(uint8_t reg_addr, uint8_t value)
{
    /* Enable SPI if not already enabled */
    if (!LL_SPI_IsEnabled(KSZ8863_SPI))
    {
        LL_SPI_Enable(KSZ8863_SPI);
    }

    KSZ8863_CS_Low();

    /* Send write command (opcode: 0b00000010) */
    KSZ8863_SPI_TransmitReceive(KSZ8863_SPI_WRITE);

    /* Send register address */
    KSZ8863_SPI_TransmitReceive(reg_addr);

    /* Send data */
    KSZ8863_SPI_TransmitReceive(value);

    KSZ8863_CS_High();

    return 0;
}

/**
 * @brief Perform software reset of KSZ8863
 */
int32_t KSZ8863_SoftReset(void)
{
    int32_t ret;

    /* Write to reset register */
    ret = KSZ8863_WriteReg(KSZ8863_REG_RESET, 0x11);
    if (ret != 0) return ret;

    /* Wait for reset to complete */
    KSZ8863_Delay(10);

    return 0;
}

/**
 * @brief Get link status of external ports
 */
uint8_t KSZ8863_GetLinkStatus(void)
{
    uint8_t status0_p1 = 0, status0_p2 = 0;
    uint8_t link_status = 0;

    /* Read Port 1 status (using correct register address 0x1E) */
    if (KSZ8863_ReadReg(KSZ8863_REG_PORT1_STATUS0, &status0_p1) == 0)
    {
        /* Note: Bit definition for link status needs to be verified from datasheet */
        /* Common bits: bit 5 or bit 0 for link status */
        if (status0_p1 & KSZ8863_PORT_STATUS0_LINK_GOOD)  // May need adjustment
        {
            link_status |= (1 << 0);
        }
    }

    /* Read Port 2 status (using correct register address 0x2E) */
    if (KSZ8863_ReadReg(KSZ8863_REG_PORT2_STATUS0, &status0_p2) == 0)
    {
        if (status0_p2 & KSZ8863_PORT_STATUS0_LINK_GOOD)  // May need adjustment
        {
            link_status |= (1 << 1);
        }
    }

    return link_status;
}

/**
 * @brief Initialize KSZ8863 switch
 */
int32_t KSZ8863_Init(void)
{
    uint8_t chip_id0 = 0, chip_id1 = 0;
    uint8_t reg_value = 0;
    int32_t ret;

    /* Ensure CS is high initially */
    KSZ8863_CS_High();
    KSZ8863_Delay(10);

    /* Perform hardware reset */
    KSZ8863_HardReset();

    /* Verify chip ID */
    ret = KSZ8863_ReadReg(KSZ8863_REG_CHIP_ID0, &chip_id0);
    if (ret != 0) return -1;

    ret = KSZ8863_ReadReg(KSZ8863_REG_CHIP_ID1, &chip_id1);
    if (ret != 0) return -1;

    /* Check if chip ID matches expected values */
    if ((chip_id0 != KSZ8863_CHIP_ID0_DEFAULT) ||
        ((chip_id1 & 0xF0) != KSZ8863_CHIP_ID1_FAMILY))
    {
        /* Chip ID mismatch */
        return -1;
    }

    /* Basic switch configuration */

    /* Configure Port 3 (Internal MAC port to STM32) */
    /* Enable transmit and receive, enable learning */
    ret = KSZ8863_ReadReg(KSZ8863_REG_PORT3_CTRL2, &reg_value);
    if (ret != 0) return -1;

    reg_value |= KSZ8863_PORT_CTRL2_TRANSMIT_EN;
    reg_value |= KSZ8863_PORT_CTRL2_RECEIVE_EN;
    reg_value &= ~KSZ8863_PORT_CTRL2_LEARNING_DIS; /* Enable learning */

    ret = KSZ8863_WriteReg(KSZ8863_REG_PORT3_CTRL2, reg_value);
    if (ret != 0) return -1;

    /* Configure Port 1 (External port) */
    ret = KSZ8863_ReadReg(KSZ8863_REG_PORT1_CTRL2, &reg_value);
    if (ret != 0) return -1;

    reg_value |= KSZ8863_PORT_CTRL2_TRANSMIT_EN;
    reg_value |= KSZ8863_PORT_CTRL2_RECEIVE_EN;
    reg_value &= ~KSZ8863_PORT_CTRL2_LEARNING_DIS;

    ret = KSZ8863_WriteReg(KSZ8863_REG_PORT1_CTRL2, reg_value);
    if (ret != 0) return -1;

    /* Configure Port 2 (External port) */
    ret = KSZ8863_ReadReg(KSZ8863_REG_PORT2_CTRL2, &reg_value);
    if (ret != 0) return -1;

    reg_value |= KSZ8863_PORT_CTRL2_TRANSMIT_EN;
    reg_value |= KSZ8863_PORT_CTRL2_RECEIVE_EN;
    reg_value &= ~KSZ8863_PORT_CTRL2_LEARNING_DIS;

    ret = KSZ8863_WriteReg(KSZ8863_REG_PORT2_CTRL2, reg_value);
    if (ret != 0) return -1;

    /* Enable switch engine */
    ret = KSZ8863_WriteReg(KSZ8863_REG_CHIP_ID0, (1 << 0));
    if (ret != 0) return -1;

    return 0;
}
