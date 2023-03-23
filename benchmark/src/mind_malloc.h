#ifndef __MIND_MALLOC_H__
#define __MIND_MALLOC_H__

#define TEST_REMOTE_ALLOC_FLAG 0xfe
#define MAX_MIND_ALLOC_SIZE (4 * 1024 * 1024 * 1024UL)  // 4 GB
#ifdef MIND_MMAP_RR // round robin
#define TEST_MACRO_ALLOC_SIZE (256 * 1024 * 1024UL)  // 256 MB
#else
#define TEST_MACRO_ALLOC_SIZE MAX_MIND_ALLOC_SIZE
#endif

#include <sys/mman.h>
#include <sched.h>
#include <iostream>
#include <stdio.h>

void mind_malloc_add_mem(void);
void *mind_malloc(unsigned long size);
void mind_free_all();
void dump_mem_layout(const char *filename, void* root_ptr);
void *load_mem_layout(const char *filename);

#endif  // __MIND_MALLOC_H__