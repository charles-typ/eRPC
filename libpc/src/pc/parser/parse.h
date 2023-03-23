#ifndef PC_LIBPC_PARSER_PARSE_H
#define PC_LIBPC_PARSER_PARSE_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <random>
#include <climits>
#include <cassert>

#include "pc/datastructure/hashtable.h"
#include "pc/utils/rand_util.h"

namespace pc
{
    namespace parser
    {
        using pc::datastructure::hashtable::key_type;
        struct query
        {
            std::string query_type;
            key_type key;
            uint64_t scan_len;
            int agg_type;
        };

        class Parse
        {
        public:
            Parse()
            {
                all_buckets = std::vector<void *>(10000000);
            }
            // Stream functions
            // virtual std::pair<bool, unsigned long long> next_key(std::ifstream &input_file);
            // virtual std::pair<bool, query *> next_query(std::ifstream &input_file);
            // Memory load functions
            virtual void read_all_keys(std::ifstream &input_file, double total_keys, bool time_series=false) = 0;
            virtual void read_all_query(std::ifstream &input_file, double total_query, bool time_series=false) = 0;
            // Memory map functions
            virtual void read_all_keys_mmap(std::string filename, double total_keys) = 0;

        //protected:
            std::vector<key_type> all_keys;
            std::vector<uint64_t> all_values;
            std::vector<query> all_query;
            std::vector<void *> all_buckets;
        };

    }

}

#endif // PC_LIBPC_PARSER_PARSE_H
