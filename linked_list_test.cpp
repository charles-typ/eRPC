#include <chrono>
#include <iostream>
#include <vector>
#include <thread>
#define num_lists 500000
#define list_length 200
#define num_threads 24

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

void func(int thread_id) {
  for (int j = 0; j < num_lists; j++) {
    uint64_t total_result = 0;
    uint64_t total_time = 0;
    struct node* search = start[j];
    int repeat = 10;
    for (int i = 0; i < repeat; i++) {
      auto start_time = std::chrono::high_resolution_clock::now();
      while (search->next != NULL) {
        total_result += search->value;
        search = search->next;
      }
      auto end_time = std::chrono::high_resolution_clock::now();
      total_time += static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(end_time -
                                                               start_time)
              .count());
      //cout << "result is: " << total_result << endl;
    }
    result[thread_id][j] = total_time / repeat;
  }
}

int main() {
	start.reserve(num_lists);
	cur.reserve(num_lists);
	prev_node.reserve(num_lists);
  int j = 0;
  for (; j < num_lists; j++) {
    start[j] = new node;
    prev_node[j] = start[j];
    for (int i = 0; i < list_length; i++) {
      cur[j] = new node;
      prev_node[j]->next = cur[j];
      prev_node[j] = cur[j];
    }
  }
  vector<thread> thread_vec;
  result.reserve(num_threads);
  for(int i = 0; i < num_threads; i++) {
	result[i].reserve(num_lists);
	std::thread t(func, i);
	thread_vec.push_back(std::move(t));
  }
  for(auto &t: thread_vec) {
	t.join();
  }
  for(int i = 0; i < num_threads; i++) {
  	for (int j = 0; j < num_lists; j++) {
		cout << "Thread[" << i << "] List[" << j << "]: " << result[i][j] << " nanoseconds" << std::endl;
	}
  }

  return 0;
}
