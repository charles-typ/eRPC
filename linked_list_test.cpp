#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cassert>
#include <chrono>
#include <climits>
#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>
#include <streambuf>
#include <string>
#include <thread>
#include <vector>
#define num_lists 500000
#define list_length 200
#define num_threads 1

using namespace std;

struct node {
  uint64_t key;
  uint64_t value;
  struct node* next;
  char padding[256 - 8 - 8 - 8];
};
vector<struct node*> start;
vector<struct node*> cur;
vector<struct node*> prev_node;
vector<vector<uint64_t>> result;
vector<uint64_t> total_result;

struct query {
  std::string query_type;
  uint64_t key;
  uint64_t scan_len;
  int agg_type;
};

vector<uint64_t> all_keys;
vector<uint64_t> all_values;
vector<query> all_query;
vector<void*> all_buckets;

void func(int thread_id) {
  std::cout << "Starting function at thread_id: " << thread_id << std::endl;
  for (int j = 0; j < all_query.size(); j++) {
    // uint64_t total_result = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    uint64_t total_time = 0;
    struct node* search = start[j % num_lists];
    int count = 0;
    while (search->next != NULL && search->key != all_query[j].key) {
      count++;
      total_result[thread_id] += search->value;
      search = search->next;
    }
    // std::cout << "Traverse length = " << count << std::endl;
    auto end_time = std::chrono::high_resolution_clock::now();
    total_time += static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(end_time -
                                                             start_time)
            .count());
    // cout << "result is: " << total_result << endl;
    result[thread_id][j] = total_time;
  }
  std::cout << "Starting function at thread_id: " << thread_id << std::endl;
}

void read_all_query(std::ifstream& input_file, double total_query,
                    bool time_series = false) {
  (void)total_query;
  (void)time_series;
  std::string line;
  double count = 0;
  while (getline(input_file, line)) {
    std::stringstream s(line);
    std::string word;
    s >> word;
    if (word != "READ" && word != "SCAN" && word != "AGGREGATE") continue;
    query q;
    q.query_type = word;
    s >> word;
    if (word.substr(0, 9) == "usertable") {
      s >> word;
    }
    assert(word.substr(0, 4) == "user");
    word = word.substr(4);
    q.key = stoull(word);
    if (q.query_type == "SCAN") {
      s >> word;
      q.scan_len = static_cast<uint64_t>(stoll(word));
    } else if (q.query_type == "AGGREGATE") {
      s >> word;
      q.scan_len = static_cast<uint64_t>(stoll(word));
      s >> word;
      q.agg_type = static_cast<uint64_t>(stoll(word));
    }
    all_query.push_back(q);
    count += 1;
  }
}

void read_all_keys(std::ifstream& input_file, double total_keys,
                   bool time_series = false) {
  all_keys.reserve(total_keys);
  std::string line;
  double count = 0;
  while (getline(input_file, line)) {
    std::stringstream s(line);
    std::string word;
    s >> word;
    if (time_series) {
      std::string delimiter = ",";
      word = word.substr(0, word.find(delimiter));
    } else {
      if (word != "INSERT") continue;
      s >> word;
      assert(word.substr(0, 4) == "user");
      word = word.substr(4);
    }
    uint64_t key = stoull(word);
    all_keys.push_back(key);
    count += 1;
  }
}

int main() {
  std::ifstream data_stream(std::string("/var/data/ycsbc-key-1-blade"));
  read_all_keys(data_stream, 100000000);
  std::cout << "key loaded: " << all_keys.size() << std::endl;
  std::ifstream query_stream(std::string("/var/data/ycsbc-query-1-blade"));
  std::cout << "query loaded: " << all_query.size() << std::endl;
  uint64_t num_queries = 10000000;
  read_all_query(query_stream, num_queries);
  start.reserve(num_lists);
  cur.reserve(num_lists);
  prev_node.reserve(num_lists);
  int j = 0;
  for (; j < num_lists; j++) {
    start[j] = new node;
    prev_node[j] = start[j];
  }
  for (j = 0; j < all_keys.size(); j++) {
    size_t list_id = all_keys[j] % num_lists;
    cur[list_id] = new node;
    cur[list_id]->key = all_keys[j];
    cur[list_id]->value = 1;
    prev_node[list_id]->next = cur[list_id];
    prev_node[list_id] = cur[list_id];
  }
  std::cout << "keys are inserted" << std::endl;
  // for (; j < num_lists; j++) {
  //   start[j] = new node;
  //   prev_node[j] = start[j];
  //   for (int i = 0; i < list_length; i++) {
  //     cur[j] = new node;
  //     cur[j]->key = i;
  //     cur[j]->value = 1;
  //     prev_node[j]->next = cur[j];
  //     prev_node[j] = cur[j];
  //   }
  // }
  vector<thread> thread_vec;
  result.reserve(num_threads);
  total_result.reserve(num_threads);
  for (int i = 0; i < num_threads; i++) {
    result[i].reserve(all_query.size());
    std::thread t(func, i);
    thread_vec.push_back(std::move(t));
  }
  for (auto& t : thread_vec) {
    t.join();
  }
  uint64_t average_latency = 0;
  for (int i = 0; i < num_threads; i++) {
    for (int j = 0; j < all_query.size(); j++) {
      // cout << "Thread[" << i << "] List[" << j << "]: " << result[i][j]
      //      << " nanoseconds" << std::endl;
      average_latency += result[i][j];
    }
    cout << "result is: " << total_result[i] << endl;
  }
  cout << "Average latency is: " << average_latency / num_threads / all_query.size()
       << endl;

  return 0;
}
