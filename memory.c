// memory.c
#include "memory.h"
#include "io.h"  // For serial output

static mem_block_t* free_list = NULL;
static stack_info_t stacks[MAX_BLOCKS];
static int stack_count = 0;
static uint32_t heap_used = 0;

// Initialize memory manager
void memory_init(void) {
    uint32_t heap_start_addr = (uint32_t)&__kernel_end;
    // Align to page boundary (4KB)
    heap_start_addr = (heap_start_addr + 0xFFF) & ~0xFFF;
    
    // Initialize heap with one big free block
    free_list = (mem_block_t*)heap_start_addr;
    free_list->start_addr = heap_start_addr + sizeof(mem_block_t);
    free_list->size = HEAP_SIZE - sizeof(mem_block_t);
    free_list->is_free = 1;
    free_list->next = NULL;
    free_list->prev = NULL;
    
    stack_count = 0;
    heap_used = 0;
    
    printf_serial("Memory manager initialized\n");
    printf_serial("Heap starts at: 0x%x\n", heap_start_addr);
    printf_serial("Heap size: %u bytes\n", HEAP_SIZE);
}

// Allocate stack for a process
uint32_t allocate_stack(int pid) {
    if (stack_count >= MAX_BLOCKS) {
        printf_serial("Error: Maximum stack count reached\n");
        return 0;
    }
    
    // Allocate stack memory from heap
    void* stack_addr = kmalloc(STACK_SIZE);
    if (!stack_addr) {
        printf_serial("Error: Failed to allocate stack for PID %d\n", pid);
        return 0;
    }
    
    // Record stack allocation
    stacks[stack_count].base_addr = (uint32_t)stack_addr;
    stacks[stack_count].size = STACK_SIZE;
    stacks[stack_count].pid = pid;
    stack_count++;
    
    printf_serial("Stack allocated for PID %d at 0x%x\n", pid, stack_addr);
    return (uint32_t)stack_addr + STACK_SIZE;  // Return stack pointer (top of stack)
}

// Free stack when process terminates
void free_stack(int pid) {
    for (int i = 0; i < stack_count; i++) {
        if (stacks[i].pid == pid) {
            kfree((void*)stacks[i].base_addr);
            
            // Remove from array (shift remaining)
            for (int j = i; j < stack_count - 1; j++) {
                stacks[j] = stacks[j + 1];
            }
            stack_count--;
            
            printf_serial("Stack freed for PID %d\n", pid);
            return;
        }
    }
    printf_serial("Warning: No stack found for PID %d\n", pid);
}

// First-fit heap allocator
void* kmalloc(uint32_t size) {
    if (size == 0) return NULL;
    
    // Align to 8 bytes
    size = (size + 7) & ~7;
    
    mem_block_t* current = free_list;
    while (current) {
        if (current->is_free && current->size >= size) {
            // Split block if enough space left
            if (current->size > size + sizeof(mem_block_t) + 8) {
                mem_block_t* new_block = (mem_block_t*)(current->start_addr + size);
                new_block->start_addr = current->start_addr + size + sizeof(mem_block_t);
                new_block->size = current->size - size - sizeof(mem_block_t);
                new_block->is_free = 1;
                new_block->next = current->next;
                new_block->prev = current;
                
                if (current->next) {
                    current->next->prev = new_block;
                }
                current->next = new_block;
                current->size = size;
            }
            
            current->is_free = 0;
            heap_used += current->size;
            
            return (void*)current->start_addr;
        }
        current = current->next;
    }
    
    printf_serial("Error: Out of memory (requested %u bytes)\n", size);
    return NULL;
}

// Free heap memory
void kfree(void* ptr) {
    if (!ptr) return;
    
    // Find the block containing this address
    mem_block_t* current = free_list;
    while (current) {
        if ((void*)current->start_addr == ptr) {
            current->is_free = 1;
            heap_used -= current->size;
            
            // Coalesce with next block if free
            if (current->next && current->next->is_free) {
                current->size += sizeof(mem_block_t) + current->next->size;
                current->next = current->next->next;
                if (current->next) {
                    current->next->prev = current;
                }
            }
            
            // Coalesce with previous block if free
            if (current->prev && current->prev->is_free) {
                current->prev->size += sizeof(mem_block_t) + current->size;
                current->prev->next = current->next;
                if (current->next) {
                    current->next->prev = current->prev;
                }
            }
            
            printf_serial("Freed memory at 0x%x\n", ptr);
            return;
        }
        current = current->next;
    }
    
    printf_serial("Error: Attempt to free invalid address 0x%x\n", ptr);
}

// Display memory statistics
void memory_stats(void) {
    printf_serial("=== Memory Statistics ===\n");
    printf_serial("Total heap: %u bytes\n", HEAP_SIZE);
    printf_serial("Used heap: %u bytes\n", heap_used);
    printf_serial("Free heap: %u bytes\n", HEAP_SIZE - heap_used);
    printf_serial("Active stacks: %d\n", stack_count);
    
    mem_block_t* current = free_list;
    int free_blocks = 0;
    while (current) {
        if (current->is_free) free_blocks++;
        current = current->next;
    }
    printf_serial("Free blocks: %d\n", free_blocks);
}

// Utility functions
uint32_t get_free_memory(void) {
    return HEAP_SIZE - heap_used;
}

uint32_t get_total_memory(void) {
    return HEAP_SIZE;
}