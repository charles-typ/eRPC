#ifndef PC_API_HASHTABLE_H
#define PC_API_HASHTABLE_H
#include <vector>
#include <utility>
#include <cstring>
#include <iostream>
#include <fstream>
#include <chrono>

#include "list.h"

namespace pc
{
  namespace datastructure
  {
    namespace hashtable
    {

      class HashTable;

      typedef uint64_t key_type;

#define DEFAULT_VALUE_LEN 8
#define PADDING_LENGTH 256 - 8 - 8
      class Node
      {
        friend class HashTable;
        friend class Iterator;

        key_type key;
        char value[DEFAULT_VALUE_LEN];
        char padding[PADDING_LENGTH];

      public:
        Node *next;

        Node(std::pair<key_type, const char *> pr) : key(pr.first)
        {
          next = NULL;
          memcpy(value, pr.second, strlen(pr.second) + 1);
        }

      public:
        bool operator==(Node other)
        {
          return (key == other.key && !strcmp(value, other.value));
        }
      };

      class Iterator
      {
        friend class HashTable;

      protected:
        std::vector<list::List<Node *>> *lists;
        unsigned long index;
        list::List<Node *>::iterator itr;

        Iterator(std::vector<list::List<Node *>> *lists_, unsigned long index_, list::List<Node *>::iterator itr_)
            : lists(lists_), index(index_), itr(itr_) {}

      public:
        Iterator &operator++()
        {
          ++itr;
          if (itr == (*lists)[index].end())
          {
            do
            {
              index++;
            } while (index < (*lists).size() && (*lists)[index].begin() == (*lists)[index].end());

            if (index == (*lists).size())
            {
              itr = NULL;
            }
            else
              itr = (*lists)[index].begin();
          }

          return *this;
        }

        bool operator==(Iterator other) { return lists == other.lists && index == other.index && itr == other.itr; }
        bool operator!=(Iterator other) { return !(*this == other); }

        std::pair<key_type, char *> operator*()
        {
          return std::make_pair((*itr)->key, (*itr)->value);
        }
        void* get_value() {
          return &((*itr)->value);
        }
      };

      class HashTableList
      {
      public:
        Node *cur;
        Node *start;
        Node *read_cur;
        HashTableList()
        {
          cur = NULL;
          start = NULL;
        }
        void push_back(Node *n)
        {
          if (start == NULL)
          {
            start = n;
            cur = n;
          }
          else
          {
            cur->next = n;
            cur = n;
          }
        }
        Node *read_from_start()
        {
          read_cur = start;
          //FIXME not used
          return NULL;
        }
        Node *read()
        {
          Node *ret = read_cur;
          if (read_cur != NULL)
            read_cur = read_cur->next;
          return ret;
        }
      };

      class HashTable
      {
        uint64_t size;
        uint64_t max_load_factor;
        uint64_t rehash_multiple;
        uint64_t capacity;
        std::vector<list::List<Node *>> lists;
        std::vector<HashTableList> real_lists;

      public:
        HashTable()
        {
          size = 0;
          rehash_multiple = 10;
          max_load_factor = 200;
          capacity = 500000;
          //capacity = 1000;
          lists = std::vector<list::List<Node *>>(capacity);
          real_lists = std::vector<HashTableList>(capacity);
        }

        void rehash()
        {
          capacity *= rehash_multiple;
          std::vector<list::List<Node *>> new_lists(capacity);
          std::vector<HashTableList> new_real_lists(capacity);
          for (auto &list : lists)
          {
            for (Node *node : list)
            {
              // std::cout << hash(node->key) << ' ' << node->key << ' ' << node->value << std::endl;
              new_lists[hash(node->key)].push_back(node);
              new_real_lists[hash(node->key)].push_back(node);
            }
          }
          // for(auto& real_list: real_lists) {
          //     real_list.read_from_start();
          //     Node* ptr;
          //     while(ptr=real_list.read()) {
          //         if(ptr == NULL)
          //             break;
          //         // std::cout << hash(node->key) << ' ' << node->key << ' ' << node->value << std::endl;
          //         new_real_lists[hash(ptr->key)].push_back(ptr);
          //     }
          // }
          swap(lists, new_lists);
          swap(real_lists, new_real_lists);
        }

        key_type hash(key_type key)
        {
          return key % capacity; // Assume all keys are unsigned
        }

        uint64_t get_capacity()
        {
          return capacity;
        }

        Iterator begin()
        {
          if (size == 0)
            return end();

          for (uint64_t i = 0; i < capacity; ++i)
          {
            if (lists[i].begin() != lists[i].end())
            {
              return Iterator(&this->lists, i, lists[i].begin());
            }
          }
          return end();
        }

        Iterator end()
        {
          return Iterator(&this->lists, capacity, NULL);
        }

        void insert(std::pair<key_type, const char *> pr)
        {
          //auto start_time_0 = std::chrono::high_resolution_clock::now();
          //auto itr = find(pr.first, false);
          //auto end_time_0 = std::chrono::high_resolution_clock::now();
          //std::cout << "Takes " << static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end_time_0-start_time_0).count()) << " nanoseconds find location" << std::endl;
          //if (itr != end())
          //{
          //  memcpy((*itr.itr)->value, pr.second, strlen(pr.second));
          //  //return itr;
          //  return;
          //}

          if (capacity * max_load_factor < size)
          {
	    std::cout << "Rehashing: " << capacity * max_load_factor << " " << size << std::endl;
            rehash();
          }

          Node *node = new Node(pr);
          lists[hash(node->key)].push_back(node);
          real_lists[hash(node->key)].push_back(node);
          // std::cout << "Inserting key: " << node->key << " into bucket: " << hash(node->key) << std::endl;
          size++;
          //auto end_time_1 = std::chrono::high_resolution_clock::now();
          //std::cout << "Takes " << static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end_time_1-end_time_0).count()) << " nanoseconds to insert" << std::endl;

          //return Iterator(&this->lists, hash(node->key), this->lists[hash(node->key)].find(node));
          return;
        }

        Iterator find(key_type key, bool verbose = false)
        {
          (void)verbose;
          //std::cout << "Finding key: " << key << " hash bucket is: " << hash(key) << std::endl;
          int count = 0;
          auto start_time_0 = std::chrono::high_resolution_clock::now();
          for (auto itr = lists[hash(key)].begin(); itr != lists[hash(key)].end(); ++itr)
          {
            Node *node = *itr;
            if (node->key == key)
            {
              //if (verbose)
              //{
                std::cout << "Found key at length: "  << count << std::endl;
              //}
              return Iterator(&this->lists, hash(key), itr);
            }
            count++;
          }

          //if (verbose)
          //{
          auto end_time_0 = std::chrono::high_resolution_clock::now();
          std::cout << "Takes " << static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end_time_0-start_time_0).count()) << " nanoseconds to traverse " << count << " nodes" << std::endl;
          //}
          return end();
        }

        void print_layout()
        {
          std::ofstream layout("/var/data/hashtable_layout");
          for (auto &list : lists)
          {
            for (Node *n : list)
            {
              layout << "key: " << n->key << " value: " << n->value << " hash: " << hash(n->key) << " address: "
                     << static_cast<void *>(&(n->key)) << std::endl;
              // layout  << "key: " << (void *)&(n->key) << " value: " << (void *)&(n->value) << std::endl;
            }
          }
          layout.close();
        }

        void print_bucket_layout()
        {
          std::ofstream layout("/var/data/hashtable_bucket_layout");
          for (auto &list : lists)
          {
            if (!list.empty())
            {
              // layout  << "key: " << (*list.begin())->key << " value: " << (*list.begin())->value << std::endl;
              layout << "bucket: " << hash((*list.begin())->key) << " address: " << static_cast<void *>(&((*list.begin())->key))
                     << std::endl;
            }
          }
          layout.close();
        }
        void print_layout_test()
        {
          std::ofstream layout("/var/data/hashtable_layout_test");
          for (auto &list : lists)
          {
            for (Node *n : list)
            {
              // layout  << "key: " << n->key << " value: " << n->value << " hash: " << hash(n->key) << " address: " << (void *)&(n->key) << " next: " << (void*)(*(uint64_t*)((uint64_t)(&(n->key)) + 24)) << std::endl;
              layout << "key: " << n->key << " value: " << n->value << " hash: " << hash(n->key) << " address: "
                     << static_cast<void *>(&(n->key)) << " next: " << static_cast<void *>((n->next)) << std::endl;
              // layout  << "key: " << (void *)&(n->key) << " value: " << (void *)&(n->value) << std::endl;
            }
          }
          layout.close();
        }

        void print_bucket_layout_test()
        {
          std::ofstream layout("/var/data/hashtable_bucket_layout_test");
          for (auto &list : lists)
          {
            if (!list.empty())
            {
              // layout  << "key: " << (*list.begin())->key << " value: " << (*list.begin())->value << std::endl;
              layout << "bucket: " << hash((*list.begin())->key) << " address: " << static_cast<void *>(&((*list.begin())->key))
                     << std::endl;
            }
          }
          layout.close();
        }
        void print_statistics()
        {
          double total_nodes = 0;
          double total_buckets = 0;
          for (auto &list : lists)
          {
            total_buckets += 1;
            for (Node *node : list)
            {
              (void)(node);
              total_nodes += 1;
            }
          }
          std::cout << "Hash table with " << total_buckets << " buckets and " << total_nodes << " nodes, with a total size of " << total_nodes * 256 << " averaging " << total_nodes / total_buckets << " nodes per bucket" << std::endl;
        }
      };

    }
  }

}

#endif // PC_API_HASHTABLE_H
