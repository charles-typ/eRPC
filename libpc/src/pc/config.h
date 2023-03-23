//
// Created by yupeng on 1/19/23.
//

#ifndef PC_LIBPC_SRC_PC_CONFIG_H_
#define PC_LIBPC_SRC_PC_CONFIG_H_

// Standard headers
#include <cstdint>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <cinttypes>
#include <iostream>

// Utils
#include "pc/utils/time_util.h"
#include "pc/utils/net_util.h"
#include "pc/datastructure/hashtable.h"
#include "ini.h"

// DPDK
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include <rte_mbuf_core.h>

using namespace pc::utils;

namespace pc
{
  static constexpr size_t kDefaultMessageQueueSize = 512;
  static constexpr size_t kDefaultWorkerQueueSize = 256;
  // Max number of worker threads allowed
  // static constexpr size_t kMaxWorkerNum = 50;
  static constexpr size_t kScheduleQueues = 2;
  // Dequeue batch size, used to reduce the overhead of dequeue in main thread
  static constexpr size_t kDequeueBulkSizeTXRX = 8;
  static constexpr size_t kDequeueBulkSizeWorker = 4;

  // Enable thread pinning and exit if thread pinning fails. Thread pinning is
  // crucial for good performance. For testing or developing Agora on machines
  // with insufficient cores, disable this flag.
  static constexpr bool kEnableThreadPinning = true;
  static constexpr bool kEnableCoreReuse = false;

  enum data_structure_type
  {
    hashtable,
    btree_agg,
    btree_scan
  };


  typedef struct
  {
    // Host Config
    rte_ether_addr srcMac;
    uint32_t srcIp;
    uint16_t srcPort;
    rte_ether_addr dstMac;
    uint32_t dstIp;
    uint16_t dstPort;
    int mode;   // 0 client 1 server
    int hostid; // 0 compute 1,2 memory
    int num_worker_threads; 
    int num_network_threads; 
    // Workload Config
    uint64_t num_keys;
    uint64_t num_queries;
    int distribution; // 0 uniform 1 zipfian
    int num_blades;
    int allocation; // 0 random 1 partition
    std::string data_filepath;
    std::string query_filepath;
    std::string memory_filepath;
    std::string bucket_filepath;
    uint64_t hash_capacity;
    // Data structure Config
    data_structure_type ds_type;
    void *base_addr;
    void *memory_1_min_range;
    void *memory_1_max_range;
    void *memory_2_min_range;
    void *memory_2_max_range;
    // Internal config
    bool running;
    int packet_length;
    int buffer_length;
  } Config;

  static int handler(void *user, const char *section, const char *name, const char *value)
  {
    Config *pconfig = static_cast<Config *>(user);
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("host", "srcMac"))
    {
      rte_ether_unformat_addr(value, &(pconfig->srcMac));
    }
    else if (MATCH("host", "srcIp"))
    {
      inet_pton(AF_INET, value, &(pconfig->srcIp));
    }
    else if (MATCH("host", "srcPort"))
    {
      pconfig->srcPort = atoi(value);
    }
    else if (MATCH("host", "dstMac"))
    {
      rte_ether_unformat_addr(value, &(pconfig->dstMac));
    }
    else if (MATCH("host", "dstIp"))
    {
      inet_pton(AF_INET, value, &(pconfig->dstIp));
    }
    else if (MATCH("host", "dstPort"))
    {
      pconfig->dstPort = atoi(value);
    }
    else if (MATCH("host", "mode"))
    {
      pconfig->mode = atoi(value);
    }
    else if (MATCH("host", "hostid"))
    {
      pconfig->hostid = atoi(value);
    }
    else if (MATCH("host", "num_worker_threads"))
    {
      pconfig->num_worker_threads = atoi(value);
    }
    else if (MATCH("host", "num_network_threads"))
    {
      pconfig->num_network_threads = atoi(value);
    }
    else if (MATCH("workload", "num_keys"))
    {
      pconfig->num_keys = S64(value);
    }
    else if (MATCH("workload", "num_queries"))
    {
      pconfig->num_queries = S64(value);
    }
    else if (MATCH("workload", "distribution"))
    {
      pconfig->distribution = atoi(value);
    }
    else if (MATCH("workload", "num_blades"))
    {
      pconfig->num_blades = atoi(value);
    }
    else if (MATCH("workload", "allocation"))
    {
      pconfig->allocation = atoi(value);
    }
    else if (MATCH("workload", "data_filepath"))
    {
      pconfig->data_filepath = std::string(value);
    }
    else if (MATCH("workload", "query_filepath"))
    {
      pconfig->query_filepath = std::string(value);
    }
    else if (MATCH("workload", "memory_filepath"))
    {
      pconfig->memory_filepath = std::string(value);
    }
    else if (MATCH("workload", "bucket_filepath"))
    {
      pconfig->bucket_filepath = std::string(value);
    }
    else if (MATCH("datastructure", "ds_type"))
    {
      pconfig->ds_type = static_cast<data_structure_type>(atoi(value));
    }
    else if (MATCH("datastructure", "base"))
    {
      pconfig->base_addr = reinterpret_cast<void *>(std::stoull((std::string(value)), nullptr, 16));
    }
    else if (MATCH("datastructure", "capacity"))
    {
      pconfig->hash_capacity = S64(value);
    }
    else if (MATCH("datastructure", "memory_1_min_range"))
    {
      pconfig->memory_1_min_range = reinterpret_cast<void *>(std::stoull((std::string(value)), nullptr, 16));
    }
    else if (MATCH("datastructure", "memory_1_max_range"))
    {
      pconfig->memory_1_max_range = reinterpret_cast<void *>(std::stoull((std::string(value)), nullptr, 16));
    }
    else if (MATCH("datastructure", "memory_2_min_range"))
    {
      pconfig->memory_2_min_range = reinterpret_cast<void *>(std::stoull((std::string(value)), nullptr, 16));
    }
    else if (MATCH("datastructure", "memory_2_max_range"))
    {
      pconfig->memory_2_max_range = reinterpret_cast<void *>(std::stoull((std::string(value)), nullptr, 16));
    }
    else if (MATCH("internal", "packet_length"))
    {
      pconfig->packet_length = atoi(value);
    }
    else if (MATCH("internal", "buffer_length"))
    {
      pconfig->buffer_length = atoi(value);
    }
    else
    {
      return 0;
    }
    return 1;
  }

  using pc::datastructure::hashtable::key_type;

  // 40 bytes
  struct Request
  {
    data_structure_type ds;
    uint64_t start_addr;
    key_type key;
    uint64_t base;
    uint64_t ht;
  };

  // 40 bytes
  struct Response
  {
    uint64_t result;
    char padding[32];
  };

}

#endif // PC_LIBPC_SRC_PC_CONFIG_H_
