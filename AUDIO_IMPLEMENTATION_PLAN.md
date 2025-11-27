# STM32H750 Doom Audio System - Implementation Plan

## Overview

Implement complete audio system for Doom port using:
- **OPL2 emulation** (DBOPL from DOSBox) for music synthesis
- **PCM mixing** for sound effects (8 simultaneous channels)
- **SAI2 peripheral** in I2S mode for digital audio output
- **Self-configuring I2S DAC** (PCM5102A or compatible)

**Audio specifications:**
- Sample rate: 44.1 kHz
- Format: 16-bit stereo PCM
- Music: OPL2 FM synthesis (49.7 kHz internally, resampled to 44.1 kHz)
- SFX: 8-channel mixer with 3D positional audio

---

## Implementation Tasks

### Phase 1: Ethernet SPI Cleanup

#### Task 1.1: Disable SPI1 After Ethernet Init

**File:** `/work/LWIP/Target/eth_custom_phy_interface.c`

**Changes:**
```c
// After line 67 (KSZ8863_Init())
int32_t eth_phy_init(void)
{
    ret = KSZ8863_Init();
    if (ret != 0) return ETH_PHY_STATUS_ERROR;

    // NEW: Disable SPI1 - no longer needed after switch config
    LL_SPI_Disable(SPI1);
    LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_SPI1);

    // Pins PD7, PG9, PG11 now available for reassignment

    return ret;
}
```

**File:** `/work/LWIP/App/lwip.c`

**Changes:**
```c
// Comment out link polling (line 134-139)
static void Ethernet_Link_Periodic_Handle(struct netif *netif)
{
    #if 0  // DISABLED - Link polling not needed, SPI1 unavailable
    if (HAL_GetTick() - EthernetLinkTimer >= 100)
    {
        EthernetLinkTimer = HAL_GetTick();
        ethernet_link_check_state(netif);
    }
    #endif
}
```

**Pins freed:** PD7, PG9, PG11 (we'll use PG9 for SAI2_FS)

---

### Phase 2: Enable Audio Features

#### Task 2.1: Enable FEATURE_SOUND

**File:** `/work/chocdoom/doomfeatures.h`

**Change line 36:**
```c
// OLD:
#undef FEATURE_SOUND

// NEW:
#define FEATURE_SOUND
```

#### Task 2.2: Enable HAL SAI Module

**File:** `/work/App/stm32h7xx_hal_conf.h`

**Uncomment line 71:**
```c
// OLD:
/* #define HAL_SAI_MODULE_ENABLED */

// NEW:
#define HAL_SAI_MODULE_ENABLED
```

---

### Phase 3: Port OPL Emulation Code

#### Task 3.1: Copy OPL Directory

**Source:** `/work/chocolate-doom/opl/`
**Destination:** `/work/chocdoom/opl/`

**Files to copy:**
```
opl/dbopl.c          (48KB - OPL2/OPL3 emulator core)
opl/dbopl.h          (Header for DBOPL)
opl/opl.c            (11KB - OPL interface abstraction)
opl/opl.h            (OPL interface header)
opl/opl_queue.c      (6KB - OPL event queue)
opl/opl_queue.h
opl/opl_timer.c      (6KB - OPL timer handling)
opl/opl_timer.h
```

**Skip platform-specific files:**
- `opl_sdl.c`, `opl_linux.c`, `opl_win32.c` (not needed for embedded)

#### Task 3.2: Copy Music Player

**Source:** `/work/chocolate-doom/src/i_oplmusic.c`
**Destination:** `/work/chocdoom/i_oplmusic.c`

**Also copy:**
- `midifile.c/h` - MIDI file parser
- `mus2mid.c/h` - MUS to MIDI converter

#### Task 3.3: Add to Build System

**File:** `/work/chocdoom/CMakeLists.txt`

**Add to DOOM_SOURCES (around line 85):**
```cmake
set(DOOM_SOURCES
    # ... existing files ...

    # OPL Music System
    opl/dbopl.c
    opl/opl.c
    opl/opl_queue.c
    opl/opl_timer.c
    i_oplmusic.c
    midifile.c
    mus2mid.c
)
```

---

### Phase 4: STM32 Audio Backend

#### Task 4.1: Create Audio Driver Files

**New file:** `/work/App/audio_stm32.c`
**New file:** `/work/App/audio_stm32.h`

**File structure:**
```c
// audio_stm32.h
#ifndef AUDIO_STM32_H
#define AUDIO_STM32_H

#include <stdint.h>

// Initialize audio system (SAI + DMA)
void Audio_Init(void);

// Start audio playback
void Audio_Start(void);

// Get buffer for mixing (double-buffered)
int16_t* Audio_GetMixBuffer(int *samples);

// Audio mixing callback (called by DMA interrupt)
void Audio_MixCallback(int16_t* buffer, int samples);

#endif
```

#### Task 4.2: GPIO Configuration

**In `audio_stm32.c`:**

```c
static void Audio_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Enable GPIO clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    // PA0: SAI2_SD_B (Data)
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;  // AF10 = SAI2
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PA2: SAI2_SCK_B (Bit Clock)
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PG9: SAI2_FS_A (Frame Sync / Word Select)
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}
```

#### Task 4.3: SAI2 Peripheral Configuration

```c
static SAI_HandleTypeDef hsai2;

static void Audio_SAI_Init(void)
{
    // Enable SAI2 clock
    __HAL_RCC_SAI2_CLK_ENABLE();

    // Configure SAI2 Block B for I2S transmission
    hsai2.Instance = SAI2_Block_B;
    hsai2.Init.AudioMode = SAI_MODEMASTER_TX;
    hsai2.Init.Synchro = SAI_ASYNCHRONOUS;
    hsai2.Init.OutputDrive = SAI_OUTPUTDRIVE_ENABLE;
    hsai2.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
    hsai2.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;
    hsai2.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_44K;
    hsai2.Init.Protocol = SAI_FREE_PROTOCOL;  // I2S mode
    hsai2.Init.DataSize = SAI_DATASIZE_16;
    hsai2.Init.FirstBit = SAI_FIRSTBIT_MSB;
    hsai2.Init.ClockStrobing = SAI_CLOCKSTROBING_FALLINGEDGE;

    // I2S frame configuration
    hsai2.FrameInit.FrameLength = 64;  // 32 bits × 2 channels
    hsai2.FrameInit.ActiveFrameLength = 32;
    hsai2.FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
    hsai2.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
    hsai2.FrameInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;

    // Slot configuration
    hsai2.SlotInit.FirstBitOffset = 0;
    hsai2.SlotInit.SlotSize = SAI_SLOTSIZE_16B;
    hsai2.SlotInit.SlotNumber = 2;  // Stereo
    hsai2.SlotInit.SlotActive = SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1;

    HAL_SAI_Init(&hsai2);
}
```

#### Task 4.4: DMA Configuration

```c
#define AUDIO_BUFFER_SIZE 2048  // Samples per half-buffer

static int16_t audio_buffer[AUDIO_BUFFER_SIZE * 2 * 2];  // Double buffer, stereo
static DMA_HandleTypeDef hdma_sai2_b;

static void Audio_DMA_Init(void)
{
    // Enable DMA clock
    __HAL_RCC_DMA1_CLK_ENABLE();

    // Configure DMA for SAI2_B
    hdma_sai2_b.Instance = DMA1_Stream4;  // Or appropriate stream
    hdma_sai2_b.Init.Request = DMA_REQUEST_SAI2_B;
    hdma_sai2_b.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_sai2_b.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sai2_b.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sai2_b.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_sai2_b.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_sai2_b.Init.Mode = DMA_CIRCULAR;
    hdma_sai2_b.Init.Priority = DMA_PRIORITY_HIGH;

    HAL_DMA_Init(&hdma_sai2_b);

    // Link DMA to SAI
    __HAL_LINKDMA(&hsai2, hdmatx, hdma_sai2_b);

    // Enable DMA interrupt
    HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
}

// DMA callbacks
void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    // First half of buffer transmitted
    // Fill first half with new audio
    Audio_MixCallback(&audio_buffer[0], AUDIO_BUFFER_SIZE);
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    // Second half transmitted
    // Fill second half with new audio
    Audio_MixCallback(&audio_buffer[AUDIO_BUFFER_SIZE * 2], AUDIO_BUFFER_SIZE);
}
```

---

### Phase 5: Doom Sound Interface

#### Task 5.1: Create STM32 Sound Module

**New file:** `/work/chocdoom/i_sound_stm32.c`

```c
#include "i_sound.h"
#include "audio_stm32.h"

// Sound channel structure
typedef struct {
    uint8_t *sample_data;
    int length;
    int position;
    int volume;      // 0-127
    int separation;  // 0-255 (0=left, 128=center, 255=right)
    boolean playing;
} sound_channel_t;

static sound_channel_t channels[8];  // 8 simultaneous sounds

static int I_STM32_StartSound(sfxinfo_t *sfxinfo, int channel,
                               int vol, int sep)
{
    // Set up channel for playback
    channels[channel].sample_data = sfxinfo->data;
    channels[channel].length = sfxinfo->length;
    channels[channel].position = 0;
    channels[channel].volume = vol;
    channels[channel].separation = sep;
    channels[channel].playing = true;

    return channel;
}

static void I_STM32_StopSound(int channel)
{
    channels[channel].playing = false;
}

static void I_STM32_UpdateSound(void)
{
    // Called each frame - nothing needed (DMA handles updates)
}

// Sound module interface
sound_module_t sound_stm32_module =
{
    NULL,  // SoundDevices (not used)
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
```

#### Task 5.2: PCM Sound Mixing

```c
void Audio_MixCallback(int16_t* buffer, int samples)
{
    // Clear buffer
    memset(buffer, 0, samples * 2 * sizeof(int16_t));

    // Mix all active sound channels
    for (int ch = 0; ch < 8; ch++)
    {
        if (!channels[ch].playing) continue;

        for (int i = 0; i < samples && channels[ch].position < channels[ch].length; i++)
        {
            // Get 8-bit unsigned sample
            uint8_t sample_u8 = channels[ch].sample_data[channels[ch].position++];

            // Convert to 16-bit signed (-32768 to +32767)
            int16_t sample = ((int16_t)sample_u8 - 128) << 8;

            // Apply volume (0-127)
            sample = (sample * channels[ch].volume) >> 7;

            // Apply stereo separation
            int left_vol = 255 - channels[ch].separation;
            int right_vol = channels[ch].separation;

            // Mix into buffer
            buffer[i * 2 + 0] += (sample * left_vol) >> 8;  // Left
            buffer[i * 2 + 1] += (sample * right_vol) >> 8; // Right
        }

        // Check if sound finished
        if (channels[ch].position >= channels[ch].length)
        {
            channels[ch].playing = false;
        }
    }

    // TODO: Add music (OPL synthesis output)
    // MixMusicIntoBuffer(buffer, samples);
}
```

---

### Phase 6: OPL Music Integration

#### Task 6.1: OPL Synthesis in Audio Callback

```c
// In audio_stm32.c
#include "opl.h"

static void MixMusicIntoBuffer(int16_t* buffer, int samples)
{
    int16_t music_buffer[samples * 2];  // Stereo

    // Generate OPL samples
    // Note: OPL runs at 49.716 kHz, may need resampling to 44.1 kHz
    // For simplicity, we can run OPL at 44.1 kHz directly
    OPL_Generate(music_buffer, samples);

    // Mix music with sound effects
    for (int i = 0; i < samples * 2; i++)
    {
        int32_t mixed = buffer[i] + music_buffer[i];

        // Clamp to prevent overflow
        if (mixed > 32767) mixed = 32767;
        if (mixed < -32768) mixed = -32768;

        buffer[i] = (int16_t)mixed;
    }
}
```

#### Task 6.2: Connect Music Module

**File:** `/work/chocdoom/i_sound.c`

```c
// Add extern declaration
extern sound_module_t sound_stm32_module;
extern music_module_t music_opl_module;

// Update module arrays
static sound_module_t *sound_modules[] =
{
#ifdef FEATURE_SOUND
    &sound_stm32_module,  // NEW: STM32 backend
#endif
    NULL,
};

static music_module_t *music_modules[] =
{
#ifdef FEATURE_SOUND
    &music_opl_module,  // OPL music
#endif
    NULL,
};
```

---

### Phase 7: Testing & Verification

#### Task 7.1: Software Test Pattern

**In `audio_stm32.c`:**

```c
void Audio_GenerateTestTone(int16_t* buffer, int samples, int frequency)
{
    static float phase = 0.0f;
    float phase_increment = (2.0f * 3.14159f * frequency) / 44100.0f;

    for (int i = 0; i < samples; i++)
    {
        int16_t sample = (int16_t)(sinf(phase) * 16000.0f);
        buffer[i * 2 + 0] = sample;  // Left
        buffer[i * 2 + 1] = sample;  // Right

        phase += phase_increment;
        if (phase > 2.0f * 3.14159f) phase -= 2.0f * 3.14159f;
    }
}
```

#### Task 7.2: Debugger Verification

**Verification points:**
1. Set breakpoint in `HAL_SAI_TxHalfCpltCallback`
2. Verify callback fires at ~86 Hz (44100 / 512)
3. Inspect `audio_buffer[]` contains non-zero values
4. Check `channels[].playing` flags change correctly
5. Verify SAI registers show active transmission

---

## Hardware Target Options

### Recommended: PCM5102A Self-Configuring DAC ⭐

**Module:** GY-PCM5102 or CJMCU-5102
**Cost:** ~$2-5
**Specs:**
- 112 dB SNR, 32-bit capable
- No I2C configuration needed
- 3.3V compatible
- Built-in headphone driver

**Connections:**
```
STM32          PCM5102A
────────────   ─────────
PA0 (SD_B)  →  DIN
PA2 (SCK_B) →  BCK
PG9 (FS)    →  LRCK/WS
3.3V        →  VCC
GND         →  GND

Hardware strapping (on PCM5102A module):
FMT  → GND (I2S format)
DEMP → GND (de-emphasis off)
XSMT → 3.3V (soft mute control)
```

**Output:** 3.5mm jack or RCA connectors (module-dependent)

---

### Alternative 1: MAX98357A (with Amplifier)

**Cost:** ~$3-5
**Specs:**
- Class D amplifier built-in (3W output)
- I2S input
- Direct speaker connection
- No I2C needed

**Best for:** Built-in speaker (no external amp needed)

**Connections:**
```
STM32          MAX98357A
────────────   ─────────
PA0 (SD_B)  →  DIN
PA2 (SCK_B) →  BCLK
PG9 (FS)    →  LRC
3.3V        →  VIN
GND         →  GND

Output:
OUTP → Speaker (+)
OUTM → Speaker (-)
```

---

### Alternative 2: UDA1334A

**Cost:** ~$4
**Specs:**
- I2S DAC with line output
- Self-configuring
- Lower noise than PCM5102A
- 3.3V compatible

**Connections:**
```
STM32          UDA1334A
────────────   ─────────
PA0 (SD_B)  →  DIN
PA2 (SCK_B) →  BCLK
PG9 (FS)    →  WSEL
3.3V        →  VIN
GND         →  GND
```

---

### Alternative 3: PCM5100 (Higher End)

**Cost:** ~$8
**Specs:**
- 106 dB SNR, 32-bit
- Differential output
- Better than PCM5102A, more expensive
- Professional audio quality

---

### Alternative 4: CS43L22 (Full Codec)

**Cost:** ~$8
**Specs:**
- Used on STM32 Discovery boards
- I2S + I2C control
- Headphone amp built-in
- More complex but more features

**Requires:** I2C configuration (PB8/PB9)

**Connections:**
```
STM32          CS43L22
────────────   ─────────
PA0 (SD_B)  →  SDIN
PA2 (SCK_B) →  SCLK
PG9 (FS)    →  LRCK
PB8 (I2C_SCL) → SCL
PB9 (I2C_SDA) → SDA
3.3V        →  VDD
GND         →  GND
```

---

### Alternative 5: S/PDIF Optical Output

**Module:** TOTX173 Toslink transmitter
**Cost:** ~$2-3
**Specs:**
- Digital optical output
- Connects to home theater receiver
- Only uses PA0 pin (SAI in S/PDIF mode)

**Best for:** Connecting to existing audio receiver/soundbar

**Connections:**
```
STM32          TOTX173
────────────   ─────────
PA0 (SD_B)  →  Input
3.3V        →  VCC
GND         →  GND

Output: Fiber optic cable to receiver
```

**Note:** Requires SAI configuration change to S/PDIF protocol mode

---

## File Structure Summary

```
/work/
├── App/
│   ├── audio_stm32.c         # NEW: STM32 audio hardware driver
│   ├── audio_stm32.h         # NEW: Audio interface
│   ├── stm32h7xx_hal_conf.h  # MODIFY: Enable HAL_SAI
│   └── CMakeLists.txt        # MODIFY: Add audio files
│
├── chocdoom/
│   ├── opl/                  # NEW: Copy from chocolate-doom
│   │   ├── dbopl.c
│   │   ├── dbopl.h
│   │   ├── opl.c
│   │   ├── opl.h
│   │   ├── opl_queue.c
│   │   ├── opl_queue.h
│   │   ├── opl_timer.c
│   │   └── opl_timer.h
│   │
│   ├── i_oplmusic.c          # NEW: Copy from chocolate-doom
│   ├── midifile.c            # NEW: Copy from chocolate-doom
│   ├── midifile.h            # NEW: Copy from chocolate-doom
│   ├── mus2mid.c             # NEW: Copy from chocolate-doom
│   ├── mus2mid.h             # NEW: Copy from chocolate-doom
│   ├── i_sound_stm32.c       # NEW: STM32 sound backend
│   ├── doomfeatures.h        # MODIFY: Enable FEATURE_SOUND
│   ├── i_sound.c             # MODIFY: Add STM32 modules
│   └── CMakeLists.txt        # MODIFY: Add OPL + audio files
│
└── LWIP/
    ├── App/lwip.c            # MODIFY: Disable link polling
    └── Target/
        └── eth_custom_phy_interface.c  # MODIFY: Disable SPI1
```

---

## Estimated Resource Usage

### Code Size
- OPL emulation: ~50 KB
- Music player: ~15 KB
- STM32 audio driver: ~10 KB
- Sound mixing: ~5 KB
- **Total:** ~80 KB (well within 64MB SDRAM)

### RAM Usage
- Audio buffers (double-buffered): 16 KB (2048 samples × 2 buffers × 2 channels × 2 bytes)
- OPL state: ~3 KB
- Sound channel state: ~1 KB
- Music decoder: ~5 KB
- **Total:** ~25 KB

### CPU Usage (estimated at 400 MHz)
- OPL synthesis: ~15%
- Sound mixing: ~5%
- DMA transfers: ~0% (hardware)
- **Total audio:** ~20%

---

## Pin Assignment Summary

| Pin | After Ethernet Init | Audio Function | Signal |
|-----|---------------------|----------------|--------|
| PA0 | Available | SAI2_SD_B | I2S Data Out |
| PA2 | Available (MDIO disabled) | SAI2_SCK_B | I2S Bit Clock |
| PD7 | Freed from SPI1 | (unused) | - |
| PG9 | Freed from SPI1 | SAI2_FS_A | I2S Frame Sync |
| PG11 | Freed from SPI1 | (unused) | - |

**Total pins used for audio:** 3 (PA0, PA2, PG9)

---

## Build Integration

### CMakeLists.txt Changes

**File:** `/work/App/CMakeLists.txt`

Add audio driver to App sources:
```cmake
# Add after existing App sources
set(APP_SOURCES
    # ... existing sources ...
    audio_stm32.c
)
```

**File:** `/work/chocdoom/CMakeLists.txt`

Add OPL and music system:
```cmake
set(DOOM_SOURCES
    # ... existing sources ...

    # OPL Music System
    opl/dbopl.c
    opl/opl.c
    opl/opl_queue.c
    opl/opl_timer.c
    i_oplmusic.c
    midifile.c
    mus2mid.c

    # STM32 Sound Backend
    i_sound_stm32.c
)

# Add OPL include directory
target_include_directories(chocdoom
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/opl  # NEW
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/App
        ${MX_Middleware_Include_Dirs}
)
```

---

## Testing Workflow

### Phase 1: Software Verification (No Hardware)

1. **Enable features and build:**
   - Enable FEATURE_SOUND
   - Enable HAL_SAI_MODULE_ENABLED
   - Build project

2. **Verify audio generation:**
   - Set breakpoint in `Audio_MixCallback`
   - Inspect `buffer[]` for non-zero values
   - Check OPL generates music samples
   - Check SFX channels mix correctly

3. **Verify DMA callbacks:**
   - Watch `dma_callback_count` variable
   - Should increment at ~86 Hz
   - Verify no underruns

4. **Check SAI peripheral:**
   - Read SAI2_Block_B->SR register
   - Verify FREQ bit toggles
   - Verify no error flags
   - Verify BSY bit set (transmitting)

### Phase 2: Hardware Testing (With PCM5102A)

1. **Connect DAC module:**
   - Wire per connection diagram
   - Double-check power/ground
   - Connect headphones to output

2. **Power on and listen:**
   - Should hear Doom audio
   - Check both music and SFX
   - Test volume controls in menu

3. **If no audio:**
   - Use logic analyzer on PA0, PA2, PG9
   - Verify I2S signals present
   - Verify ~1.4 MHz on PA2 (BCLK)
   - Verify ~44.1 kHz on PG9 (FS)
   - Check data toggles on PA0

---

## Common Issues & Solutions

### Issue: No Audio Output

**Possible causes:**
1. SAI not configured correctly → Check SAI registers
2. DMA not running → Check DMA interrupts enabled
3. Wrong GPIO alternate function → Verify AF10 for all pins
4. DAC module not powered → Check 3.3V and GND
5. Wrong DAC pin connections → Verify DIN/BCK/LRCK mapping

### Issue: Audio Glitches/Crackling

**Possible causes:**
1. DMA underruns → Increase buffer size or lower CPU load
2. Mixing callback too slow → Optimize mixing code
3. Interrupt priority too low → Raise DMA interrupt priority
4. Clock jitter → Check SAI clock configuration

### Issue: Music but No Sound Effects (or vice versa)

**Possible causes:**
1. Volume settings at 0 → Check Doom menu settings
2. Sound module not initialized → Check `sound_stm32_module` registered
3. Music module not initialized → Check `music_opl_module` registered
4. WAD file missing sounds → Verify DOOM1.WAD integrity

---

## Next Steps After Implementation

1. **Hardware assembly:**
   - Solder PCM5102A module
   - Wire connections
   - Add amplifier and speaker (optional)

2. **Optimization:**
   - Profile CPU usage
   - Tune buffer sizes
   - Optimize mixing if needed

3. **Features:**
   - Add volume control via GPIO buttons
   - Add audio visualization (spectrum analyzer on LCD)
   - Add reverb/echo effects (optional)

4. **Polish:**
   - Tune volume levels
   - Adjust stereo separation
   - Test with all Doom music and SFX

---

## References

- STM32H750 Reference Manual: SAI chapter
- DBOPL emulator: DOSBox project
- Chocolate Doom source: Audio system documentation
- I2S specification: Philips I2S bus specification
- PCM5102A datasheet: TI documentation

---

## Appendix: Audio Signal Flow

```
┌────────────────────────────────────────────────────────────┐
│ DOOM GAME ENGINE                                            │
│                                                             │
│  ┌──────────────┐         ┌─────────────┐                 │
│  │ Game Events  │──────→  │ S_StartSound│                 │
│  │ (pistol shot)│         │ (s_sound.c) │                 │
│  └──────────────┘         └──────┬──────┘                 │
│                                   │                         │
│                                   ▼                         │
│                          ┌─────────────────┐               │
│                          │I_STM32_StartSound│              │
│                          │ (i_sound_stm32.c)│              │
│                          └────────┬─────────┘              │
│                                   │                         │
│  ┌──────────────┐                 │                         │
│  │ Music System │                 │                         │
│  │ (MUS → OPL)  │                 │                         │
│  └──────┬───────┘                 │                         │
│         │                         │                         │
└─────────┼─────────────────────────┼─────────────────────────┘
          │                         │
          ▼                         ▼
┌─────────────────────────────────────────────────────────────┐
│ AUDIO MIXING (audio_stm32.c)                                │
│                                                              │
│  Audio_MixCallback() [Called by DMA interrupt at 44.1kHz]  │
│                                                              │
│  ┌──────────────┐         ┌──────────────┐                │
│  │ OPL Synthesis│         │ PCM SFX Mixer│                │
│  │ (music)      │         │ (8 channels) │                │
│  └──────┬───────┘         └──────┬───────┘                │
│         │                         │                         │
│         │  Apply volume          │ Apply volume            │
│         │  (musicVolume)         │ (sfxVolume)            │
│         ▼                         ▼                         │
│  ┌──────────────────────────────────┐                      │
│  │   Mix: music + sfx               │                      │
│  │   Apply stereo panning           │                      │
│  │   Clamp to 16-bit range          │                      │
│  └────────────┬─────────────────────┘                      │
│               │                                             │
└───────────────┼─────────────────────────────────────────────┘
                │
                ▼
┌────────────────────────────────────────────────────────────┐
│ SAI2 PERIPHERAL + DMA                                       │
│                                                             │
│  ┌──────────┐    DMA     ┌──────────┐                     │
│  │ Buffer   │─────────→  │ SAI2_DR  │                     │
│  │(SDRAM)   │  (auto)    │(hardware)│                     │
│  └──────────┘            └─────┬────┘                     │
│                                 │                           │
└─────────────────────────────────┼───────────────────────────┘
                                  │
                                  ▼
┌────────────────────────────────────────────────────────────┐
│ GPIO PINS (I2S Signals)                                     │
│                                                             │
│  PA0 (SAI2_SD_B)  ────→  Data stream (audio samples)      │
│  PA2 (SAI2_SCK_B) ────→  Bit clock (~1.4 MHz)             │
│  PG9 (SAI2_FS)    ────→  Frame sync (44.1 kHz)            │
│                                                             │
└────────────────────┬───────────────────────────────────────┘
                     │
                     ▼
            ┌─────────────────┐
            │   PCM5102A DAC   │
            │   (Hardware)     │
            └────────┬─────────┘
                     │
                     ▼
              Analog Audio Out
              (Headphones/Amp)
```

---

*End of Implementation Plan*
