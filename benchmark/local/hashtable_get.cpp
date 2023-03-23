#include <chrono>
#include "pc/datastructure/hashtable.h"
#include "pc/parser/local_parse.h"
#include "./config.h"

static configuration config;

int pin_to_core(int core_id)
{
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (core_id < 0 || core_id >= num_cores)
        return -1;

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    pthread_t current_thread = pthread_self();
    return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

int main(int argc, char*argv[]) {
    if(ini_parse(argv[1], handler, &config) < 0) {
        printf("Can't load 'test.ini'\n");
        return 1;
    }
    std::ifstream data_stream(config.data_filepath);
    std::ifstream query_stream(config.query_filepath);

    read_all_keys(data_stream, config.num_keys);
    read_all_query(query_stream, config.num_queries);

    HashTable::HashTable ht;
    std::cout << "Start inserting keys" << std::endl;
    for(int i = 0; i < config.num_keys; ++i) {
        std::string value = gen_random(7);
        ht.insert(std::make_pair(i, value.c_str()));
    }

    uint64_t start = __rdtsc();
    for(int i = 0; i < config.num_queries; i++) {
        auto result = ht.find(all_query[i].key);
    }
    uint64_t end = __rdtsc();
    std::cout << "Took CPU cycles: " << end - start << std::endl;

    return 0;
}
