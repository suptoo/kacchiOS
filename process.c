// process.c
#include "process.h"
#include "memory.h"
#include "io.h"
#include "types.h"

static pcb_t process_table[MAX_PROCESSES];
static int next_pid = INIT_PID;
static int current_pid = NULL_PID;
static int process_count = 0;

// Message queue for IPC (bonus)
typedef struct message {
    int from_pid;
    int to_pid;
    uint32_t size;
    void* data;
    struct message* next;
} message_t;

static message_t* message_queue = NULL;

// Initialize process manager
void process_manager_init(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = -1;
        process_table[i].state = TERMINATED;
        process_table[i].next = NULL;
    }
    
    // Create initial null/init process
    process_table[0].pid = NULL_PID;
    process_table[0].state = CURRENT;
    process_table[0].priority = 0;
    process_table[0].cpu_time = 0;
    process_table[0].next = NULL;
    const char* null_name = "null_process";
    for (int j = 0; null_name[j] && j < 31; j++) {
        ((char*)&process_table[0].page_directory)[j] = null_name[j];
    }
    
    current_pid = NULL_PID;
    process_count = 1;
    next_pid = INIT_PID;
    
    printf_serial("Process manager initialized\n");
}

// Create a new process
int create_process(void (*entry_point)(void), const char* name) {
    if (process_count >= MAX_PROCESSES) {
        printf_serial("Error: Maximum process limit reached\n");
        return -1;
    }
    
    // Find free slot
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == TERMINATED || process_table[i].pid == -1) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        printf_serial("Error: No free PCB slots\n");
        return -1;
    }
    
    // Allocate stack
    uint32_t stack_top = allocate_stack(next_pid);
    if (!stack_top) {
        printf_serial("Error: Failed to allocate stack for new process\n");
        return -1;
    }
    
    // Initialize PCB
    process_table[slot].pid = next_pid;
    process_table[slot].state = READY;
    process_table[slot].program_counter = (uint32_t)entry_point;
    process_table[slot].stack_pointer = stack_top;
    process_table[slot].stack_base = stack_top - STACK_SIZE;
    process_table[slot].priority = 1;  // Default priority
    process_table[slot].cpu_time = 0;
    process_table[slot].next = NULL;
    
    // Store process name in page_directory field (repurposed for name storage)
    const char* name_src = name ? name : "unnamed";
    int i = 0;
    while (name_src[i] && i < 31) {
        ((char*)&process_table[slot].page_directory)[i] = name_src[i];
        i++;
    }
    ((char*)&process_table[slot].page_directory)[i] = '\0';
    
    // Initialize stack for context switch
    // Push initial context onto stack
    uint32_t* stack = (uint32_t*)stack_top;
    
    // Simulated saved registers for context switch
    // This would be architecture-specific
    *(--stack) = 0x10;  // EFLAGS
    *(--stack) = 0x08;  // CS
    *(--stack) = (uint32_t)entry_point;  // EIP
    *(--stack) = 0;  // EAX
    *(--stack) = 0;  // ECX
    *(--stack) = 0;  // EDX
    *(--stack) = 0;  // EBX
    --stack;
    *stack = (uint32_t)stack;  // ESP
    *(--stack) = 0;  // EBP
    *(--stack) = 0;  // ESI
    *(--stack) = 0;  // EDI
    
    process_table[slot].stack_pointer = (uint32_t)stack;
    
    printf_serial("Created process PID %d: %s\n", next_pid, name);
    
    process_count++;
    return next_pid++;
}

// Terminate a process
void terminate_process(int pid) {
    if (pid == NULL_PID) {
        printf_serial("Error: Cannot terminate null process\n");
        return;
    }
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid) {
            // Free allocated memory
            free_stack(pid);
            
            // Clean up IPC messages (bonus)
            message_t* msg = message_queue;
            message_t* prev = NULL;
            while (msg) {
                if (msg->from_pid == pid || msg->to_pid == pid) {
                    if (prev) {
                        prev->next = msg->next;
                    } else {
                        message_queue = msg->next;
                    }
                    // Free message data
                    kfree(msg->data);
                    kfree(msg);
                    msg = prev ? prev->next : message_queue;
                } else {
                    prev = msg;
                    msg = msg->next;
                }
            }
            
            // Mark PCB as free
            process_table[i].state = TERMINATED;
            process_table[i].pid = -1;
            process_count--;
            
            printf_serial("Terminated process PID %d\n", pid);
            return;
        }
    }
    
    printf_serial("Error: Process PID %d not found\n", pid);
}

// Change process state
void set_process_state(int pid, process_state_t state) {
    pcb_t* proc = get_process(pid);
    if (proc) {
        process_state_t old_state = proc->state;
        proc->state = state;
        printf_serial("PID %d: %d -> %d\n", pid, old_state, state);
        
        if (state == CURRENT) {
            current_pid = pid;
        }
    }
}

// Get PCB by PID
pcb_t* get_process(int pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid) {
            return &process_table[i];
        }
    }
    return NULL;
}

// Get process state
process_state_t get_process_state(int pid) {
    pcb_t* proc = get_process(pid);
    return proc ? proc->state : TERMINATED;
}

// List all processes
void list_processes(void) {
    printf_serial("=== Process List (%d active) ===\n", process_count);
    printf_serial("PID\tState\t\tPC\t\tSP\t\tCPU Time\n");
    printf_serial("---\t-----\t\t---\t\t---\t\t--------\n");
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state != TERMINATED && process_table[i].pid != -1) {
            const char* state_str;
            switch (process_table[i].state) {
                case TERMINATED: state_str = "TERMINATED"; break;
                case READY: state_str = "READY"; break;
                case CURRENT: state_str = "CURRENT"; break;
                case BLOCKED: state_str = "BLOCKED"; break;
                case SUSPENDED: state_str = "SUSPENDED"; break;
                default: state_str = "UNKNOWN";
            }
            
            printf_serial("%d\t%s\t0x%x\t0x%x\t%u\n",
                process_table[i].pid,
                state_str,
                process_table[i].program_counter,
                process_table[i].stack_pointer,
                process_table[i].cpu_time);
        }
    }
}

// Utility functions
int get_current_pid(void) {
    return current_pid;
}

pcb_t* get_current_process(void) {
    return get_process(current_pid);
}

int get_next_pid(void) {
    return next_pid;
}

// Bonus: IPC implementation
void send_message(int to_pid, void* msg, uint32_t size) {
    if (!msg || size == 0) return;
    
    // Check if destination process exists
    pcb_t* dest = get_process(to_pid);
    if (!dest || dest->state == TERMINATED) {
        printf_serial("Error: Destination process %d not found\n", to_pid);
        return;
    }
    
    // Allocate message
    message_t* new_msg = (message_t*)kmalloc(sizeof(message_t));
    if (!new_msg) return;
    
    // Allocate message data
    new_msg->data = kmalloc(size);
    if (!new_msg->data) {
        kfree(new_msg);
        return;
    }
    
    // Copy message data
    memcpy(new_msg->data, msg, size);
    
    // Set message metadata
    new_msg->from_pid = current_pid;
    new_msg->to_pid = to_pid;
    new_msg->size = size;
    new_msg->next = NULL;
    
    // Add to queue
    if (!message_queue) {
        message_queue = new_msg;
    } else {
        message_t* last = message_queue;
        while (last->next) last = last->next;
        last->next = new_msg;
    }
    
    printf_serial("Message sent from PID %d to PID %d\n", current_pid, to_pid);
}

void* receive_message(int* from_pid) {
    pcb_t* current = get_current_process();
    if (!current) return NULL;
    
    // Find first message for current process
    message_t* msg = message_queue;
    message_t* prev = NULL;
    
    while (msg) {
        if (msg->to_pid == current->pid) {
            // Remove from queue
            if (prev) {
                prev->next = msg->next;
            } else {
                message_queue = msg->next;
            }
            
            // Return message data
            if (from_pid) *from_pid = msg->from_pid;
            void* data = msg->data;
            kfree(msg);
            return data;
        }
        prev = msg;
        msg = msg->next;
    }
    
    return NULL;
}