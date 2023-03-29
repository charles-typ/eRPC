#include <chrono>
#include <iomanip>
#include <ctime>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include "hashtable_mind.h"
#include "pc/parser/rpc_parse.h"
#include "mind_malloc.h"

using namespace pc::parser;

std::string gen_random(const size_t len)
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (size_t i = 0; i < len; ++i)
    {
        tmp_s += alphanum[static_cast<size_t>(rand()) % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}

// std::string data_filepath = "../../data/load.dat";
// std::string query_filepath = "../../data/transactionsE.dat";
const std::string latency_result = "/home/sslee/pulse_bench/pc-api/bench/mind/log/hashtb_lat.txt";

#ifdef MEM_DUAL_BLADES
#ifdef VERBOSE_FLAG
const int num_keys = 10000;
const std::string mem_layout_name = "/home/sslee/pulse_bench/pc-api/data/hashtb_layout_test.bin";    // share the same layout
const int num_memory_regions = 2; // x 4 GB per region
#else
const int num_keys = 1000000000;
const std::string mem_layout_name = "/home/sslee/pulse_bench/pc-api/data/hashtb_layout.bin";    // share the same layout
const int num_memory_regions = 10; // x 4 GB per region
#endif

const std::string data_filepath = "/home/sslee/pulse_bench/pc-api/data/ycsbc-key-2-blade";
const std::string query_filepath = "/home/sslee/pulse_bench/pc-api/data/ycsbc-query-2-blade-max";
#else
#ifdef VERBOSE_FLAG
const int num_keys = 10000;
const std::string mem_layout_name = "/home/sslee/pulse_bench/pc-api/data/hashtb_layout_single_test.bin";
const int num_memory_regions = 1; // x 4 GB per region
#else
const int num_keys = 500000000;
const std::string mem_layout_name = "/home/sslee/pulse_bench/pc-api/data/hashtb_layout_single.bin";    // share the same layout
const int num_memory_regions = 6; // x 4 GB per region
#endif

const std::string data_filepath = "/home/sslee/pulse_bench/pc-api/data/ycsbc-key-1-blade";
const std::string query_filepath = "/home/sslee/pulse_bench/pc-api/data/ycsbc-query-1-blade-max";
#endif
#ifdef VERBOSE_FLAG
const int num_queries = 100;
#else
const int num_queries = 100000;   // 10000000;
#endif

static unsigned long long debug_total_chase_cnt = 0;

void dump_latency(unsigned int *latency_array, size_t len, unsigned int total_duration)
{
    std::cout << "=print latency result=...";
    std::ostringstream result_str;
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    result_str << latency_result << "_";
    result_str << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S");
    std::ofstream of_latency(result_str.str());
    of_latency << "Total latency: " << total_duration << " us | Chase_cnt: " << debug_total_chase_cnt << std::endl;
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

#ifndef KEY_COUNT_ONLY
#ifdef LOAD_FROM_FILE
    void *root_ptr = load_mem_layout(mem_layout_name.c_str());
    HashTableMIND::HashTable hashtb;
    hashtb.set_root(root_ptr);
#else
    for (int i = 0; i < num_memory_regions; ++i)
    {
        mind_malloc_add_mem();
    }
    HashTableMIND::HashTable hashtb;
    parser.read_all_keys(data_stream, num_keys);
    for (size_t i = 0; i < num_keys; ++i)
    {
        std::string val_string = gen_random(7);
        hashtb.insert(parser.all_keys[i], val_string.c_str(), true);
        if (i % (num_keys / 100) == 0)
            std::cout << i << "/" << num_keys << std::endl;
    }
#endif
    unsigned int *latency_array = static_cast<unsigned int *>(malloc(num_queries * sizeof(unsigned int)));

    //std::vector<query> query_list;
    //read_all_query_short(query_stream, query_list, num_queries);
    parser.read_all_query(query_stream, num_queries);
    // for (int i = 0; i < num_queries; ++i)
    // {
    //     auto pr = next_query(query_stream);
    //     if (pr.first == false)
    //         break;
    //     query_list.push_back(pr.second);
    // }

    std::cout << "== After loading queries == " << std::endl;
    std::cout << "Wait for 30 sec to reset the fault" << std::endl;
    sleep(30);
    auto start_tot = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < num_queries; ++i)
    {
        uint64_t cnt = 0;
        //auto q = query_list[i];
        //assert(q.query_type == "READ");
        auto start = std::chrono::high_resolution_clock::now();
        auto found = hashtb.find(parser.all_query[i].key, false, reinterpret_cast<int*>(&cnt));
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        unsigned int latency = duration.count();
        if (i % (num_queries / 100) == 0)
        {
            std::cout << latency << " us | " << (found ? found->key : 0) << " <-> " << parser.all_query[i].key;
            std::cout << " | cnt: " << cnt << std::endl;
            if (found && found->key != parser.all_query[i].key)
            {
                std::cout << "ERR::" << (found ? found->key : 0) << " <-> " << parser.all_query[i].key;
                std::cout << "(lat: " << latency << " us)" << std::endl;
            }
        }
        latency_array[i] = latency;
        debug_total_chase_cnt += cnt;

        // std::cout << "Scan result" << std::endl;
        // for(auto &r: result) {
        //     std::cout << r.key  << " " << r.value << std::endl;
        // }
        // std::cout << std::endl;
    }
    hashtb.print_bucket_length();

    auto stop_tot = std::chrono::high_resolution_clock::now();
    auto duration_tot = std::chrono::duration_cast<std::chrono::microseconds>(stop_tot - start_tot);
    dump_latency(latency_array, num_queries, duration_tot.count());
    free(latency_array);
    // tree.print_layout("./layout.txt");
#ifdef STORE_TO_FILE
    printf("Root addr: 0x%lx\n", (unsigned long)hashtb.get_root());
    dump_mem_layout(mem_layout_name.c_str(), hashtb.get_root());
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