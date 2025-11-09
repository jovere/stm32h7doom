/**
 ******************************************************************************
 * @file    mt25ql512abb.h
 * @brief   MT25QL512ABB QSPI Flash driver header
 *          Minimal implementation for memory-mapped read-only access
 ******************************************************************************
 */

#ifndef MT25QL512ABB_H
#define MT25QL512ABB_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Flash size and memory organization */
#define MT25QL512ABB_FLASH_SIZE              (uint32_t)(512*1024*1024/8)  /* 512 Mbits => 64 MBytes */
#define MT25QL512ABB_SECTOR_64K              (uint32_t)(64 * 1024)        /* 1024 sectors of 64KBytes */
#define MT25QL512ABB_SUBSECTOR_32K           (uint32_t)(32 * 1024)        /* 2048 subsectors of 32KBytes */
#define MT25QL512ABB_SUBSECTOR_4K            (uint32_t)(4  * 1024)        /* 16384 subsectors of 4KBytes */
#define MT25QL512ABB_PAGE_SIZE               (uint32_t)256                /* 262144 pages of 256 Bytes */

/* Error codes */
#define MT25QL512ABB_OK                      (0)
#define MT25QL512ABB_ERROR                   (-1)

/* Flash Commands - Identification */
#define MT25QL512ABB_READ_ID_CMD             0x9FU   /* Read ID */
#define MT25QL512ABB_READ_STATUS_REG_CMD     0x05U   /* Read Status Register */

/* Flash Commands - Read Memory (1-1-1 mode - default) */
#define MT25QL512ABB_READ_CMD                0x03U   /* Normal Read 3/4 Byte Address */
#define MT25QL512ABB_FAST_READ_CMD           0x0BU   /* Fast Read 3/4 Byte Address */

/* Flash Commands - Read Memory (1-1-4 mode) */
#define MT25QL512ABB_1I4O_FAST_READ_CMD      0x6BU   /* Quad Output Fast Read 3/4 Byte Address */
#define MT25QL512ABB_4_BYTE_ADDR_1I4O_FAST_READ_CMD  0x6CU   /* Quad Output Fast Read 4 Byte address */

/* Flash Commands - 4-Byte Addressing */
#define MT25QL512ABB_ENTER_4_BYTE_ADDR_MODE_CMD  0xB7U   /* Enter 4 Byte Address Mode */
#define MT25QL512ABB_4_BYTE_ADDR_READ_CMD        0x13U   /* Normal Read 4 Byte address */
#define MT25QL512ABB_4_BYTE_ADDR_FAST_READ_CMD   0x0CU   /* Fast Read 4 Byte address */

/* Status Register Bits */
#define MT25QL512ABB_SR_WIP                  (0x01U)  /* Write In Progress bit */
#define MT25QL512ABB_SR_WEL                  (0x02U)  /* Write Enable Latch bit */

/* Timing constants */
#define MT25QL512ABB_AUTOPOLLING_INTERVAL_TIME  0x10U
#define MT25QL512ABB_TIMEOUT                    1000U

/* Interface modes */
typedef enum {
    MT25QL512ABB_SPI_MODE = 0,          /* 1-1-1 commands */
    MT25QL512ABB_SPI_1I2O_MODE,         /* 1-1-2 commands */
    MT25QL512ABB_SPI_2IO_MODE,          /* 1-2-2 commands */
    MT25QL512ABB_SPI_1I4O_MODE,         /* 1-1-4 commands */
    MT25QL512ABB_SPI_4IO_MODE,          /* 1-4-4 commands */
    MT25QL512ABB_DPI_MODE,              /* 2-2-2 commands */
    MT25QL512ABB_QPI_MODE,              /* 4-4-4 commands */
} MT25QL512ABB_Interface_t;

/* Address size modes */
typedef enum {
    MT25QL512ABB_3BYTES_SIZE = 0,
    MT25QL512ABB_4BYTES_SIZE = 1,
} MT25QL512ABB_AddressSize_t;

/* Dual flash mode */
typedef enum {
    MT25QL512ABB_DUALFLASH_DISABLE = 0,
    MT25QL512ABB_DUALFLASH_ENABLE = 1,
} MT25QL512ABB_DualFlash_t;

/* Flash information structure */
typedef struct {
    uint32_t FlashSize;
    uint32_t EraseSectorSize;
    uint32_t EraseSectorsNumber;
    uint32_t EraseSubSectorSize;
    uint32_t EraseSubSectorNumber;
    uint32_t EraseSubSector1Size;
    uint32_t EraseSubSector1Number;
    uint32_t ProgPageSize;
    uint32_t ProgPagesNumber;
} MT25QL512ABB_Info_t;

/* Function prototypes */
int32_t MT25QL512ABB_GetFlashInfo(MT25QL512ABB_Info_t *pInfo);
int32_t MT25QL512ABB_AutoPollingMemReady(QSPI_HandleTypeDef *Ctx, MT25QL512ABB_Interface_t Mode, MT25QL512ABB_DualFlash_t DualFlash);
int32_t MT25QL512ABB_Enter4BytesAddressMode(QSPI_HandleTypeDef *Ctx, MT25QL512ABB_Interface_t Mode);
int32_t MT25QL512ABB_EnableMemoryMappedModeSTR(QSPI_HandleTypeDef *Ctx, MT25QL512ABB_Interface_t Mode, MT25QL512ABB_AddressSize_t AddressSize);

#ifdef __cplusplus
}
#endif

#endif /* MT25QL512ABB_H */
