#include <string>
#include "sys/types.h"
#include "sys/sysinfo.h"

#include "pc/datastructure/bplustree.h"
#include "pc/parser/rpc_parse.h"
#include "pc/utils/time_util.h"
#include "pc/utils/rand_util.h"
#include "pc/utils/core_util.h"
#include "pc/utils/memory_util.h"
#include "benchmark_util.h"

using namespace std;
using namespace pc::datastructure;
using namespace pc::parser;
using namespace pc::utils;

const uint64_t num_keys = 500000000;
struct btree_element
{
    char result[8];
} __attribute__((aligned(8)));

int main()
{
    init();
    BPlusTree tree("./");
    auto start = get_time();
    std::ifstream data_stream(std::string("/var/data/ycsbc-key-1-blade"));
    RpcParse parser;
    parser.read_all_keys(data_stream, num_keys);
    auto end_1 = get_time();
    std::cout << "Takes " << (end_1 - start) / 1000 / 1000 /1000 << " seconds to read keys to memory" << std::endl;
    // Insert random key and values
    vector<string> ctr(num_keys);
    for (int i = 0; i < num_keys; ++i)
    {
        ctr[i] = gen_random(7);
    }
    auto end_2 = get_time();
    std::cout << "Takes " << (end_2 - end_1) / 1000 / 1000 /1000 << " seconds to generate random values " << std::endl;
    for (int i = 0; i < num_keys; ++i)
    {
        tree.insert(parser.all_keys[i], (void *)ctr[i].c_str());
        // std::cout << "Insert complete" << endl;
    }
    auto end_3 = get_time();
    std::cout << "Takes " << (end_3 - end_2) / 1000 / 1000 /1000 << " seconds to insert to B+Tree " << std::endl;

    collect_memory_stat();
    std::cout << "Current virtual memory used by process is: " << virtual_memory_used_process() << std::endl;
    tree.print_statistics();

    uint64_t num_queries = 10000000;
    std::ifstream query_stream(std::string("/var/data/ycsbe-query-1-blade"));
    parser.read_all_query(query_stream, num_queries);

    auto end_4 = get_time();
    std::cout << "Takes " << (end_4 - end_3) / 1000 / 1000 /1000 << " seconds to load queries " << std::endl;
    PinToCore(0);
    for (int i = 0; i < num_queries; i++) {
        std::vector<struct bplustree_entry> result;
        auto node_count = tree.scan(parser.all_query[i].key, parser.all_query[i].scan_len, result);
        std::cout << "Node count: " << node_count << " for scan length: " << parser.all_query[i].scan_len << std::endl;
    }
    auto end_5 = get_time();
    std::cout << "Takes " << (end_5 - end_4) / 1000 / 1000 /1000 << " seconds to execute queries " << std::endl;

    return 0;
}
