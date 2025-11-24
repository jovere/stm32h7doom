/**
 ******************************************************************************
 * @file    quadspi.h
 * @brief   QSPI abstraction layer header for MT25QL512ABB
 *          Provides BSP-style interface for QSPI flash initialization
 ******************************************************************************
 */

#ifndef QUADSPI_H
#define QUADSPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"
#include "mt25ql512abb.h"

/* Memory mapped base address */
#define QSPI_BASE_ADDRESS          0x90000000

/* Number of QSPI instances */
#define QSPI_INSTANCES_NUMBER      1

/* BSP QSPI status/error codes */
#define BSP_ERROR_NONE             0
#define BSP_ERROR_WRONG_PARAM      -1
#define BSP_ERROR_PERIPH_FAILURE   -2
#define BSP_ERROR_COMPONENT_FAILURE -3

/* Interface mode options */
typedef enum {
    BSP_QSPI_SPI_1_1_1_MODE = 0,  /* 1-1-1 mode (safest, slowest) */
    BSP_QSPI_SPI_1_1_4_MODE = 1,  /* 1-1-4 mode (common fast mode) */
    BSP_QSPI_QPI_4_4_4_MODE = 2,  /* 4-4-4 mode (fastest) */
} BSP_QSPI_Interface_t;

/* Transfer rate mode */
typedef enum {
    BSP_QSPI_STR_TRANSFER = 0,  /* Single Transfer Rate (133 MHz max) */
    BSP_QSPI_DTR_TRANSFER = 1,  /* Double Transfer Rate (90 MHz max) */
} BSP_QSPI_Transfer_t;

/* Dual flash mode */
typedef enum {
    BSP_QSPI_DUALFLASH_DISABLE = 0,
    BSP_QSPI_DUALFLASH_ENABLE = 1,
} BSP_QSPI_DualFlash_t;

/* QSPI initialization structure */
typedef struct {
    BSP_QSPI_Interface_t InterfaceMode;
    BSP_QSPI_Transfer_t TransferRate;
    BSP_QSPI_DualFlash_t DualFlashMode;
} BSP_QSPI_Init_t;

/* QSPI context structure */
typedef struct {
    uint32_t IsInitialized;
    MT25QL512ABB_Interface_t InterfaceMode;
    MT25QL512ABB_DualFlash_t DualFlashMode;
    uint32_t IsMspCallbacksValid;
} QSPI_Ctx_t;

/* QSPI info structure */
typedef struct {
    uint32_t FlashSize;
    uint32_t SectorSize;
    uint32_t PageSize;
} BSP_QSPI_Info_t;

/* QSPI access states */
#define QSPI_ACCESS_NONE           0x00U
#define QSPI_ACCESS_INDIRECT       0x01U
#define QSPI_ACCESS_MMP            0x02U

/* Function prototypes */
extern QSPI_HandleTypeDef hqspi;
extern QSPI_Ctx_t QSPICtx[QSPI_INSTANCES_NUMBER];

int32_t BSP_QSPI_Init(uint32_t Instance, BSP_QSPI_Init_t *Init);
int32_t BSP_QSPI_DeInit(uint32_t Instance);
int32_t BSP_QSPI_EnableMemoryMappedMode(uint32_t Instance);
int32_t BSP_QSPI_DisableMemoryMappedMode(uint32_t Instance);
int32_t BSP_QSPI_GetInfo(uint32_t Instance, BSP_QSPI_Info_t *pInfo);

#ifdef __cplusplus
}
#endif

#endif /* QUADSPI_H */
