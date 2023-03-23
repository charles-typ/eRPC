#include <chrono> 
#include "pc/datastructure/bplustree.h"
#include "pc/parser/local_parse.h"
#include "./config.h"
static configuration config;
int main(int argc, char*argv[]) {   


    if(ini_parse(argv[1], handler, &config) < 0) {
        printf("Can't load 'test.ini'\n");
        return 1;
    }
    std::ifstream data_stream(config.data_filepath);
    std::ifstream query_stream(config.query_filepath);

    read_time_series_keys(data_stream, config.num_keys);
    read_time_series_query(query_stream, config.num_keys);

    BPlusTree tree("./"); 

    for(int i = 0; i < config.num_keys; ++i) {
        std::cout << all_keys[i] << std::endl;
        std::string value_string = gen_random(7);
        tree.insert(all_keys[i], (void *)value_string.c_str()); 
    }
    uint64_t start = __rdtsc();
    for(int i = 0; i < config.num_queries; i++) {
        std::cout << all_query[i].key << std::endl;
        long result;
        tree.scan(all_query[i].key, all_query[i].scan_len, result, 0);
        std::cout << "Aggregation result" << std::endl;
    }
    uint64_t end = __rdtsc();
    std::cout << "Took CPU cycles: " << end - start << std::endl;
    return 0; 
}
