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

static std::vector<query> all_query;

long long getRand(long long l, long long r)
{
    std::uniform_int_distribution<long long> uid(l, r);
    return uid(rng);
}

// returns {true, key} if exists, otherwise {false, 0}
std::pair<bool, key_type> next_key(std::ifstream &input_file)
{
    std::string line;
    while (getline(input_file, line))
    {
        std::stringstream s(line);
        std::string word;
        s >> word;
        if (word != "INSERT")
            continue;
        // s >> word;
        // assert(word == "usertable");
        s >> word;
        assert(word.substr(0, 4) == "user");
        word = word.substr(4);
        key_type key = stoull(word);
        return std::make_pair(true, key);
    }

    return std::make_pair(false, 0);
}

std::pair<bool, query *> next_query(std::ifstream &input_file)
{
    std::string line;
    while (getline(input_file, line))
    {
        std::stringstream s(line);
        std::string word;
        s >> word;
        if (word != "READ" && word != "SCAN" && word != "AGGREGATE")
            continue;
        query *q = new query();
        q->query_type = word;
        // s >> word;
        // assert(word == "usertable");
        s >> word;
        assert(word.substr(0, 4) == "user");
        word = word.substr(4);
        q->key = stoull(word);
        if (q->query_type == "SCAN")
        {
            s >> word;
            q->scan_len = stoll(word);
        }
        else if (q->query_type == "AGGREGATE")
        {
            s >> word;
            q->scan_len = stoll(word);
            s >> word;
            q->agg_type = stoll(word);
        }

        return std::make_pair(true, q);
    }

    return {false, NULL};
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

#endif // PC_API_UTILS_PULSE_PARSE_H