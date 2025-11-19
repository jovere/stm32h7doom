//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2025 STM32 Port
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//     Networking module using LwIP Raw UDP API
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "doomtype.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_misc.h"
#include "net_defs.h"
#include "net_io.h"
#include "net_packet.h"
#include "net_lwip.h"
#include "z_zone.h"

// LwIP includes
#include "lwip/udp.h"
#include "lwip/ip_addr.h"

#define DEFAULT_PORT 2342

// Received packet queue (simple ring buffer)
#define RX_QUEUE_SIZE 16

typedef struct
{
    net_packet_t *packet;
    ip_addr_t addr;
    u16_t port;
} rx_queue_entry_t;

static struct udp_pcb *udp_pcb_doom = NULL;
static boolean initted = false;
static int port = DEFAULT_PORT;

// Receive queue
static rx_queue_entry_t rx_queue[RX_QUEUE_SIZE];
static int rx_queue_head = 0;
static int rx_queue_tail = 0;
static int rx_queue_count = 0;

// Address table for managing net_addr_t structures
typedef struct
{
    net_addr_t net_addr;
    ip_addr_t lwip_addr;
    u16_t port;
} addrpair_t;

static addrpair_t **addr_table = NULL;
static int addr_table_size = -1;

// Initialize address table
static void NET_LwIP_InitAddrTable(void)
{
    addr_table_size = 16;
    addr_table = Z_Malloc(sizeof(addrpair_t *) * addr_table_size, PU_STATIC, 0);
    memset(addr_table, 0, sizeof(addrpair_t *) * addr_table_size);
}

// Compare IP addresses
static boolean AddressesEqual(ip_addr_t *a, u16_t port_a, ip_addr_t *b, u16_t port_b)
{
    return ip_addr_cmp(a, b) && (port_a == port_b);
}

// Find or create address entry
static net_addr_t *NET_LwIP_FindAddress(ip_addr_t *addr, u16_t port)
{
    addrpair_t *new_entry;
    int empty_entry = -1;
    int i;

    if (addr_table_size < 0)
    {
        NET_LwIP_InitAddrTable();
    }

    // Search for existing entry
    for (i = 0; i < addr_table_size; ++i)
    {
        if (addr_table[i] != NULL &&
            AddressesEqual(addr, port, &addr_table[i]->lwip_addr, addr_table[i]->port))
        {
            return &addr_table[i]->net_addr;
        }

        if (empty_entry < 0 && addr_table[i] == NULL)
            empty_entry = i;
    }

    // Need to add new entry - expand table if necessary
    if (empty_entry < 0)
    {
        addrpair_t **new_addr_table;
        int new_addr_table_size;

        empty_entry = addr_table_size;
        new_addr_table_size = addr_table_size * 2;
        new_addr_table = Z_Malloc(sizeof(addrpair_t *) * new_addr_table_size, PU_STATIC, 0);
        memset(new_addr_table, 0, sizeof(addrpair_t *) * new_addr_table_size);
        memcpy(new_addr_table, addr_table, sizeof(addrpair_t *) * addr_table_size);
        Z_Free(addr_table);
        addr_table = new_addr_table;
        addr_table_size = new_addr_table_size;
    }

    // Create new entry
    new_entry = Z_Malloc(sizeof(addrpair_t), PU_STATIC, 0);
    ip_addr_copy(new_entry->lwip_addr, *addr);
    new_entry->port = port;
    new_entry->net_addr.module = &net_lwip_module;
    new_entry->net_addr.handle = new_entry;

    addr_table[empty_entry] = new_entry;

    return &new_entry->net_addr;
}

// Free an address
static void NET_LwIP_FreeAddress(net_addr_t *addr)
{
    int i;

    for (i = 0; i < addr_table_size; ++i)
    {
        if (addr == &addr_table[i]->net_addr)
        {
            Z_Free(addr_table[i]);
            addr_table[i] = NULL;
            return;
        }
    }

    I_Error("NET_LwIP_FreeAddress: Attempted to remove an unused address!");
}

// UDP receive callback - called by LwIP when packet arrives
static void udp_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                               const ip_addr_t *addr, u16_t port)
{
    rx_queue_entry_t *entry;

    // Check if queue is full
    if (rx_queue_count >= RX_QUEUE_SIZE)
    {
        // Queue full, drop packet
        pbuf_free(p);
        return;
    }

    // Allocate packet structure
    net_packet_t *packet = NET_NewPacket(p->tot_len);
    if (packet == NULL)
    {
        pbuf_free(p);
        return;
    }

    // Copy data from pbuf chain
    pbuf_copy_partial(p, packet->data, p->tot_len, 0);
    packet->len = p->tot_len;

    // Add to queue
    entry = &rx_queue[rx_queue_tail];
    entry->packet = packet;
    ip_addr_copy(entry->addr, *addr);
    entry->port = port;

    rx_queue_tail = (rx_queue_tail + 1) % RX_QUEUE_SIZE;
    rx_queue_count++;

    pbuf_free(p);
}

// Initialize as client
static boolean NET_LwIP_InitClient(void)
{
    int p;

    if (initted)
        return true;

    // Check for -port parameter
    p = M_CheckParmWithArgs("-port", 1);
    if (p > 0)
        port = atoi(myargv[p + 1]);

    // Create UDP PCB
    udp_pcb_doom = udp_new();
    if (udp_pcb_doom == NULL)
    {
        I_Error("NET_LwIP_InitClient: Unable to create UDP PCB!");
        return false;
    }

    // Bind to any local port (client)
    err_t err = udp_bind(udp_pcb_doom, IP_ADDR_ANY, 0);
    if (err != ERR_OK)
    {
        I_Error("NET_LwIP_InitClient: Unable to bind UDP socket!");
        return false;
    }

    // Set receive callback
    udp_recv(udp_pcb_doom, udp_recv_callback, NULL);

    // Initialize receive queue
    rx_queue_head = 0;
    rx_queue_tail = 0;
    rx_queue_count = 0;

    initted = true;
    return true;
}

// Initialize as server
static boolean NET_LwIP_InitServer(void)
{
    int p;

    if (initted)
        return true;

    // Check for -port parameter
    p = M_CheckParmWithArgs("-port", 1);
    if (p > 0)
        port = atoi(myargv[p + 1]);

    // Create UDP PCB
    udp_pcb_doom = udp_new();
    if (udp_pcb_doom == NULL)
    {
        I_Error("NET_LwIP_InitServer: Unable to create UDP PCB!");
        return false;
    }

    // Bind to specific port (server)
    err_t err = udp_bind(udp_pcb_doom, IP_ADDR_ANY, port);
    if (err != ERR_OK)
    {
        I_Error("NET_LwIP_InitServer: Unable to bind to port %i", port);
        return false;
    }

    // Set receive callback
    udp_recv(udp_pcb_doom, udp_recv_callback, NULL);

    // Initialize receive queue
    rx_queue_head = 0;
    rx_queue_tail = 0;
    rx_queue_count = 0;

    initted = true;
    return true;
}

// Send a packet
static void NET_LwIP_SendPacket(net_addr_t *addr, net_packet_t *packet)
{
    struct pbuf *p;
    addrpair_t *addrpair;
    ip_addr_t dest_addr;
    u16_t dest_port;

    if (!initted || udp_pcb_doom == NULL)
        return;

    // Handle broadcast
    if (addr == &net_broadcast_addr)
    {
        ip_addr_set_any(false, &dest_addr);
        ip_addr_set_hton(&dest_addr, IP_ADDR_BROADCAST);
        dest_port = port;
    }
    else
    {
        addrpair = (addrpair_t *) addr->handle;
        ip_addr_copy(dest_addr, addrpair->lwip_addr);
        dest_port = addrpair->port;
    }

    // Allocate pbuf
    p = pbuf_alloc(PBUF_TRANSPORT, packet->len, PBUF_RAM);
    if (p == NULL)
        return;

    // Copy packet data
    memcpy(p->payload, packet->data, packet->len);

    // Send
    udp_sendto(udp_pcb_doom, p, &dest_addr, dest_port);

    // Free pbuf
    pbuf_free(p);
}

// Receive a packet (non-blocking, returns from queue)
static boolean NET_LwIP_RecvPacket(net_addr_t **addr, net_packet_t **packet)
{
    rx_queue_entry_t *entry;

    // Process LwIP stack to handle incoming/outgoing UDP packets
    extern void MX_LWIP_Process(void);
    MX_LWIP_Process();

    if (rx_queue_count == 0)
        return false;

    // Get packet from queue
    entry = &rx_queue[rx_queue_head];
    *packet = entry->packet;
    *addr = NET_LwIP_FindAddress(&entry->addr, entry->port);

    rx_queue_head = (rx_queue_head + 1) % RX_QUEUE_SIZE;
    rx_queue_count--;

    return true;
}

// Convert address to string
static void NET_LwIP_AddrToString(net_addr_t *addr, char *buffer, int buffer_len)
{
    addrpair_t *addrpair = (addrpair_t *) addr->handle;

    snprintf(buffer, buffer_len, "%s:%d",
             ip4addr_ntoa(&addrpair->lwip_addr),
             addrpair->port);
}

// Resolve address from string (format: "192.168.1.10" or "192.168.1.10:2342")
static net_addr_t *NET_LwIP_ResolveAddress(char *address)
{
    ip_addr_t ip;
    u16_t addr_port = port;
    char *addr_hostname;
    char *colon;

    if (address == NULL)
        return NULL;

    // Check for port specification
    colon = strchr(address, ':');
    addr_hostname = M_StringDuplicate(address);

    if (colon != NULL)
    {
        addr_hostname[colon - address] = '\0';
        addr_port = atoi(colon + 1);
    }

    // Parse IP address
    if (!ip4addr_aton(addr_hostname, &ip))
    {
        free(addr_hostname);
        return NULL;  // Invalid IP address
    }

    free(addr_hostname);

    return NET_LwIP_FindAddress(&ip, addr_port);
}

// Network module structure
net_module_t net_lwip_module =
{
    NET_LwIP_InitClient,
    NET_LwIP_InitServer,
    NET_LwIP_SendPacket,
    NET_LwIP_RecvPacket,
    NET_LwIP_AddrToString,
    NET_LwIP_FreeAddress,
    NET_LwIP_ResolveAddress,
};
