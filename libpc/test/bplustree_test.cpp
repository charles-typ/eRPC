#include "pc/datastructure/bplustree.h"
#include "pc/parser/rpc_parse.h"
#include "pc/utils/time_util.h"
#include "pc/utils/rand_util.h"
#include "test_util.h"
#include <string>

using namespace std;
using namespace pc::datastructure;
using namespace pc::parser;
using namespace pc::utils;

const uint64_t num_keys = 10000;
struct btree_element
{
    char result[8];
} __attribute__((aligned(8)));

int main()
{
    BPlusTree tree("./");

    std::ifstream data_stream(std::string("/var/data/ycsbc-key-test"));
    RpcParse parser;
    parser.read_all_keys(data_stream, num_keys);
    // Insert random key and values
    vector<string> ctr(num_keys);

    for (int i = 0; i < num_keys; ++i)
    {
        ctr[i] = gen_random(7);
    }

    for (int i = 0; i < num_keys; ++i)
    {
        tree.insert(parser.all_keys[i], (void *)ctr[i].c_str());
        // std::cout << "Insert complete" << endl;
    }

    std::cout << "load finishes!" << std::endl;

    // Find
    //std::cout << "Read result" << endl;
    //void *read_result_ptr;
    //for (int i = 0; i < num_keys; ++i)
    //{
    //    std::cout << " Reading result for key: " << i << std::endl;
    //    int ret = tree.read(i, read_result_ptr);
    //    if (read_result_ptr != NULL)
    //    {
    //        std::cout << "Result is: " << (char *)read_result_ptr << std::endl;
    //    }
    //    else
    //    {
    //        std::cout << "Key not found!" << std::endl;
    //    }
    //}

    // Scan

    uint64_t num_queries = 10000;
    std::ifstream query_stream(std::string("/var/data/ycsbc-query-test"));
    parser.read_all_query(query_stream, num_queries);

    for (int i = 0; i < 2; i++) {
        std::vector<struct bplustree_entry> results;
        tree.scan(parser.all_query[i].key, 10, results);
        cout << "Scan result" << endl;
        for (auto &r : results)
        {
            cout << r.key << " " << r.value << endl;
        }
    }

    return 0;
}
