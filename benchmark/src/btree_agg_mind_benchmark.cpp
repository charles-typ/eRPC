#include <chrono>
#include <iomanip>
#include <ctime>
#include <unistd.h>
#include "bplustree_mind.h"
#include "pc/parser/rpc_parse.h"
#include "mind_malloc.h"
#include "benchmark_util.h"

using namespace pc::parser;

// std::string data_filepath = "../../data/load.dat";
// std::string query_filepath = "../../data/transactionsE.dat";
const std::string latency_result = "/home/sslee/pulse_bench/pc-api/bench/mind/log/btagg_lat.txt";

#ifdef MEM_DUAL_BLADES
#ifdef VERBOSE_FLAG
const int num_keys = 10000;
const std::string mem_layout_name = "/home/sslee/pulse_bench/pc-api/data/btree_agg_layout_dual_test.bin";
const int num_memory_regions = 2;
#else
const int num_keys = (1000000000 * 0.94);
#ifdef MIND_MMAP_RR
const std::string mem_layout_name = "/home/sslee/pulse_bench/pc-api/data/btree_agg_layout_dual_rr.bin";
#else
const std::string mem_layout_name = "/home/sslee/pulse_bench/pc-api/data/btree_agg_layout_dual.bin";
#endif
const int num_memory_regions = 12 * (MAX_MIND_ALLOC_SIZE / TEST_MACRO_ALLOC_SIZE); // x 4 GB per region
#endif

const std::string data_filepath = "/home/sslee/pulse_bench/pc-api/data/time-key-2-blade";
const std::string query_filepath = "/home/sslee/pulse_bench/pc-api/data/time-query-2-blade";
#else
#ifdef VERBOSE_FLAG
const int num_keys = 10000;
const std::string mem_layout_name = "/home/sslee/pulse_bench/pc-api/data/btree_agg_layout_single_test.bin";
const int num_memory_regions = 1; // x 4 GB per region
#else
const int num_keys = (500000000 * 0.94);
#ifdef MIND_MMAP_RR
const std::string mem_layout_name = "/home/sslee/pulse_bench/pc-api/data/btree_agg_layout_single_rr.bin";
#else
const std::string mem_layout_name = "/home/sslee/pulse_bench/pc-api/data/btree_agg_layout_single.bin";
#endif
const int num_memory_regions = 6 * (MAX_MIND_ALLOC_SIZE / TEST_MACRO_ALLOC_SIZE); // x 4 GB per region
#endif

const std::string data_filepath = "/home/sslee/pulse_bench/pc-api/data/time-key-1-blade";
const std::string query_filepath = "/home/sslee/pulse_bench/pc-api/data/time-query-1-blade";
#endif
#ifdef VERBOSE_FLAG
const int num_queries = 100;
#else
const int num_queries = 100000;   // 10000000;
#endif

void dump_latency(unsigned int *latency_array, size_t len, unsigned int total_duration)
{
    std::cout << "=print latency result=...";
    std::ostringstream result_str;
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    result_str << latency_result << "_";
    result_str << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S");
    std::ofstream of_latency(result_str.str());
    of_latency << "Total latency: " << total_duration << " us" << std::endl;
    of_latency << "Data fille: " << data_filepath << " us" << std::endl;
    of_latency << "Query fille: " << query_filepath << " us" << std::endl;
    of_latency << "#Keys: " << num_keys << ", #Queries: " << num_queries;
    of_latency << ", #MIND mem regions: " << num_memory_regions << std::endl;

    for (size_t i = 0; i < len; i ++)
        of_latency << latency_array[i] << " us" << std::endl;
    of_latency.close();
    std::cout << "Done" << std::endl;
}

int main()
{
    std::ifstream data_stream(data_filepath);
    std::ifstream query_stream(query_filepath);
    RpcParse parser;

    BPlusTree tree("./");

#ifndef KEY_COUNT_ONLY
#ifdef LOAD_FROM_FILE
    void *root_ptr = load_mem_layout(mem_layout_name.c_str());
    tree.set_root(root_ptr);
#else
    for (int i = 0; i < num_memory_regions; ++i)
    {
        mind_malloc_add_mem();
    }
    // std::vector<key_type> key_list;
    // std::vector<key_type> value_list;
    // read_time_series_keys(data_stream, key_list, value_list, num_keys);
    parser.read_all_keys(data_stream, num_keys, true);
    for (size_t i = 0; i < num_keys; ++i)
    {
        std::string val_string = gen_random(7);
        tree.insert(parser.all_keys[i], const_cast<void*>(static_cast<const void*>(val_string.c_str())), false);    // use sacn length as value
        if (i % (num_keys / 100) == 0)
            std::cout << i << "/" << num_keys << std::endl;
    }
#endif
    unsigned int *latency_array = static_cast<unsigned int *>(malloc(num_queries * sizeof(unsigned int)));

    parser.read_time_series_query(query_stream, num_queries);
    std::cout << "== Start new count == " << std::endl;
    std::cout << "Wait for 30 sec to reset the fault" << std::endl;
#ifndef STORE_TO_FILE
    sleep(30);
    auto start_tot = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < num_queries; ++i)
    {
        long result;
        auto start = std::chrono::high_resolution_clock::now(); 
        auto total_chase = tree.aggregate(parser.all_query[i].key, parser.all_query[i].scan_len, result, 0, (i % (num_queries / 100) == 0)); 
        auto stop = std::chrono::high_resolution_clock::now(); 
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        unsigned int latency = duration.count();
        if (i % (num_queries / 100) == 0)
            std::cout << latency << " us | chase: " << total_chase << std::endl;
        latency_array[i] = latency;
    }
    auto stop_tot = std::chrono::high_resolution_clock::now();
    auto duration_tot = std::chrono::duration_cast<std::chrono::microseconds>(stop_tot - start_tot);
    dump_latency(latency_array, num_queries, duration_tot.count());
    free(latency_array);
#else
    // tree.print_layout("./layout.txt");
    printf("Root addr: 0x%lx\n", (unsigned long)tree.get_root());
    dump_mem_layout(mem_layout_name.c_str(), tree.get_root());
#endif
    mind_free_all();
#else //KEY_COUNT_ONLY
    for (unsigned long i = 0; i < num_keys; ++i)
    {
        auto pr = next_key(data_stream);
        if (pr.first == false)
            break;

	if (i % 1000000 == 0)
            std::cout << "Keys: " << i << std::endl;
    }
#endif //KEY_COUNT_ONLY
    return 0;
}
