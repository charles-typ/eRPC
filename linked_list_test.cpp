#include <iostream>


using namespace std;

struct node {
	uint64_t key;
	uint64_t value;
	void* next;
	char padding[256-8-8-8];
};

int main() {
	struct node* start;
	start = new node;
	struct node* cur;
	struct node* prev = start;
	for(i = 0; i < 100; i++) {
		cur = new node;
		prev->next = cur;
		prev = cur;
	}
	total_result;



	return 0;
} 
