#ifndef PC_API_HASHTABLE_MIND_H
#define PC_API_HASHTABLE_MIND_H
#include "pc/datastructure/list.h"
#include <vector>
#include <utility>
#include <cstring>
#include <iostream>
#include <fstream>
#include "mind_malloc.h"

namespace HashTableMIND {

class HashTable;

typedef uint64_t key_type;

#define DEFAULT_VALUE_LEN 8
class Node {
    friend class HashTable;
    friend class Iterator;

public: // TODO: proper encapsulation
    key_type key;
    char value[DEFAULT_VALUE_LEN];
    Node *next;

    Node(std::pair<key_type, const char*> pr) {
        init(pr.first, pr.second);
    }

    void init(key_type _key, const char* _value) 
    {
        key = _key;
        memcpy(value, _value, std::min((size_t)DEFAULT_VALUE_LEN, strlen(_value) + 1));
        next = NULL;
    }

    bool operator==(Node other) {
        return (key == other.key && !strcmp(value, other.value));
    }
};

// class Iterator {
//     friend class HashTable;

//     protected:
//     std::vector<List::List<Node *>>* lists;
//     int index;
//     List::List<Node *>::iterator itr;

//     Iterator(std::vector<List::List<Node *>>* lists_, int index_, List::List<Node *>::iterator itr_): lists(lists_), index(index_), itr(itr_) {}

//     public:
//     Iterator& operator++() {
//         ++itr;
//         if(itr == (*lists)[index].end()) {
//             do {
//                 index++;
//             } while(index < (*lists).size() && (*lists)[index].begin() == (*lists)[index].end());

//             if(index == (*lists).size()) {
//                 itr = NULL;
//             } else itr = (*lists)[index].begin();
//         }

//         return *this;
//     }

//     bool operator==(Iterator other) {return lists == other.lists && index == other.index && itr == other.itr; }
//     bool operator!=(Iterator other) {return !(*this == other); }

//     std::pair<key_type, char*> operator*() {
//         return std::make_pair((*itr)->key, (*itr)->value);
//     }
// };

class HashTable {
    int size;
    int max_load_factor;
    int rehash_multiple;
    uint64_t capacity;
    // std::vector<List::List<Node *>> lists;
    Node **bucket_ptr_list = NULL;

    public:
    HashTable() {
        size = 0;
        rehash_multiple = 2;
        max_load_factor = 2;
#ifdef MEM_DUAL_BLADES
        capacity = 5000000;
#else
        capacity = 2500000;
#endif
        // capacity = 1000;

        // lists = std::vector<List::List<Node *>>(capacity);
        bucket_ptr_list = (Node **)mind_malloc(sizeof(Node *) * capacity);
	    memset(bucket_ptr_list, sizeof(Node *) * capacity, 0);
        std::cout << "Capacity (#buckets): " << (unsigned long)capacity << std::endl;
	    std::cout << "Allocated buckets: " << (unsigned long)bucket_ptr_list << std::endl;
    }

    // void rehash() {
    //     capacity *= rehash_multiple;
    //     std::vector<List::List<Node *>> new_lists(capacity);
    //     for(auto& list: lists) {
    //         for(Node *node: list) {
    //             // std::cout << hash(node->key) << ' ' << node->key << ' ' << node->value << std::endl;
    //             new_lists[hash(node->key)].push_back(node);
    //         }
    //     }
    //     swap(lists, new_lists);
    // }

    key_type hash(key_type key) {
        return key % capacity; // Assume all keys are unsigned
    }

    // Iterator begin() {
    //     if(size == 0) return end();

    //     for(int i = 0; i < capacity; ++i) {
    //         if(lists[i].begin() != lists[i].end()) {
    //             return Iterator(&this->lists, i, lists[i].begin());
    //         }
    //     }
    //     return end();
    // }

    // Iterator end() {
    //     return Iterator(&this->lists, capacity, NULL);
    // }

    // Iterator insert(std::pair<key_type, const char*> pr) {
    //     auto itr = find(pr.first, false);
    //     if(itr != end()) {
    //         memcpy((*itr.itr)->value, pr.second, strlen(pr.second));
    //         return itr;
    //     }

    //     if(capacity * max_load_factor == size) {
    //         rehash();
    //     }

    //     Node *node = new Node(pr);
    //     lists[hash(node->key)].push_back(node);
    //     //std::cout << "Inserting key: " << node->key << " into bucket: " << hash(node->key) << std::endl;
    //     size++;

    //     return Iterator(&this->lists, hash(node->key), this->lists[hash(node->key)].find(node));
    // }

    // Iterator find(key_type key, bool verbose=false) {
    //     //std::cout << "Finding key: " << key << " hash bucket is: " << hash(key) << std::endl;
    //     for(auto itr = lists[hash(key)].begin(); itr != lists[hash(key)].end(); ++itr) {
    //         Node *node = *itr;
    //         if(node->key == key) {
    //             if(verbose) {
    //                 std::cout << "Found" << std::endl;
    //             }
    //             return Iterator(&this->lists, hash(key), itr);
    //         }
    //     }

    //     if(verbose) {
    //         std::cout << "Not Found" << std::endl;
    //     }
    //     return end();
    // }

    void *get_root(void)
    {
        return (void *)bucket_ptr_list; // just an address
    }

    int set_root(void *root_ptr) {
        bucket_ptr_list = (Node **)root_ptr;
        return 1;
    }

    // check the bucket and insert
    int insert_in_bucket(Node *new_node)
    {
        // assert(new_node);
        key_type bucket_id = hash(new_node->key);
        Node **bucket_head = &bucket_ptr_list[bucket_id];
        if (!(*bucket_head))   // first node of the bucket
        {
            (*bucket_head) = new_node;
            return 0;
        }
        // add it in front of the link
        new_node->next = (*bucket_head);
        (*bucket_head) = new_node;
        //std::cout << "Inserting key: " << node->key << " into bucket: " << hash(node->key) << std::endl;
        size++;
        return 0;
    }

    int insert(key_type key, const char* value, bool allow_duplicated = false) {
        if (!allow_duplicated)
        {
            Node *node_ptr = find(key, false);
            // update
            if (node_ptr) {
                memcpy(node_ptr->value, value, std::min((size_t)DEFAULT_VALUE_LEN, strlen(value) + 1));
                return 1;
            }
        }
     
        Node *new_node = (Node *)mind_malloc(sizeof(Node));
        new_node->init(key, value);
        insert_in_bucket(new_node);

        return 0;
    }

// REF
/*
struct node* cur_ptr = (struct node*)addr;
while(cur_ptr->next_ptr != NULL && cur_ptr >= memory_1_min_range && cur_ptr <= memory_1_max_range) {
    if(cur_ptr->key == key) {
        return std::make_pair(cur_ptr->value, 0);
    } else {
        cur_ptr = (struct node*)cur_ptr->next_ptr;
    }
}
if(cur_ptr < memory_1_min_range || cur_ptr > memory_1_max_range ) { // this should go to another blade
    return std::make_pair((uint64_t)cur_ptr, 1);
} else if (cur_ptr->next_ptr == NULL) { // this should never happen
    std::cout << "Current pointer is empty " << std::endl;
}
*/

    Node *find(key_type key, bool verbose=false, int *cnt = NULL) {
        Node *cur_ptr = bucket_ptr_list[hash(key)];
        int chase_cnt = 0;
#ifdef VERBOSE_FLAG_VERBOSE
            std::cout << "FIND (" << key << "): " << (unsigned long)cur_ptr;
#endif
        while (cur_ptr)
        {
            if (cur_ptr->key == key)
                break;
            cur_ptr = cur_ptr->next;
            chase_cnt ++;
#ifdef VERBOSE_FLAG_VERBOSE
            std::cout << "->" << (unsigned long)cur_ptr;
#endif
        }
#ifdef VERBOSE_FLAG_VERBOSE
        std::cout << std::endl;
#endif
        if (cnt)
            *cnt = chase_cnt;
        if (cur_ptr)
        {
            if(verbose) {
                std::cout << "Found" << std::endl;
            }
            return cur_ptr;
        }

        if(verbose) {
            std::cout << "Not Found" << std::endl;
        }
        return NULL;
    }

    void print_bucket_length(void)
    {
        for (int i = 0; i < capacity; i += (capacity / 100))
        {
            unsigned int size = 0;
            Node *cur_ptr = bucket_ptr_list[i];
            while (cur_ptr)
            {
                cur_ptr = cur_ptr->next;
                size ++;
            }
            std::cout << "Bucket [" << i << "] " << size << std::endl;
        }
    }

    // void print_layout() {
    //     std::ofstream layout("/var/data/hashtable_layout");
    //     for(auto& list: lists) {
    //         for(Node *n: list) {
    //             layout  << "key: " << n->key << " value: " << n->value << " hash: " << hash(n->key) << " address: " << (void *)&(n->key) << std::endl;
    //             //layout  << "key: " << (void *)&(n->key) << " value: " << (void *)&(n->value) << std::endl;
    //         }
    //     }
    //     layout.close();
    // }

    // void print_bucket_layout() {
    //     std::ofstream layout("/var/data/hashtable_bucket_layout");
    //     for(auto& list: lists) {
    //         if(!list.empty()) {
    //             //layout  << "key: " << (*list.begin())->key << " value: " << (*list.begin())->value << std::endl;
    //             layout  << "bucket: " << hash((*list.begin())->key) << " address: " << (void *)&((*list.begin())->key) << std::endl;
    //         }
    //     }
    //     layout.close();
    // }
};
}

#endif // PC_API_HASHTABLE_MIND_H