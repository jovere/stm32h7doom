/**
  ******************************************************************************
  * @file    i_sound_stm32.c
  * @brief   STM32 Sound Backend for Doom
  *          Implements sound_module_t interface for STM32H750
  ******************************************************************************
  */

#include <string.h>
#include <stdio.h>

#include "i_sound.h"
#include "i_system.h"
#include "doomtype.h"
#include "w_wad.h"
#include "z_zone.h"
#include "i_opl_stm32.h"
#include "opl/opl_timer.h"

/* STM32 audio driver */
extern void Audio_Init(void);
extern void Audio_Start(void);
extern void Audio_MixCallback(int16_t* buffer, int samples);

/* Sound channel structure */
typedef struct
{
    uint8_t *sample_data;      /* PCM sample data (8-bit unsigned) */
    int length;                /* Sample length in bytes */
    int position;              /* Current playback position */
    int step;                  /* Sample rate conversion step (fixed point 16.16) */
    int step_remainder;        /* Fractional part for resampling */
    int volume;                /* Volume (0-127) */
    int separation;            /* Stereo separation (0-255: 0=left, 128=center, 255=right) */
    boolean playing;           /* Is this channel active? */
    sfxinfo_t *sfxinfo;        /* Sound effect info */
} sound_channel_t;

/* Private variables */
#define NUM_CHANNELS 8
static sound_channel_t channels[NUM_CHANNELS];

/* Forward declarations */
static boolean I_STM32_InitSound(boolean use_sfx_prefix);
static void I_STM32_ShutdownSound(void);
static int I_STM32_GetSfxLumpNum(sfxinfo_t *sfxinfo);
static void I_STM32_UpdateSound(void);
static void I_STM32_UpdateSoundParams(int channel, int vol, int sep);
static int I_STM32_StartSound(sfxinfo_t *sfxinfo, int channel, int vol, int sep);
static void I_STM32_StopSound(int channel);
static boolean I_STM32_SoundIsPlaying(int channel);
static void I_STM32_PrecacheSounds(sfxinfo_t *sounds, int num_sounds);

/**
 * @brief Initialize STM32 sound system
 */
static boolean I_STM32_InitSound(boolean use_sfx_prefix)
{
    int i;

    /* Initialize all channels */
    for (i = 0; i < NUM_CHANNELS; i++)
    {
        memset(&channels[i], 0, sizeof(sound_channel_t));
    }

    /* Initialize STM32 audio hardware */
    Audio_Init();
    Audio_Start();

    printf("[Audio] STM32 sound system initialized\n");
    printf("[Audio] %d sound channels available\n", NUM_CHANNELS);

    return true;
}

/**
 * @brief Shutdown sound system
 */
static void I_STM32_ShutdownSound(void)
{
    /* Stop all channels */
    int i;
    for (i = 0; i < NUM_CHANNELS; i++)
    {
        channels[i].playing = false;
    }
}

/**
 * @brief Get WAD lump number for sound effect
 */
static int I_STM32_GetSfxLumpNum(sfxinfo_t *sfxinfo)
{
    char namebuf[11];

    /* Construct sound lump name */
    sprintf(namebuf, "ds%s", sfxinfo->name);

    return W_GetNumForName(namebuf);
}

/**
 * @brief Update sound system (called each frame)
 */
static void I_STM32_UpdateSound(void)
{
    /* Nothing needed - DMA handles continuous playback */
}

/**
 * @brief Update sound parameters for a channel
 */
static void I_STM32_UpdateSoundParams(int channel, int vol, int sep)
{
    if (channel < 0 || channel >= NUM_CHANNELS)
        return;

    channels[channel].volume = vol;
    channels[channel].separation = sep;
}

/**
 * @brief Start playing a sound effect
 */
static int I_STM32_StartSound(sfxinfo_t *sfxinfo, int channel, int vol, int sep)
{
    int lumpnum;
    int lumplen;
    byte *data;
    int samplerate;

    if (channel < 0 || channel >= NUM_CHANNELS)
        return -1;

    /* Stop any sound currently playing on this channel */
    channels[channel].playing = false;

    /* Load sound effect from WAD */
    lumpnum = sfxinfo->lumpnum;
    lumplen = W_LumpLength(lumpnum);

    /* Sound lump format: header (8 bytes) + sample data */
    /* Header: uint16 format(3), uint16 samplerate, uint32 length */
    data = W_CacheLumpNum(lumpnum, PU_STATIC);

    if (lumplen < 8)
    {
        return -1;  /* Invalid sound lump */
    }

    /* Extract sample rate from header */
    samplerate = (data[3] << 8) | data[2];

    /* Calculate resampling step (fixed point 16.16) */
    /* step = (source_rate / target_rate) * 65536 */
    channels[channel].step = (samplerate << 16) / 44100;
    channels[channel].step_remainder = 0;

    /* Set up channel */
    channels[channel].sample_data = data + 8;  /* Skip header */
    channels[channel].length = lumplen - 8;
    channels[channel].position = 0;
    channels[channel].volume = vol;
    channels[channel].separation = sep;
    channels[channel].playing = true;
    channels[channel].sfxinfo = sfxinfo;

    return channel;
}

/**
 * @brief Stop a sound channel
 */
static void I_STM32_StopSound(int channel)
{
    if (channel < 0 || channel >= NUM_CHANNELS)
        return;

    channels[channel].playing = false;
}

/**
 * @brief Check if a sound is playing
 */
static boolean I_STM32_SoundIsPlaying(int channel)
{
    if (channel < 0 || channel >= NUM_CHANNELS)
        return false;

    return channels[channel].playing;
}

/**
 * @brief Precache sounds (optional - loads sounds into cache)
 */
static void I_STM32_PrecacheSounds(sfxinfo_t *sounds, int num_sounds)
{
    /* We load sounds on-demand, so precaching not strictly needed */
    /* But we can cache popular sounds here if memory allows */
}

/**
 * @brief Audio mixing callback - mixes all active sound channels
 *
 * Called by DMA interrupt when audio buffer needs refilling.
 */
void Audio_MixCallback(int16_t* buffer, int samples)
{
    int i, j;
    int ch;
    uint64_t elapsed_us;

    /* Clear buffer */
    memset(buffer, 0, samples * 2 * sizeof(int16_t));

    /* Calculate elapsed time for OPL timer (microseconds) */
    /* samples at 44.1kHz: elapsed_us = (samples * 1000000) / 44100 */
    elapsed_us = (samples * 1000000ULL) / 44100;

    /* Advance OPL timer and process any pending MIDI events */
    OPL_Timer_AdvanceTime(elapsed_us);

    /* Generate and mix OPL music */
    OPL_STM32_GenerateSamples(buffer, samples);

    /* Mix all active sound effect channels */
    for (ch = 0; ch < NUM_CHANNELS; ch++)
    {
        if (!channels[ch].playing)
            continue;

        for (i = 0; i < samples; i++)
        {
            int sample_pos = channels[ch].position >> 16;  /* Get integer part */

            /* Check if we've reached the end */
            if (sample_pos >= channels[ch].length)
            {
                channels[ch].playing = false;
                break;
            }

            /* Get 8-bit unsigned sample and convert to 16-bit signed */
            uint8_t sample_u8 = channels[ch].sample_data[sample_pos];
            int16_t sample = ((int16_t)sample_u8 - 128) << 8;

            /* Apply volume (0-127) */
            sample = (sample * channels[ch].volume) >> 7;

            /* Apply stereo separation (0-255) */
            int left_vol = 255 - channels[ch].separation;
            int right_vol = channels[ch].separation;

            /* Mix into output buffer (with clipping) */
            int32_t left = buffer[i * 2 + 0] + ((sample * left_vol) >> 8);
            int32_t right = buffer[i * 2 + 1] + ((sample * right_vol) >> 8);

            /* Clamp to prevent overflow */
            if (left > 32767) left = 32767;
            if (left < -32768) left = -32768;
            if (right > 32767) right = 32767;
            if (right < -32768) right = -32768;

            buffer[i * 2 + 0] = (int16_t)left;
            buffer[i * 2 + 1] = (int16_t)right;

            /* Advance sample position (with resampling) */
            channels[ch].position += channels[ch].step;
        }
    }
}

/**
 * @brief Sound devices supported by STM32 module
 */
static snddevice_t sound_devices[] =
{
    SNDDEVICE_SB,  /* Generic Sound Blaster compatible (digital audio) */
};

/**
 * @brief STM32 sound module interface
 */
sound_module_t sound_stm32_module =
{
    sound_devices,                  /* sound_devices */
    arrlen(sound_devices),          /* num_sound_devices */
    I_STM32_InitSound,
    I_STM32_ShutdownSound,
    I_STM32_GetSfxLumpNum,
    I_STM32_UpdateSound,
    I_STM32_UpdateSoundParams,
    I_STM32_StartSound,
    I_STM32_StopSound,
    I_STM32_SoundIsPlaying,
    I_STM32_PrecacheSounds,
};
