/**
 ******************************************************************************
 * @file    mt25ql512abb.c
 * @brief   MT25QL512ABB QSPI Flash driver implementation
 *          Minimal implementation for memory-mapped read-only access
 ******************************************************************************
 */

#include "mt25ql512abb.h"

#include <stdio.h>
#include <string.h>

/**
 * @brief  Get Flash information
 * @param  pInfo pointer to information structure
 * @retval error status
 */
int32_t MT25QL512ABB_GetFlashInfo(MT25QL512ABB_Info_t *pInfo)
{
    if (pInfo == NULL)
        return MT25QL512ABB_ERROR;

    /* Configure the structure with the memory configuration */
    pInfo->FlashSize              = MT25QL512ABB_FLASH_SIZE;
    pInfo->EraseSectorSize        = MT25QL512ABB_SECTOR_64K;
    pInfo->EraseSectorsNumber     = (MT25QL512ABB_FLASH_SIZE / MT25QL512ABB_SECTOR_64K);
    pInfo->EraseSubSectorSize     = MT25QL512ABB_SUBSECTOR_32K;
    pInfo->EraseSubSectorNumber   = (MT25QL512ABB_FLASH_SIZE / MT25QL512ABB_SUBSECTOR_32K);
    pInfo->EraseSubSector1Size    = MT25QL512ABB_SUBSECTOR_4K;
    pInfo->EraseSubSector1Number  = (MT25QL512ABB_FLASH_SIZE / MT25QL512ABB_SUBSECTOR_4K);
    pInfo->ProgPageSize           = MT25QL512ABB_PAGE_SIZE;
    pInfo->ProgPagesNumber        = (MT25QL512ABB_FLASH_SIZE / MT25QL512ABB_PAGE_SIZE);

    return MT25QL512ABB_OK;
}

/**
 * @brief  Polling WIP (Write In Progress) bit until it becomes 0
 * @param  Ctx Component object pointer
 * @param  Mode Interface mode
 * @param  DualFlash Dual flash mode state
 * @retval error status
 */
int32_t MT25QL512ABB_AutoPollingMemReady(QSPI_HandleTypeDef *Ctx, MT25QL512ABB_Interface_t Mode, MT25QL512ABB_DualFlash_t DualFlash)
{
    QSPI_CommandTypeDef      s_command;
    QSPI_AutoPollingTypeDef  s_config;

    if (Ctx == NULL)
        return MT25QL512ABB_ERROR;

    /* Configure automatic polling mode to wait for memory ready */
    s_command.InstructionMode   = (Mode == MT25QL512ABB_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES :
                                  (Mode == MT25QL512ABB_DPI_MODE) ? QSPI_INSTRUCTION_2_LINES :
                                  QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = MT25QL512ABB_READ_STATUS_REG_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DummyCycles       = 0U;
    s_command.DataMode          = (Mode == MT25QL512ABB_QPI_MODE) ? QSPI_DATA_4_LINES :
                                  (Mode == MT25QL512ABB_DPI_MODE) ? QSPI_DATA_2_LINES :
                                  QSPI_DATA_1_LINE;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    s_config.Match           = 0;
    s_config.Mask            = (DualFlash == MT25QL512ABB_DUALFLASH_ENABLE) ?
                               ((MT25QL512ABB_SR_WIP << 8) | MT25QL512ABB_SR_WIP) :
                               MT25QL512ABB_SR_WIP;
    s_config.MatchMode       = QSPI_MATCH_MODE_AND;
    s_config.StatusBytesSize = (DualFlash == MT25QL512ABB_DUALFLASH_ENABLE) ? 2U : 1U;
    s_config.Interval        = MT25QL512ABB_AUTOPOLLING_INTERVAL_TIME;
    s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

    HAL_StatusTypeDef ret = HAL_QSPI_AutoPolling(Ctx, &s_command, &s_config, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
    if (ret != HAL_OK)
    {
        printf("Failed with value %d.\n", ret);
        return MT25QL512ABB_ERROR;
    }

    return MT25QL512ABB_OK;
}

/**
 * @brief  Flash enter 4 Byte address mode
 * @param  Ctx Component object pointer
 * @param  Mode Interface mode
 * @retval error status
 */
int32_t MT25QL512ABB_Enter4BytesAddressMode(QSPI_HandleTypeDef *Ctx, MT25QL512ABB_Interface_t Mode)
{
    QSPI_CommandTypeDef s_command;

    if (Ctx == NULL)
        return MT25QL512ABB_ERROR;

    /* Configure the command to enter 4-byte address mode */
    s_command.InstructionMode   = (Mode == MT25QL512ABB_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES :
                                  (Mode == MT25QL512ABB_DPI_MODE) ? QSPI_INSTRUCTION_2_LINES :
                                  QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = MT25QL512ABB_ENTER_4_BYTE_ADDR_MODE_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DummyCycles       = 0;
    s_command.DataMode          = QSPI_DATA_NONE;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Send the command */
    if (HAL_QSPI_Command(Ctx, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return MT25QL512ABB_ERROR;
    }

    return MT25QL512ABB_OK;
}

/**
 * @brief  Enable memory-mapped mode (STR - Single Transfer Rate)
 * @param  Ctx Component object pointer
 * @param  Mode Interface mode
 * @param  AddressSize Address size mode (3 or 4 bytes)
 * @retval error status
 */
int32_t MT25QL512ABB_EnableMemoryMappedModeSTR(QSPI_HandleTypeDef *Ctx, MT25QL512ABB_Interface_t Mode, MT25QL512ABB_AddressSize_t AddressSize)
{
    QSPI_CommandTypeDef      s_command;
    QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;

    if (Ctx == NULL)
        return MT25QL512ABB_ERROR;

    /* Initialize memory mapped command structure */
    memset(&s_command, 0, sizeof(QSPI_CommandTypeDef));
    memset(&s_mem_mapped_cfg, 0, sizeof(QSPI_MemoryMappedTypeDef));

    /* Configure the read command for memory mapped mode */
    switch(Mode)
    {
    case MT25QL512ABB_SPI_1I4O_MODE:  /* 1-1-4 commands - common fast mode */
        s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        s_command.Instruction     = (AddressSize == MT25QL512ABB_3BYTES_SIZE) ?
                                    MT25QL512ABB_1I4O_FAST_READ_CMD :
                                    MT25QL512ABB_4_BYTE_ADDR_1I4O_FAST_READ_CMD;
        s_command.AddressMode     = QSPI_ADDRESS_1_LINE;
        s_command.DummyCycles     = 8;
        s_command.DataMode        = QSPI_DATA_4_LINES;
        break;

    case MT25QL512ABB_SPI_MODE:       /* 1-1-1 commands - default safe mode */
    default:
        s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        s_command.Instruction     = (AddressSize == MT25QL512ABB_3BYTES_SIZE) ?
                                    MT25QL512ABB_FAST_READ_CMD :
                                    MT25QL512ABB_4_BYTE_ADDR_FAST_READ_CMD;
        s_command.AddressMode     = QSPI_ADDRESS_1_LINE;
        s_command.DummyCycles     = 8;
        s_command.DataMode        = QSPI_DATA_1_LINE;
        break;
    }

    /* Common configuration for all modes */
    s_command.AddressSize       = (AddressSize == MT25QL512ABB_3BYTES_SIZE) ?
                                  QSPI_ADDRESS_24_BITS : QSPI_ADDRESS_32_BITS;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Configure memory mapped mode parameters */
    s_mem_mapped_cfg.TimeOutPeriod = 0;

    /* Enable memory-mapped mode */
    if (HAL_QSPI_MemoryMapped(Ctx, &s_command, &s_mem_mapped_cfg) != HAL_OK)
    {
        return MT25QL512ABB_ERROR;
    }

    return MT25QL512ABB_OK;
}
