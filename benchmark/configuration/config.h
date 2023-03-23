//
// Created by yupeng on 1/18/23.
//

#ifndef PC_LIBPC_SRC_PC_CONFIGURATION_CONFIG_H_
#define PC_LIBPC_SRC_PC_CONFIGURATION_CONFIG_H_

#include "cluster.h"
#include "ini.h"

typedef struct {
  uint32_t srcIp;
  uint32_t srcPort;
  //int datastructure; // 0 hash table 1 B+tree scan 2 B+tree aggregate
  int mode; // 0 client 1 server
  int hostid; // 0 compute 1,2 memory
  int64_t num_keys;
  int64_t num_queries;
  int distribution; // 0 uniform 1 zipfian
  int num_blades;
  int allocation; // 0 random 1 partition
  std::string data_filepath;
  std::string query_filepath;
  std::string memory_filepath;
  std::string bucket_filepath;
  uint64_t hash_capacity;
  void* base_addr;
  void* memory_1_min_range;
  void* memory_1_max_range;
  void* memory_2_min_range;
  void* memory_2_max_range;
} configuration;

#endif //PC_LIBPC_SRC_PC_CONFIGURATION_CONFIG_H_
