#include <chrono>
#include <iostream>
#include <vector>
#include <thread>
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
vector<vector<uint64_t>> total_result;

void func(int thread_id) {
    int repeat = 20;
	for (int i = 0; i < repeat; i++) {
  for (int j = 0; j < num_lists; j++) {
    //uint64_t total_result = 0;
    uint64_t total_time = 0;
      struct node* search = start[j];
      auto start_time = std::chrono::high_resolution_clock::now();
	  int count = 0;
      while (search->next != NULL && search->key != 100) {
		count++;
        total_result[thread_id][j] += search->value;
        search = search->next;
      }
	  //std::cout << "Traverse length = " << count << std::endl;
      auto end_time = std::chrono::high_resolution_clock::now();
      total_time += static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(end_time -
                                                               start_time)
              .count());
      //cout << "result is: " << total_result << endl;
    result[thread_id][j] = total_time;
  }
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
      cur[j]->key = i;
      cur[j]->value = 1;
      prev_node[j]->next = cur[j];
      prev_node[j] = cur[j];
    }
  }
  vector<thread> thread_vec;
  result.reserve(num_threads);
  total_result.reserve(num_threads);
  for(int i = 0; i < num_threads; i++) {
	result[i].reserve(num_lists);
	total_result[i].reserve(num_lists);
	std::thread t(func, i);
	thread_vec.push_back(std::move(t));
  }
  for(auto &t: thread_vec) {
	t.join();
  }
  for(int i = 0; i < num_threads; i++) {
  	for (int j = 0; j < num_lists; j++) {
		cout << "Thread[" << i << "] List[" << j << "]: " << result[i][j] << " nanoseconds" << std::endl;
		cout << "result is: " << total_result[i][j] << endl;
	}
  }

  return 0;
}
