#ifndef __UNIQUE_ID_H
#define __UNIQUE_ID_H

#include <stdint.h>
#include <stdbool.h>

// STM32H7 unique ID is 96 bits at this address
#define STM32_UID_BASE 0x1FF1E800UL

// Read the STM32 unique device ID (12 bytes / 96 bits)
void GetUniqueID(uint8_t *uid);

// Get a unique byte derived from the unique ID
// index: which byte to return (0-11)
uint8_t GetUniqueIDByte(uint8_t index);

// Get a unique IP address last octet based on unique ID
// Returns a value between 11-254 (avoiding .0, .1, .10, .255)
uint8_t GetUniqueIPLastOctet(void);

// Network mode flag (set before MX_LWIP_Init is called)
extern bool network_server_mode;

#endif /* __UNIQUE_ID_H */
