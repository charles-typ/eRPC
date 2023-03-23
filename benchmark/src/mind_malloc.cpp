#include "mind_malloc.h"

const unsigned long mind_print_internval = 100000;
const unsigned int max_num_mmap = 512;
static int mind_alloc_allocated = 0;
static unsigned long long mind_address[max_num_mmap];
static unsigned long long mind_address_next[max_num_mmap];
static int mind_alloc_next_idx = 0;
unsigned long long print_idx = 0;

void mind_malloc_add_mem(void)
{
    int next_mn_idx = mind_alloc_allocated;
    int is_remote = 0;
    if (next_mn_idx >= max_num_mmap)
    {
        std::cout << "Cannot allocate more memory" << std::endl;
        exit(-1);
    }
    mind_alloc_allocated++;

#ifndef MIND_MMAP
    mind_address[next_mn_idx] = (unsigned long long)mmap(NULL, TEST_MACRO_ALLOC_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#else
    mind_address[next_mn_idx] = (unsigned long long)mmap(NULL, TEST_MACRO_ALLOC_SIZE, PROT_READ | PROT_WRITE, TEST_REMOTE_ALLOC_FLAG, -1, 0);
    is_remote = 1;
#endif
    if (mind_address[next_mn_idx])
    {
        mind_address_next[next_mn_idx] = mind_address[next_mn_idx];
        printf("MIND_ALLOC :: memory added [region: %d | is_remote: %d] - 0x%lx\n",
               next_mn_idx, is_remote, (unsigned long)mind_address[next_mn_idx]);
    } else {
        std::cout << "Cannot allocate more memory :: " << mind_address[next_mn_idx] << std::endl;
        exit(-1);
    }
}

void *mind_malloc(unsigned long size)
{
    void *ret = (void*)NULL;
#ifdef MIND_MMAP_RR // round robin
    // round robing
    for (int i = 0; i < mind_alloc_allocated; i ++)
    {
        mind_alloc_next_idx = (mind_alloc_next_idx + 1) % mind_alloc_allocated;
        if (mind_address_next[mind_alloc_next_idx] + size - mind_address[mind_alloc_next_idx] < TEST_MACRO_ALLOC_SIZE)
        {
            ret = (void *)mind_address_next[mind_alloc_next_idx];
            mind_address_next[mind_alloc_next_idx] += size;

            print_idx++;
            if (print_idx % mind_print_internval == 0)
            {
                printf("MIND_ALLOC [%llu]:: 0x%lx (len: %lu) from mem [%d] (used: %llu KB/%llu KB)\n",
                        print_idx, (unsigned long)ret, size, mind_alloc_next_idx,
                    ((unsigned long long)ret - mind_address[mind_alloc_next_idx]) / 1024,
                    (unsigned long long)TEST_MACRO_ALLOC_SIZE / 1024);
            }
            break;
        }
    }
#else
    // sequential
    if (mind_address_next[mind_alloc_next_idx] + size - mind_address[mind_alloc_next_idx] < TEST_MACRO_ALLOC_SIZE ||
        (++mind_alloc_next_idx) < mind_alloc_allocated )
    {
        ret = (void *)mind_address_next[mind_alloc_next_idx];
        mind_address_next[mind_alloc_next_idx] += size;

        print_idx++;
        if (print_idx % mind_print_internval == 0)
        {
            printf("MIND_ALLOC [%llu]:: 0x%lx (len: %lu) from mem [%d] (used: %llu KB/%llu KB)\n",
                    print_idx, (unsigned long)ret, size, mind_alloc_next_idx,
                   ((unsigned long long)ret - mind_address[mind_alloc_next_idx]) / 1024,
                   (unsigned long long)TEST_MACRO_ALLOC_SIZE / 1024);
        }
    }
#endif
    return ret;
}

void mind_free_all()
{
    for (int i = 0; i < mind_alloc_allocated; i++)
    {
        if (mind_address[i])
            munmap((void*)mind_address[i], TEST_MACRO_ALLOC_SIZE);
    }
}

void dump_mem_layout(const char *filename, void* root_ptr)
{
    FILE *mem_layout_bin = fopen(filename, "wb");
    if(mem_layout_bin != NULL)
    {
        int wsize = fwrite((void *)&mind_alloc_allocated, sizeof(mind_alloc_allocated), 1, mem_layout_bin);
        wsize += fwrite((void *)&root_ptr, sizeof(root_ptr), 1, mem_layout_bin);
        if (wsize == 0)
        {
            printf("Cannot start writing: %s [%d]\n", filename, wsize);
            return ;
        }

        for (int i = 0; i < mind_alloc_allocated; i++)
        {
            // size_t to_go = TEST_MACRO_ALLOC_SIZE;
            // printf("Written [0x%lx] - len: %lu\n",
            //         (unsigned long)mind_address[i], TEST_MACRO_ALLOC_SIZE - to_go);
            const size_t wrote = fwrite((void*)mind_address[i], TEST_MACRO_ALLOC_SIZE, 1, mem_layout_bin);
            // if (wrote)
            //     break;
            // to_go -= wrote;
            printf("Written [0x%lx] - len: %lu\n",
                (unsigned long)mind_address[i], wrote * TEST_MACRO_ALLOC_SIZE);
        }
        fclose(mem_layout_bin);
    } else {
        printf("Cannot open file for writing: %s\n", filename);
    }
}

void *load_mem_layout(const char *filename)
{
    FILE *mem_layout_bin = fopen(filename, "rb");
    int load_region_size = 0;
    void *root_ptr = NULL;
    if (mem_layout_bin != NULL)
    {
        int rsize = fread((void *)&load_region_size, sizeof(load_region_size), 1, mem_layout_bin);
        rsize += fread((void *)&root_ptr, sizeof(root_ptr), 1, mem_layout_bin);
        if (!rsize || !load_region_size || !root_ptr)
        {
            printf("Cannot start reading: %s [%d] 0x%lx\n", filename, rsize, (unsigned long)root_ptr);
            return NULL;
        }
        printf("Number of regions: %d | root: 0x%lx\n", load_region_size, (unsigned long)root_ptr);

        for (int i = 0; i < load_region_size; ++i)
        {
            mind_malloc_add_mem();
        }

        for (int i = 0; i < mind_alloc_allocated; i++)
        {
            // size_t to_go = TEST_MACRO_ALLOC_SIZE;
            // printf("Read [0x%lx] - len: %lu\n",
            //         (unsigned long)mind_address[i], TEST_MACRO_ALLOC_SIZE - to_go);
            // while(to_go > 0)
            // {
            //     const size_t wrote = fread((void*)mind_address[i], to_go, 1, mem_layout_bin);
            //     if (wrote == 0)
            //         break;
            //     to_go -= wrote;
            // }
            const size_t read = fread((void*)mind_address[i], TEST_MACRO_ALLOC_SIZE, 1, mem_layout_bin);
            printf("Read [0x%lx] - len: %lu\n",
                   (unsigned long)mind_address[i], read * TEST_MACRO_ALLOC_SIZE);
        }
        fclose(mem_layout_bin);
    } else {
        printf("Cannot open file for reading: %s\n", filename);  
        return NULL;
    }
    // return (void *)mind_address[0];
    return root_ptr;
}