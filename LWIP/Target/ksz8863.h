/**
  ******************************************************************************
  * @file    ksz8863.h
  * @brief   KSZ8863 3-Port Ethernet Switch Driver
  *          Register definitions and function prototypes
  ******************************************************************************
  */

#ifndef KSZ8863_H
#define KSZ8863_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "main.h"

/* KSZ8863 SPI Command Opcodes */
#define KSZ8863_SPI_READ            0x03
#define KSZ8863_SPI_WRITE           0x02

/* KSZ8863 Register Addresses */
#define KSZ8863_REG_CHIP_ID0        0x00
#define KSZ8863_REG_CHIP_ID1        0x01
#define KSZ8863_REG_GLOBAL_CTRL0    0x02
#define KSZ8863_REG_GLOBAL_CTRL1    0x03
#define KSZ8863_REG_GLOBAL_CTRL2    0x04
#define KSZ8863_REG_GLOBAL_CTRL3    0x05
#define KSZ8863_REG_GLOBAL_CTRL4    0x06
#define KSZ8863_REG_GLOBAL_CTRL5    0x07

/* Port 1 Registers */
#define KSZ8863_REG_PORT1_CTRL0     0x10
#define KSZ8863_REG_PORT1_CTRL1     0x11
#define KSZ8863_REG_PORT1_CTRL2     0x12
#define KSZ8863_REG_PORT1_CTRL3     0x13
#define KSZ8863_REG_PORT1_CTRL4     0x14
#define KSZ8863_REG_PORT1_STATUS0   0x1E
#define KSZ8863_REG_PORT1_STATUS1   0x1F

/* Port 2 Registers */
#define KSZ8863_REG_PORT2_CTRL0     0x20
#define KSZ8863_REG_PORT2_CTRL1     0x21
#define KSZ8863_REG_PORT2_CTRL2     0x22
#define KSZ8863_REG_PORT2_CTRL3     0x23
#define KSZ8863_REG_PORT2_CTRL4     0x24
#define KSZ8863_REG_PORT2_STATUS0   0x2E
#define KSZ8863_REG_PORT2_STATUS1   0x2F

/* Port 3 Registers (Internal MAC port to STM32) */
#define KSZ8863_REG_PORT3_CTRL0     0x30
#define KSZ8863_REG_PORT3_CTRL1     0x31
#define KSZ8863_REG_PORT3_CTRL2     0x32
#define KSZ8863_REG_PORT3_CTRL3     0x33
#define KSZ8863_REG_PORT3_CTRL4     0x34
#define KSZ8863_REG_PORT3_STATUS1   0x3F

/* Reset Register */
#define KSZ8863_REG_RESET           0x43

/* Chip ID Values */
#define KSZ8863_CHIP_ID0_DEFAULT    0x88
#define KSZ8863_CHIP_ID1_FAMILY     0x30

/* Port Control Register Bits */
#define KSZ8863_PORT_CTRL2_TRANSMIT_EN  (1 << 2)
#define KSZ8863_PORT_CTRL2_RECEIVE_EN   (1 << 1)
#define KSZ8863_PORT_CTRL2_LEARNING_DIS (1 << 0)

/* Port Status Register Bits (Register 0x1E for Port1, 0x2E for Port2) */
#define KSZ8863_PORT_STATUS0_MDI_X              (1 << 7)  // MDI-X Status (1=MDI, 0=MDI-X)
#define KSZ8863_PORT_STATUS0_AN_DONE            (1 << 6)  // Auto-negotiation Done
#define KSZ8863_PORT_STATUS0_LINK_GOOD          (1 << 5)  // Link Good
#define KSZ8863_PORT_STATUS0_PARTNER_FLOW_CTRL  (1 << 4)  // Partner Flow Control capable
#define KSZ8863_PORT_STATUS0_PARTNER_100FD      (1 << 3)  // Partner 100BT Full-Duplex
#define KSZ8863_PORT_STATUS0_PARTNER_100HD      (1 << 2)  // Partner 100BT Half-Duplex
#define KSZ8863_PORT_STATUS0_PARTNER_10FD       (1 << 1)  // Partner 10BT Full-Duplex
#define KSZ8863_PORT_STATUS0_PARTNER_10HD       (1 << 0)  // Partner 10BT Half-Duplex

/* Function Prototypes */

/**
 * @brief Initialize KSZ8863 switch
 * @return 0 on success, -1 on error
 */
int32_t KSZ8863_Init(void);

/**
 * @brief Read a register from KSZ8863 via SPI
 * @param reg_addr Register address to read
 * @param value Pointer to store the read value
 * @return 0 on success, -1 on error
 */
int32_t KSZ8863_ReadReg(uint8_t reg_addr, uint8_t *value);

/**
 * @brief Write a register to KSZ8863 via SPI
 * @param reg_addr Register address to write
 * @param value Value to write
 * @return 0 on success, -1 on error
 */
int32_t KSZ8863_WriteReg(uint8_t reg_addr, uint8_t value);

/**
 * @brief Get link status of external ports
 * @return Bit 0: Port 1 link status, Bit 1: Port 2 link status (1=up, 0=down)
 */
uint8_t KSZ8863_GetLinkStatus(void);

/**
 * @brief Perform software reset of KSZ8863
 * @return 0 on success, -1 on error
 */
int32_t KSZ8863_SoftReset(void);

/**
 * @brief Hardware reset of KSZ8863 (if reset pin is connected)
 * @return 0 on success
 */
int32_t KSZ8863_HardReset(void);

#ifdef __cplusplus
}
#endif

#endif /* KSZ8863_H */
