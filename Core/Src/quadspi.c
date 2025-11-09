/**
 ******************************************************************************
 * @file    quadspi.c
 * @brief   QSPI abstraction layer implementation for MT25QL512ABB
 *          Provides BSP-style interface for QSPI flash initialization
 ******************************************************************************
 */

#include "quadspi.h"
#include <string.h>

/* QSPI context array */
QSPI_Ctx_t QSPICtx[QSPI_INSTANCES_NUMBER];

/* Convert interface mode enum between BSP and MT25QL types */
static MT25QL512ABB_Interface_t BSP_to_MT25QL_Mode(BSP_QSPI_Interface_t bsp_mode)
{
    switch(bsp_mode) {
    case BSP_QSPI_SPI_1_1_1_MODE:
        return MT25QL512ABB_SPI_MODE;
    case BSP_QSPI_SPI_1_1_4_MODE:
        return MT25QL512ABB_SPI_1I4O_MODE;
    case BSP_QSPI_QPI_4_4_4_MODE:
        return MT25QL512ABB_QPI_MODE;
    default:
        return MT25QL512ABB_SPI_MODE;
    }
}

/**
 * @brief  Initialize QSPI interface and flash device
 * @param  Instance QSPI instance number
 * @param  Init Initialization structure
 * @retval BSP status
 */
int32_t BSP_QSPI_Init(uint32_t Instance, BSP_QSPI_Init_t *Init)
{
    int32_t ret = BSP_ERROR_NONE;
    MT25QL512ABB_Info_t flash_info;

    /* Validate inputs */
    if (Instance >= QSPI_INSTANCES_NUMBER || Init == NULL) {
        return BSP_ERROR_WRONG_PARAM;
    }

    /* Check if already initialized */
    if (QSPICtx[Instance].IsInitialized != QSPI_ACCESS_NONE) {
        return BSP_ERROR_NONE;
    }

    /* Initialize context */
    memset(&QSPICtx[Instance], 0, sizeof(QSPI_Ctx_t));
    QSPICtx[Instance].DualFlashMode = (MT25QL512ABB_DualFlash_t)Init->DualFlashMode;
    QSPICtx[Instance].InterfaceMode = BSP_to_MT25QL_Mode(Init->InterfaceMode);

    /* Get flash information */
    if (MT25QL512ABB_GetFlashInfo(&flash_info) != MT25QL512ABB_OK) {
        return BSP_ERROR_COMPONENT_FAILURE;
    }

    /* Poll for memory ready */
    if (MT25QL512ABB_AutoPollingMemReady(&hqspi, QSPICtx[Instance].InterfaceMode,
                                         QSPICtx[Instance].DualFlashMode) != MT25QL512ABB_OK) {
        return BSP_ERROR_COMPONENT_FAILURE;
    }

    /* Enter 4-byte addressing mode (required for 512Mb flash) */
    if (MT25QL512ABB_Enter4BytesAddressMode(&hqspi, QSPICtx[Instance].InterfaceMode) != MT25QL512ABB_OK) {
        return BSP_ERROR_COMPONENT_FAILURE;
    }

    /* Mark as initialized for indirect access */
    QSPICtx[Instance].IsInitialized = QSPI_ACCESS_INDIRECT;

    return ret;
}

/**
 * @brief  De-initialize QSPI interface
 * @param  Instance QSPI instance number
 * @retval BSP status
 */
int32_t BSP_QSPI_DeInit(uint32_t Instance)
{
    if (Instance >= QSPI_INSTANCES_NUMBER) {
        return BSP_ERROR_WRONG_PARAM;
    }

    /* Disable memory mapped mode if active */
    if (QSPICtx[Instance].IsInitialized == QSPI_ACCESS_MMP) {
        HAL_QSPI_Abort(&hqspi);
    }

    QSPICtx[Instance].IsInitialized = QSPI_ACCESS_NONE;
    return BSP_ERROR_NONE;
}

/**
 * @brief  Enable memory-mapped mode for QSPI flash
 * @param  Instance QSPI instance number
 * @retval BSP status
 */
int32_t BSP_QSPI_EnableMemoryMappedMode(uint32_t Instance)
{
    int32_t ret = BSP_ERROR_NONE;

    if (Instance >= QSPI_INSTANCES_NUMBER) {
        return BSP_ERROR_WRONG_PARAM;
    }

    if (QSPICtx[Instance].IsInitialized == QSPI_ACCESS_NONE) {
        return BSP_ERROR_PERIPH_FAILURE;
    }

    /* Enable memory-mapped mode in STR (Single Transfer Rate) */
    if (MT25QL512ABB_EnableMemoryMappedModeSTR(&hqspi, QSPICtx[Instance].InterfaceMode,
                                                MT25QL512ABB_4BYTES_SIZE) != MT25QL512ABB_OK) {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    } else {
        QSPICtx[Instance].IsInitialized = QSPI_ACCESS_MMP;
    }

    return ret;
}

/**
 * @brief  Disable memory-mapped mode for QSPI flash
 * @param  Instance QSPI instance number
 * @retval BSP status
 */
int32_t BSP_QSPI_DisableMemoryMappedMode(uint32_t Instance)
{
    if (Instance >= QSPI_INSTANCES_NUMBER) {
        return BSP_ERROR_WRONG_PARAM;
    }

    if (QSPICtx[Instance].IsInitialized != QSPI_ACCESS_MMP) {
        return BSP_ERROR_PERIPH_FAILURE;
    }

    HAL_QSPI_Abort(&hqspi);
    QSPICtx[Instance].IsInitialized = QSPI_ACCESS_INDIRECT;

    return BSP_ERROR_NONE;
}

/**
 * @brief  Get QSPI flash information
 * @param  Instance QSPI instance number
 * @param  pInfo Pointer to info structure
 * @retval BSP status
 */
int32_t BSP_QSPI_GetInfo(uint32_t Instance, BSP_QSPI_Info_t *pInfo)
{
    MT25QL512ABB_Info_t flash_info;

    if (Instance >= QSPI_INSTANCES_NUMBER || pInfo == NULL) {
        return BSP_ERROR_WRONG_PARAM;
    }

    if (MT25QL512ABB_GetFlashInfo(&flash_info) != MT25QL512ABB_OK) {
        return BSP_ERROR_COMPONENT_FAILURE;
    }

    pInfo->FlashSize = flash_info.FlashSize;
    pInfo->SectorSize = flash_info.EraseSectorSize;
    pInfo->PageSize = flash_info.ProgPageSize;

    return BSP_ERROR_NONE;
}
