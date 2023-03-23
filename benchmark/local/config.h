#ifndef PC_API_BENCH_LOCAL_CONFIG_H
#define PC_API_BENCH_LOCAL_CONFIG_H
#include "./deps/inih/ini.h"


#include <stdint.h>
#include <inttypes.h>


// For measure CPU cycles
#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif


int64_t S64(const char *s) {
  int64_t i;
  char c ;
  int scanned = sscanf(s, "%" SCNd64 "%c", &i, &c);
  if (scanned == 1) return i;
  if (scanned > 1) {
    // TBD about extra data found
    return i;
    }
  // TBD failed to scan;
  return 0;
}



typedef struct {
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


static int handler(void* user, const char* section, const char* name, const char* value) {
    configuration* pconfig = (configuration*)user;
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("workload", "num_keys")) {
        pconfig->num_keys = S64(value);
    } else if (MATCH("workload", "num_queries")) {
        pconfig->num_queries = S64(value);
    } else if (MATCH("workload", "distribution")) {
        pconfig->distribution = atoi(value);
    } else if (MATCH("workload", "num_blades")) {
        pconfig->num_blades = atoi(value);
    } else if (MATCH("workload", "allocation")) {
        pconfig->allocation = atoi(value);
    } else if (MATCH("workload", "data_filepath")) {
        pconfig->data_filepath = std::string(value);
    } else if (MATCH("workload", "query_filepath")) {
        pconfig->query_filepath = std::string(value);
    } else if (MATCH("workload", "memory_filepath")) {
        pconfig->memory_filepath = std::string(value);
    } else if (MATCH("workload", "bucket_filepath")) {
        pconfig->bucket_filepath = std::string(value);
    } else if (MATCH("datastructure", "base")) {
        pconfig->base_addr = (void*)std::stoull((std::string(value)), nullptr, 16);
    } else if (MATCH("datastructure", "capacity")) {
        pconfig->hash_capacity = S64(value);
    } else if (MATCH("datastructure", "memory_1_min_range")) {
        pconfig->memory_1_min_range = (void*)std::stoull((std::string(value)), nullptr, 16);
    } else if (MATCH("datastructure", "memory_1_max_range")) {
        pconfig->memory_1_max_range = (void*)std::stoull((std::string(value)), nullptr, 16);
    } else if (MATCH("datastructure", "memory_2_min_range")) {
        pconfig->memory_2_min_range = (void*)std::stoull((std::string(value)), nullptr, 16);
    } else if (MATCH("datastructure", "memory_2_max_range")) {
        pconfig->memory_2_max_range = (void*)std::stoull((std::string(value)), nullptr, 16);
    } else {
        return 0;
    }
    return 1;
}






std::string gen_random(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}
#endif // PC_API_BENCH_LOCAL_CONFIG_H