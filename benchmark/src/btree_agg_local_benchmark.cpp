
#include <string>
#include "sys/types.h"
#include "sys/sysinfo.h"

#include "pc/datastructure/bplustree.h"
#include "pc/parser/rpc_parse.h"
#include "benchmark_util.h"

using namespace std;
using namespace pc::datastructure;
using namespace pc::parser;
using namespace pc::utils;

const uint64_t num_keys = 500000000;
//const uint64_t num_queries = 10000000;
const uint64_t num_queries = 5;

struct btree_element
{
    char result[8];
} __attribute__((aligned(8)));

int main()
{
    BPlusTree tree("./");
    //auto start = get_time();
    std::ifstream data_stream(std::string("/var/data/time-key-1-blade"));
    RpcParse parser;
    parser.read_all_keys(data_stream, num_keys, true);
    //auto end_1 = get_time();
    //std::cout << "Takes " << (end_1 - start) / 1000 / 1000 /1000 << " seconds to read keys to memory" << std::endl;
    // Insert random key and values
    vector<string> ctr(num_keys);
    for (uint64_t i = 0; i < num_keys; ++i)
    {
        ctr[i] = gen_random(7);
    }
    //auto end_2 = get_time();
    //std::cout << "Takes " << (end_2 - end_1) / 1000 / 1000 /1000 << " seconds to generate random values " << std::endl;
    for (uint64_t i = 0; i < num_keys; ++i)
    {
        tree.insert(parser.all_keys[i], const_cast<void*>(reinterpret_cast<const void *>(ctr[i].c_str())));
        // std::cout << "Insert complete" << endl;
    }
    //auto end_3 = get_time();
    //std::cout << "Takes " << (end_3 - end_2) / 1000 / 1000 /1000 << " seconds to insert to B+Tree " << std::endl;

    tree.print_statistics();

    std::vector<uint64_t> query;
    uint64_t start = 2147483648;
    for(int i = 0; i < 20; i++) {
        query.push_back(start * std::pow(2, i));
    }


    //std::ifstream query_stream(std::string("/var/data/time-query-1-blade"));
    //parser.read_time_series_query(query_stream, num_queries);

    long result;
    uint64_t repeat = 10;
    for(uint64_t i = 0; i < 20; i++) {
        uint64_t total_time = 0;
	    for(uint64_t j = 0; j < repeat; j++)
	    {
            auto start_time = std::chrono::high_resolution_clock::now();
        	tree.aggregate_time_stat(parser.all_keys[0], query[i], result);
            auto end_time = std::chrono::high_resolution_clock::now();
            total_time+= static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end_time-start_time).count());
	    }
        std::cout << "Takes " <<  total_time / repeat << " nanoseconds to aggregate "<< start * std::pow(2, i) <<" queries." << std::endl;
    }

    return 0;
}
