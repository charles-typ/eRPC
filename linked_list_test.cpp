#include <chrono>
#include <iostream>

using namespace std;

struct node {
  uint64_t key;
  uint64_t value;
  struct node* next;
  char padding[4096 - 8 - 8 - 8];
};

int main() {
  int num_lists = 500000;
  int list_length = 100;
  struct node* start[num_lists];
  struct node* cur[num_lists];
  struct node* prev[num_lists];
  for (int j = 0; j < num_lists; j++) {
	cout << j << endl;
    start[j] = new node;
    prev[j] = start[j];
    for (int i = 0; i < 100; i++) {
      cur[j] = new node;
      prev[j]->next = cur[j];
      prev[j] = cur[j];
    }
  }
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
      cout << "result is: " << total_result << endl;
    }
  	cout << "Traverse time for 100 nodes: " << total_time / repeat << endl;
  }

  return 0;
}
