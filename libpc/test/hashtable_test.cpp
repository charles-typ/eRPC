#include <iostream>
#include <string>
#include "../src/pc/datastructure/hashtable.h"
#include "pc/parser/rpc_parse.h"
#include "pc/utils/time_util.h"
#include "pc/utils/rand_util.h"
#include "test_util.h"

using namespace pc::parser;
using namespace pc::utils;

int main()
{
    init();
    pc::datastructure::hashtable::HashTable ht;
    RpcParse parser;
    std::ifstream data_stream(std::string("/var/data/ycsbc-key-1-blade"));
    //std::ifstream query_stream(std::string("/var/data/"));
    uint64_t num_keys = 500000000;
    parser.read_all_keys(data_stream, num_keys);
    //read_all_query(query_stream, );

    for(int i = 0; i < num_keys; ++i) {
        std::string value = gen_random(7);
        ht.insert(std::make_pair(parser.all_keys[i], value.c_str()));
    }
    std::cout << "load finishes! sleeping for 180 seconds" << std::endl;
    sleep(180);

    //auto itr = ht.begin();
    //std::cout << (*itr).first << ' ' << (*itr).second << std::endl;
    //++itr;

    //std::cout << (*itr).first << ' ' << (*itr).second << std::endl;
    //for (auto pr : ht)
    //{
    //    std::cout << pr.first << ' ' << pr.second << std::endl;
    //}

    //ht.print_layout();
    return 0;
}
