/**
  ******************************************************************************
  * @file    i_opl_stm32.c
  * @brief   STM32 OPL Backend - Direct DBOPL Integration
  *          No SDL, no threading - just pure OPL synthesis
  ******************************************************************************
  */

#include <string.h>
#include <stdio.h>
#include "opl/opl.h"
#include "opl/opl_internal.h"
#include "opl/opl_timer.h"
#include "opl/dbopl.h"

/* DBOPL chip instance */
static Chip opl_chip;
static int opl_initialized = 0;
static int opl3_mode = 0;

/* Register select latches */
static unsigned int register_num = 0;
static unsigned int register_num_opl3 = 0;

/* Internal buffer for OPL sample generation (mono 32-bit) */
#define OPL_BUFFER_SIZE 2048
static Bit32s opl_buffer[OPL_BUFFER_SIZE];

/**
 * @brief Initialize OPL emulation
 */
static int OPL_STM32_Init(unsigned int port_base)
{
    (void)port_base;  /* Unused - we're using software emulation */

    /* Initialize DBOPL */
    DBOPL_InitTables();
    Chip__Chip(&opl_chip);
    Chip__Setup(&opl_chip, opl_sample_rate);

    /* Start the OPL timer system for MIDI callback processing */
    OPL_Timer_StartThread();

    opl_initialized = 1;
    opl3_mode = 0;  /* Start in OPL2 mode */

    printf("[OPL] STM32 DBOPL emulator initialized at %u Hz\n", opl_sample_rate);

    return 1;  /* Success */
}

/**
 * @brief Shutdown OPL emulation
 */
static void OPL_STM32_Shutdown(void)
{
    OPL_Timer_StopThread();
    opl_initialized = 0;
}

/**
 * @brief Write to OPL port
 */
/* Timer state for status register emulation */
static int timer_started = 0;
static int read_count_since_start = 0;

static void OPL_STM32_WritePort(opl_port_t port, unsigned int value)
{
    unsigned int reg;

    if (!opl_initialized)
        return;

    switch (port)
    {
        case OPL_REGISTER_PORT:
            /* OPL2 register select */
            register_num = value;
            break;

        case OPL_REGISTER_PORT_OPL3:
            /* OPL3 register select */
            register_num_opl3 = value;
            break;

        case OPL_DATA_PORT:
            /* Data write to currently selected register */
            if (register_num_opl3 != 0)
            {
                /* OPL3 register write */
                reg = register_num_opl3 | 0x100;
                register_num_opl3 = 0;
            }
            else
            {
                /* OPL2 register write */
                reg = register_num;
            }

            /* Check for OPL3 mode enable */
            if (reg == 0x105)
            {
                opl3_mode = (value & 0x01) ? 1 : 0;
            }

            /* Track timer control register writes for status emulation */
            if (reg == 0x04)  /* Timer control register */
            {
                if (value == 0x21)  /* Timer 1 start */
                {
                    timer_started = 1;
                    read_count_since_start = 0;
                }
                else if (value == 0x60)  /* Timer reset */
                {
                    timer_started = 0;
                    read_count_since_start = 0;
                }
            }

            /* Write to DBOPL chip */
            Chip__WriteReg(&opl_chip, reg, value);
            break;
    }
}

/**
 * @brief Read from OPL port (status register)
 */
static unsigned int OPL_STM32_ReadPort(opl_port_t port)
{
    /* Software emulation: fake status register for detection
     * Track timer control register writes (0x21 = start, 0x60 = reset)
     * Return timer status bits after timer has been running for a while
     */

    if (timer_started)
    {
        read_count_since_start++;

        /* After ~200 reads following timer start, return "timer expired" status */
        if (read_count_since_start > 200)
        {
            return 0xc0;  /* Bits 7,6 set = timer 1 and 2 expired */
        }
    }

    return 0x00;  /* Timers not started or not expired yet */
}

/**
 * @brief Generate OPL samples
 *
 * Called by audio mixer to generate music samples.
 * Mixes OPL output with existing audio buffer.
 *
 * @param buffer Stereo 16-bit output buffer (L, R, L, R...)
 * @param samples Number of stereo sample pairs
 */
void OPL_STM32_GenerateSamples(int16_t *buffer, int samples)
{
    int i;

    if (!opl_initialized || samples <= 0)
        return;

    /* Limit to buffer size */
    if (samples > OPL_BUFFER_SIZE)
        samples = OPL_BUFFER_SIZE;

    /* Generate OPL samples using DBOPL */
    if (opl3_mode)
    {
        Chip__GenerateBlock3(&opl_chip, samples, opl_buffer);
    }
    else
    {
        Chip__GenerateBlock2(&opl_chip, samples, opl_buffer);
    }

    /* Mix OPL output into stereo buffer */
    /* DBOPL outputs mono 32-bit samples, we convert to stereo 16-bit */
    for (i = 0; i < samples; i++)
    {
        /* Convert 32-bit mono to 16-bit stereo */
        /* DBOPL output range is approximately -8192 to +8192 */
        /* Scale by 2 to get better volume */
        int32_t sample = (opl_buffer[i] * 2);

        /* Clamp to 16-bit range */
        if (sample > 32767) sample = 32767;
        if (sample < -32768) sample = -32768;

        /* Mix into both channels (mono -> stereo) */
        buffer[i * 2 + 0] += (int16_t)sample;  /* Left */
        buffer[i * 2 + 1] += (int16_t)sample;  /* Right */
    }
}

/**
 * @brief OPL driver structure for STM32
 */
opl_driver_t opl_stm32_driver =
{
    "STM32 DBOPL",
    OPL_STM32_Init,
    OPL_STM32_Shutdown,
    OPL_STM32_ReadPort,
    OPL_STM32_WritePort,
    OPL_Timer_SetCallback,      /* Use STM32 timer callbacks */
    OPL_Timer_ClearCallbacks,
    OPL_Timer_Lock,
    OPL_Timer_Unlock,
    OPL_Timer_SetPaused,
    OPL_Timer_AdjustCallbacks
};
