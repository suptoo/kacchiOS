// memory.h
#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

#define HEAP_START   ((uint32_t)&__kernel_end)  // Start after kernel
#define HEAP_SIZE    0x00100000  // 1MB heap
#define STACK_SIZE   0x00002000  // 8KB stack per process
#define MAX_BLOCKS   256

// External symbols from linker script
extern uint32_t __kernel_end;

// Memory block structure for heap management
typedef struct mem_block {
    uint32_t start_addr;
    uint32_t size;
    int is_free;
    struct mem_block* next;
    struct mem_block* prev;
} mem_block_t;

// Stack allocation record
typedef struct {
    uint32_t base_addr;
    uint32_t size;
    int pid;  // Process ID this stack belongs to
} stack_info_t;

// Memory Manager API
void memory_init(void);
uint32_t allocate_stack(int pid);
void free_stack(int pid);
void* kmalloc(uint32_t size);
void kfree(void* ptr);
void memory_stats(void);
uint32_t get_free_memory(void);
uint32_t get_total_memory(void);

#endif