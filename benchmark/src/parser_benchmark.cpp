#include "pc/parser/rpc_parse.h"
#include "pc/utils/time_util.h"
#include "pc/utils/rand_util.h"
#include <string>

using namespace std;
using namespace pc::parser;
using namespace pc::utils;

const uint64_t total_num_keys = 500000000;
const uint64_t num_keys = 50000;

int main()
{
  init();
  std::string filename("/var/data/ycsbc-key-1-blade");
  std::ifstream data_stream(filename);
  RpcParse parser;
  auto start_1 = get_time();
  parser.read_all_keys_test(data_stream, total_num_keys, num_keys);
  auto end_1 = get_time();
  std::cout << "Takes " << (end_1 - start_1) / 1000 / 1000 << " ms to read " << num_keys << " keys" << std::endl;
  std::cout << "Reading " << total_num_keys << " keys will take: " << (end_1 - start_1) / 1000 / 1000 * total_num_keys / num_keys / 1000 << " s to read" << std::endl;
  //auto start_2 = get_time();
  //parser.read_all_keys_mmap_test(filename, total_num_keys, num_keys);
  //auto end_2 = get_time();
  //std::cout << "MMAP takes " << end_2 - start_2 << " ns to read " << num_keys << " keys" << std::endl;

  return 0;
}
