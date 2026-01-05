// process.h
#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

#define MAX_PROCESSES 32
#define NULL_PID 0
#define INIT_PID 1

// Process states
typedef enum {
    TERMINATED = 0,
    READY,
    CURRENT,
    // Bonus states (for bonus points)
    BLOCKED,
    SUSPENDED
} process_state_t;

// Process Control Block (PCB)
typedef struct pcb {
    int pid;
    process_state_t state;
    uint32_t program_counter;
    uint32_t stack_pointer;
    uint32_t stack_base;
    uint32_t* page_directory;  // For future MMU support
    int priority;              // For scheduling
    uint32_t cpu_time;         // Total CPU time used
    struct pcb* next;          // For linked list in scheduler
} pcb_t;

// Process Manager API
void process_manager_init(void);
int create_process(void (*entry_point)(void), const char* name);
void terminate_process(int pid);
void set_process_state(int pid, process_state_t state);
pcb_t* get_process(int pid);
process_state_t get_process_state(int pid);
void list_processes(void);
int get_current_pid(void);
pcb_t* get_current_process(void);
int get_next_pid(void);

// Bonus: IPC functions
void send_message(int to_pid, void* msg, uint32_t size);
void* receive_message(int* from_pid);

#endif