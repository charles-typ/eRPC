#ifndef PC_LIBPC_PARSER_RPC_PARSE_H
#define PC_LIBPC_PARSER_RPC_PARSE_H

#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <sstream>
#include <istream>
#include <climits>
#include <cassert>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "pc/datastructure/hashtable.h"
#include "pc/utils/rand_util.h"
#include "parse.h"

using namespace pc::datastructure;
using namespace pc::utils;

namespace pc
{
    namespace parser
    {
        class RpcParse : public Parse
        {
        public:
            RpcParse() {}
            struct hashtable_entry
            {
                uint64_t key;
                char value[8];
                uint64_t next_ptr;
            };
            void read_all_keys(std::ifstream &input_file, double total_keys, bool time_series=false);
            void read_all_keys_mmap(std::string filename, double total_keys);
            void read_all_keys_test(std::ifstream &input_file, double total_keysm, uint64_t num_iter);
            void read_all_keys_mmap_test(std::string filename, double total_keys, uint64_t num_iter);
            void read_all_query(std::ifstream &input_file, double total_query, bool time_series=false);

            // Data structure specific

            void load_hashtable(std::ifstream &input_file, void *ptr, void *base_ptr);
            void load_route(std::ifstream &input_file, void *base);
            void read_time_series_keys(std::ifstream &input_file, double total_keys);
            void read_time_series_query(std::ifstream &input_file, double total_query);
        };

    }
}

#endif // PC_LIBPC_PARSER_RPC_PARSE_H