
#include "btree_agg_worker.h"

namespace pc
{
    namespace worker
    {
        void BtreeaggWorker::run_function(Request *req)
        {
            (void)req;
            // std::cout << "Trying to find this address: " << (void *)(req.start_addr) << std::endl;
            // if ((void *)(req.start_addr) <= config.memory_1_max_range && (void *)addr >= config.memory_1_min_range)
            //{
            //   struct node *cur_ptr = (struct node *)(addr - (uint64_t)base + (uint64_t)ht);
            //   std::cout << "Current pointer is: " << (void *)cur_ptr << std::endl;
            //   std::cout << "Next pointer is:" << (void *)cur_ptr->next_ptr << std::endl;
            //   while (cur_ptr->next_ptr != NULL && (void *)((uint64_t)cur_ptr - ht + base) >= config.memory_1_min_range && (void *)((uint64_t)cur_ptr - ht + base) <= config.memory_1_max_range)
            //   {
            //     std::cout << "into here1" << std::endl;
            //     if (cur_ptr->key == key)
            //     {
            //       std::cout << "into here2" << std::endl;
            //       return std::make_pair(cur_ptr->value, 0);
            //     }
            //     else
            //     {
            //       cur_ptr = (struct node *)((uint64_t)cur_ptr->next_ptr - base + ht);
            //       std::cout << "Current pointer now is: " << (void *)cur_ptr << std::endl;
            //       std::cout << "Next pointer is:" << (void *)cur_ptr->next_ptr << std::endl;
            //     }
            //   }
            //   std::cout << "into here3" << std::endl;
            //   if (((void *)(uint64_t)cur_ptr - ht + base) < config.memory_1_min_range || (void *)((uint64_t)cur_ptr - ht + base) > config.memory_2_max_range)
            //   { // this should go to another blade
            //     std::cout << "into here4" << std::endl;
            //     return std::make_pair(((uint64_t)cur_ptr - ht + base), 1);
            //   }
            //   else if (cur_ptr->next_ptr == NULL)
            //   { // this should never happen
            //     std::cout << "Current pointer is empty " << std::endl;
            //   }
            // }
            // else
            //{
            //   std::cout << "This should never happen!" << std::endl;
            // }
        }

    }
}