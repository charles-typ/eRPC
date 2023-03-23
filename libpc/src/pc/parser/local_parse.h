#ifndef PC_API_UTILS_PULSE_PARSE_H
#define PC_API_UTILS_PULSE_PARSE_H
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <random>
#include <climits>
#include <cassert>
#include "pc/datastructure/hashtable.h"

using namespace HashTable;

std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

struct query
{
    std::string query_type;
    key_type key;
    uint64_t scan_len;
    int agg_type;
};

static std::vector<key_type> all_keys;
static std::vector<key_type> all_values;

static std::vector<query> all_query;

static std::vector<void *> all_buckets = std::vector<void *>(10000000);

long long getRand(long long l, long long r)
{
    std::uniform_int_distribution<long long> uid(l, r);
    return uid(rng);
}

// returns {true, key} if exists, otherwise {false, 0}
void read_all_keys(std::ifstream &input_file, double total_keys)
{
    std::string line;
    double count = 0;
    while (getline(input_file, line))
    {
        std::stringstream s(line);
        std::string word;
        s >> word;
        if (word != "INSERT")
            continue;
        s >> word;
        assert(word.substr(0, 4) == "user");
        word = word.substr(4);
        key_type key = stoull(word);
        all_keys.push_back(key);
        count += 1;
    }
}

// void read_all_keys_mmap(std::string filename, double total_keys) {
//
//     int fd;
//     if((fd = open(filename, O_RDWR)) == -1) {
//         std::cout << "Open file error " << filename << std::endl;
//         return;
//     }
//
//     struct stat filestat;
//
//     if(fstat(fd, &filestat) !=0) {
//        perror("stat failed");
//        exit(1);
//     }
//
//     char* key_log = (char*)mmap(NULL, filestat.st_size, PROT_READ, MAP_SHARED, fd, 0)
//     std::string line;
//     double count = 0;
//     while(getline(input_file, line)) {
//         std::stringstream s(line);
//         std::string word;
//         s >> word;
//         if(word != "INSERT") continue;
//         s >> word;
//         assert(word.substr(0, 4) == "user");
//         word = word.substr(4);
//         key_type key = stoull(word);
//         all_keys.push_back(key);
//         count += 1;
//     }
// }

void read_all_query(std::ifstream &input_file, double total_query)
{
    std::string line;
    double count = 0;
    while (getline(input_file, line))
    {
        std::stringstream s(line);
        std::string word;
        s >> word;
        if (word != "READ" && word != "SCAN" && word != "AGGREGATE")
            continue;
        query q;
        q.query_type = word;
        s >> word;
        assert(word.substr(0, 4) == "user");
        word = word.substr(4);
        q.key = stoull(word);
        if (q.query_type == "SCAN")
        {
            s >> word;
            q.scan_len = stoll(word);
        }
        else if (q.query_type == "AGGREGATE")
        {
            s >> word;
            q.scan_len = stoll(word);
            s >> word;
            q.agg_type = stoll(word);
        }
        all_query.push_back(q);
        count += 1;
    }
}

struct hashtable_entry
{
    uint64_t key;
    char value[8];
    uint64_t next_ptr;
};

void load_hashtable(std::ifstream &input_file, void *ptr, void *base_ptr)
{
    std::string line;
    // double count = 0;
    while (getline(input_file, line))
    {
        std::stringstream s(line);
        std::string word;
        hashtable_entry entry;
        s >> word; // unused key:
        s >> word; // real key
        entry.key = stoull(word);
        s >> word; // unused value:
        s >> word; // real value;
        strcpy(entry.value, word.c_str());
        s >> word; // unused hash
        s >> word; // real hash
        s >> word; // unused address:
        s >> word; // address
        void *addr = (void *)std::stoull(word, nullptr, 16);
        s >> word; // unused next:
        s >> word; // next
        entry.next_ptr = std::stoull(word, nullptr, 16);
        memcpy((void *)((uint64_t)ptr + ((uint64_t)addr - (uint64_t)base_ptr)), &entry, sizeof(struct hashtable_entry));
    }
}

void load_route(std::ifstream &input_file, void *base)
{
    std::string line;
    double count = 0;
    // std::cout << "Into this function" << std::endl;
    while (getline(input_file, line))
    {
        // std::cout << "line" << std::endl;
        std::stringstream s(line);
        std::string word;
        s >> word; // unused bucket
        s >> word; // bucket number
        uint64_t bucket = stoull(word);
        s >> word; // unused address
        s >> word; // bucket_address
        // std::cout << "Inserting this bucket: " << bucket << " for word: " << word << std::endl;
        all_buckets[bucket] = (void *)std::stoull(word, nullptr, 16);
    }
}

void read_time_series_keys(std::ifstream &input_file, double total_keys)
{
    std::string line;
    double count = 0;
    while (getline(input_file, line))
    {
        std::stringstream s(line);
        std::string word;
        getline(s, word, ',');
        uint64_t key = (uint64_t)std::stoull(word);
        getline(s, word, ',');
        uint64_t value = (uint64_t)std::stoull(word);
        all_keys.push_back(key);
        all_values.push_back(value);
        count += 1;
    }
}

void read_time_series_query(std::ifstream &input_file, double total_query)
{
    std::string line;
    double count = 0;
    while (getline(input_file, line))
    {
        std::stringstream s(line);
        std::string word;
        getline(s, word, ' ');
        query q;
        q.key = (uint64_t)std::stoull(word);
        getline(s, word, ' ');
        q.scan_len = (uint64_t)std::stoull(word);
        all_query.push_back(q);
        count += 1;
    }
}

#endif // PC_API_UTILS_PULSE_PARSE_H
