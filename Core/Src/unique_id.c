#include "unique_id.h"
#include <string.h>

// Network mode flag (set by main.c before MX_LWIP_Init)
bool network_server_mode = false;

// Read the STM32 unique device ID (12 bytes / 96 bits)
void GetUniqueID(uint8_t *uid)
{
    uint32_t *uid_base = (uint32_t *)STM32_UID_BASE;

    // The UID is stored as 3 x 32-bit words
    memcpy(uid, uid_base, 12);
}

// Get a specific byte from the unique ID
uint8_t GetUniqueIDByte(uint8_t index)
{
    if (index >= 12)
        return 0;

    uint8_t uid[12];
    GetUniqueID(uid);
    return uid[index];
}

// Get a unique IP address last octet based on unique ID
// Returns a value between 11-254 (avoiding .0, .1, .10, .255)
uint8_t GetUniqueIPLastOctet(void)
{
    uint8_t uid[12];
    GetUniqueID(uid);

    // XOR bytes 0-2 (X/Y wafer coordinates - highest entropy)
    // Byte 0: X-coord high, Byte 1: X-coord low, Byte 2: Y-coord high
    uint8_t result = uid[0] ^ uid[1] ^ uid[2];

    // Map to range 11-254 (244 possible values)
    // Avoid: .0 (network), .1 (gateway), .10 (server), .255 (broadcast)
    result = 11 + (result % 244);

    // If we landed on .10 (server address), skip to .11
    if (result == 10)
        result = 11;

    return result;
}
