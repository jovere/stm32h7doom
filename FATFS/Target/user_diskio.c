/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * @file    user_diskio.c
  * @brief   This file includes a diskio driver skeleton to be completed by the user.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
 /* USER CODE END Header */

#ifdef USE_OBSOLETE_USER_CODE_SECTION_0
/*
 * Warning: the user section 0 is no more in use (starting from CubeMx version 4.16.0)
 * To be suppressed in the future.
 * Kept to ensure backward compatibility with previous CubeMx versions when
 * migrating projects.
 * User code previously added there should be copied in the new user sections before
 * the section contents can be deleted.
 */
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
#endif

/* USER CODE BEGIN DECL */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "ff_gen_drv.h"
#include "quadspi.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* FATFS sector size */
#define FATFS_SECTOR_SIZE           512

/* QSPI memory mapped base address */
#define QSPI_FLASH_BASE_ADDR        0x90000000

/* FATFS filesystem offset in flash (2MB for application code) */
#define FATFS_FLASH_OFFSET          (2 * 1024 * 1024)

/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;

/* USER CODE END DECL */

/* Private function prototypes -----------------------------------------------*/
DSTATUS USER_initialize (BYTE pdrv);
DSTATUS USER_status (BYTE pdrv);
DRESULT USER_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
#if _USE_WRITE == 1
  DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT USER_ioctl (BYTE pdrv, BYTE cmd, void *buff);
#endif /* _USE_IOCTL == 1 */

Diskio_drvTypeDef  USER_Driver =
{
  USER_initialize,
  USER_status,
  USER_read,
#if  _USE_WRITE
  USER_write,
#endif  /* _USE_WRITE == 1 */
#if  _USE_IOCTL == 1
  USER_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_initialize (
	BYTE pdrv           /* Physical drive nmuber to identify the drive */
)
{
  /* USER CODE BEGIN INIT */
    BSP_QSPI_Init_t init_config;

    /* Only support drive 0 */
    if (pdrv != 0) {
        Stat = STA_NOINIT;
        return Stat;
    }

    /* Initialize QSPI flash with safe defaults (1-1-1 mode, STR) */
    init_config.InterfaceMode = BSP_QSPI_SPI_1_1_1_MODE;
    init_config.TransferRate = BSP_QSPI_STR_TRANSFER;
    init_config.DualFlashMode = BSP_QSPI_DUALFLASH_DISABLE;

    /* Initialize the flash */
    if (BSP_QSPI_Init(0, &init_config) != BSP_ERROR_NONE) {
        Stat = STA_NOINIT;
        return Stat;
    }

    /* Enable memory-mapped mode for read operations */
    if (BSP_QSPI_EnableMemoryMappedMode(0) != BSP_ERROR_NONE) {
        Stat = STA_NOINIT;
        return Stat;
    }

    /* Mark disk as ready */
    Stat = 0;
    return Stat;
  /* USER CODE END INIT */
}

/**
  * @brief  Gets Disk Status
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_status (
	BYTE pdrv       /* Physical drive number to identify the drive */
)
{
  /* USER CODE BEGIN STATUS */
    /* Only support drive 0 */
    if (pdrv != 0) {
        return STA_NOINIT;
    }

    /* Return current status (0 means ready, STA_NOINIT means not initialized) */
    return Stat;
  /* USER CODE END STATUS */
}

/**
  * @brief  Reads Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT USER_read (
	BYTE pdrv,      /* Physical drive nmuber to identify the drive */
	BYTE *buff,     /* Data buffer to store read data */
	DWORD sector,   /* Sector address in LBA */
	UINT count      /* Number of sectors to read */
)
{
  /* USER CODE BEGIN READ */
    uint32_t flash_addr;
    uint32_t read_size;
    uint8_t *flash_ptr;

    /* Validate parameters */
    if (pdrv != 0 || buff == NULL || count == 0) {
        return RES_PARERR;
    }

    /* Check if disk is ready */
    if (Stat != 0) {
        return RES_NOTRDY;
    }

    /* Calculate flash address:
     * Memory mapped flash starts at QSPI_FLASH_BASE_ADDR
     * Filesystem starts at offset FATFS_FLASH_OFFSET from flash base
     * Each sector is FATFS_SECTOR_SIZE bytes
     */
    flash_addr = QSPI_FLASH_BASE_ADDR + FATFS_FLASH_OFFSET + (sector * FATFS_SECTOR_SIZE);
    read_size = count * FATFS_SECTOR_SIZE;

    /* Use memory-mapped read by direct pointer dereference */
    flash_ptr = (uint8_t *)flash_addr;
    memcpy(buff, flash_ptr, read_size);

    return RES_OK;
  /* USER CODE END READ */
}

/**
  * @brief  Writes Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT USER_write (
	BYTE pdrv,          /* Physical drive nmuber to identify the drive */
	const BYTE *buff,   /* Data to be written */
	DWORD sector,       /* Sector address in LBA */
	UINT count          /* Number of sectors to write */
)
{
  /* USER CODE BEGIN WRITE */
  /* USER CODE HERE */
    return RES_OK;
  /* USER CODE END WRITE */
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation
  * @param  pdrv: Physical drive number (0..)
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT USER_ioctl (
	BYTE pdrv,      /* Physical drive nmuber (0..) */
	BYTE cmd,       /* Control code */
	void *buff      /* Buffer to send/receive control data */
)
{
  /* USER CODE BEGIN IOCTL */
    DRESULT res = RES_ERROR;
    BSP_QSPI_Info_t flash_info;

    /* Validate parameters */
    if (pdrv != 0) {
        return RES_PARERR;
    }

    switch(cmd) {
    case CTRL_SYNC:
        /* Since we're in read-only mode, nothing to sync */
        res = RES_OK;
        break;

    case GET_SECTOR_COUNT:
        /* Return number of sectors in filesystem region
         * Filesystem: 62 MB = (62 * 1024 * 1024) / 512 sectors
         */
        if (buff != NULL) {
            *(DWORD *)buff = (62 * 1024 * 1024) / FATFS_SECTOR_SIZE;
            res = RES_OK;
        }
        break;

    case GET_SECTOR_SIZE:
        /* Return sector size */
        if (buff != NULL) {
            *(WORD *)buff = FATFS_SECTOR_SIZE;
            res = RES_OK;
        }
        break;

    case GET_BLOCK_SIZE:
        /* Return block size (erase unit) - using 64KB sectors */
        if (buff != NULL) {
            *(DWORD *)buff = (64 * 1024) / FATFS_SECTOR_SIZE;  /* 128 sectors per block */
            res = RES_OK;
        }
        break;

    case CTRL_TRIM:
        /* TRIM is not needed for read-only operation */
        res = RES_OK;
        break;

    default:
        res = RES_PARERR;
        break;
    }

    return res;
  /* USER CODE END IOCTL */
}
#endif /* _USE_IOCTL == 1 */

