
#include <iostream>
#include <string>
#include "pc/datastructure/hashtable.h"
#include "pc/parser/rpc_parse.h"
#include "benchmark_util.h"

using namespace pc::parser;
using namespace pc::utils;

struct linked_list_node
{
    uint64_t key;
    uint64_t value;
    struct linked_list_node *next;
    char padding[256 - 24];
};

int main(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "local") == 0)
    {
        //PinToCore(0);
        std::cout << "Size of node: " << sizeof(struct linked_list_node) << std::endl;
        struct linked_list_node start;
        struct linked_list_node *cur = &start;
        for (int i = 0; i < 1000000; i++)
        {
            auto ptr = new (struct linked_list_node);
            ptr->next = NULL;
            cur->next = ptr;
            cur = ptr;
        }
        //auto start_t = get_time();
        cur = &start;
        uint64_t count = 0;
        int count_node = 0;
        while (cur->next != NULL)
        {
            count_node++;
            count += cur->key;
            count += cur->value;
            cur->padding[256 - 24 - 1] = 'a';
            cur = cur->next;
        }
        //auto end_1 = get_time();
        //std::cout << "Takes " << (end_1 - start_t) << " nanoseconds to traverse a " << count_node << " linkedlist" << std::endl;
    }
    else
    {
        pc::datastructure::hashtable::HashTable ht;
        RpcParse parser;
        std::ifstream data_stream(std::string("/var/data/ycsbc-key-1-blade"));
        uint64_t num_keys = 100000000;
        //auto start = get_time();
        parser.read_all_keys(data_stream, num_keys);
        //auto end_1 = get_time();
        //std::cout << "Takes " << (end_1 - start) / 1000 / 1000 / 1000 << " seconds to read keys to memory" << std::endl;
        for (uint64_t i = 0; i < num_keys; ++i)
        {
            std::string value = gen_random(7);
            ht.insert(std::make_pair(parser.all_keys[i], value.c_str()));
        }
        //auto end_2 = get_time();
        //std::cout << "Takes " << (end_2 - end_1) / 1000 / 1000 / 1000 << " seconds to insert to Hashtable " << std::endl;

        ht.print_statistics();

        std::ifstream query_stream(std::string("/var/data/ycsbc-query-1-blade"));
        uint64_t num_queries = 10000000;
        //auto end_3 = get_time();
        parser.read_all_query(query_stream, num_queries);
        //auto end_4 = get_time();
        //std::cout << "Takes " << (end_4 - end_3) / 1000 / 1000 / 1000 << " seconds to load queries " << std::endl;
        //PinToCore(0);
        uint64_t total_time = 0;

        for (uint64_t i = 0; i < num_queries; i++)
        {
            auto start_time = std::chrono::high_resolution_clock::now();
            auto result = ht.find(parser.all_query[i].key);
            auto end_time = std::chrono::high_resolution_clock::now();
            (void)result;
            total_time += static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end_time-start_time).count());
        }
        std::cout << "Takes " <<  total_time / num_queries << " nanoseconds on each query in average " << std::endl;
        //auto end_5 = get_time();
        //std::cout << "Takes " << (end_5 - end_4) << " nanoseconds to execute queries " << std::endl;
    }
    return 0;
}
