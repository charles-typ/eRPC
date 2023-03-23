#ifndef PC_API_RPC_CLUSTER
#define PC_API_RPC_CLUSTER
// Platform headers
#include <linux/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

// Standard headers
#include <cstdint>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <cinttypes>
#include <iostream>
#define MAKE_IPV4_ADDR(a, b, c, d) (a + (b<<8) + (c<<16) + (d<<24))
#define NUM_QUERY 100

// Cluster info
static uint32_t compute_ip = MAKE_IPV4_ADDR(10, 10, 10, 201);
static uint32_t memory_1_ip = MAKE_IPV4_ADDR(10, 10, 10, 221);
static uint32_t memory_2_ip = MAKE_IPV4_ADDR(10, 10, 10, 222);
static uint32_t compute_port = 10000;
static uint32_t memory_1_port = 10001;
static uint32_t memory_2_port = 10002;
uint8_t g_dest_mac_addr_0[ETH_ALEN]; // compute
uint8_t g_dest_mac_addr_1[ETH_ALEN]; // memory 1
uint8_t g_dest_mac_addr_2[ETH_ALEN]; // memory 2

#endif// PC_API_RPC_CLUSTER
