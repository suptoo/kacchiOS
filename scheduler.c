// scheduler.c
#include "scheduler.h"
#include "memory.h"
#include "io.h"

static pcb_t* ready_queue = NULL;
static sched_config_t config;
static uint32_t timer_ticks = 0;
static uint32_t current_tick = 0;
static uint32_t context_switches = 0;
static pcb_t* idle_process = NULL;

// Initialize scheduler
void scheduler_init(sched_policy_t policy, uint32_t quantum) {
    config.policy = policy;
    config.time_quantum = quantum;
    config.aging_enabled = 0;
    config.max_priority = 10;
    
    ready_queue = NULL;
    timer_ticks = 0;
    current_tick = 0;
    context_switches = 0;
    
    // Create idle process if no processes are ready
    idle_process = get_process(NULL_PID);
    
    printf_serial("Scheduler initialized with ");
    switch (policy) {
        case SCHED_ROUND_ROBIN:
            printf_serial("Round Robin (quantum: %u)\n", quantum);
            break;
        case SCHED_PRIORITY:
            printf_serial("Priority Scheduling\n");
            break;
        case SCHED_FCFS:
            printf_serial("FCFS\n");
            break;
    }
}

// Main scheduling function
void schedule(void) {
    pcb_t* current = get_current_process();
    pcb_t* next = pick_next_process();
    
    if (!next) {
        // No process in ready queue, run idle
        next = idle_process;
    }
    
    if (current != next) {
        context_switch(next);
    }
}

// Context switch (simplified - actual implementation needs assembly)
void context_switch(pcb_t* next) {
    pcb_t* current = get_current_process();
    
    if (current == next) return;
    
    printf_serial("Context switch: PID %d -> PID %d\n", 
                 current ? current->pid : -1, 
                 next->pid);
    
    // Save current process state
    if (current && current->state == CURRENT) {
        // In real OS, save registers to current->stack_pointer
        current->state = READY;
        add_to_ready_queue(current);
    }
    
    // Update next process
    next->state = CURRENT;
    remove_from_ready_queue(next->pid);
    
    // Update current PID
    if (current) {
        current->cpu_time += current_tick;
    }
    current_tick = 0;
    
    // In actual implementation:
    // 1. Save current context to its stack
    // 2. Load next context from its stack
    // 3. Switch stacks
    // 4. Resume execution
    
    context_switches++;
    
    // For demonstration, just update the current process pointer
    // In real implementation, this would involve switching stacks and PC
}

// Add process to ready queue
void add_to_ready_queue(pcb_t* process) {
    if (!process || process->state == TERMINATED) return;
    
    process->next = NULL;
    
    if (!ready_queue) {
        ready_queue = process;
    } else {
        // Add to end of queue (for FCFS/Round Robin)
        if (config.policy == SCHED_ROUND_ROBIN || config.policy == SCHED_FCFS) {
            pcb_t* last = ready_queue;
            while (last->next) last = last->next;
            last->next = process;
        }
        // Insert based on priority (for Priority Scheduling)
        else if (config.policy == SCHED_PRIORITY) {
            pcb_t* current = ready_queue;
            pcb_t* prev = NULL;
            
            while (current && current->priority >= process->priority) {
                prev = current;
                current = current->next;
            }
            
            if (!prev) {
                process->next = ready_queue;
                ready_queue = process;
            } else {
                process->next = current;
                prev->next = process;
            }
        }
    }
    
    process->state = READY;
}

// Remove process from ready queue
void remove_from_ready_queue(int pid) {
    if (!ready_queue) return;
    
    if (ready_queue->pid == pid) {
        ready_queue = ready_queue->next;
        return;
    }
    
    pcb_t* current = ready_queue;
    pcb_t* prev = NULL;
    
    while (current && current->pid != pid) {
        prev = current;
        current = current->next;
    }
    
    if (current) {
        if (prev) prev->next = current->next;
        current->next = NULL;
    }
}

// Pick next process based on scheduling policy
pcb_t* pick_next_process(void) {
    if (!ready_queue) return NULL;
    
    pcb_t* selected = NULL;
    
    switch (config.policy) {
        case SCHED_ROUND_ROBIN:
        case SCHED_FCFS:
            selected = ready_queue;
            break;
            
        case SCHED_PRIORITY:
            selected = ready_queue;
            pcb_t* current = ready_queue;
            while (current) {
                if (current->priority < selected->priority) {
                    selected = current;
                }
                current = current->next;
            }
            break;
    }
    
    // Bonus: Apply aging
    if (config.aging_enabled) {
        pcb_t* current = ready_queue;
        while (current) {
            if (current != selected && (uint32_t)current->priority < config.max_priority) {
                current->priority++;  // Increase priority of waiting processes
            }
            current = current->next;
        }
        // Reset selected process priority
        if (selected && selected->priority > 1) {
            selected->priority--;
        }
    }
    
    return selected;
}

// Timer interrupt handler (called by timer ISR)
void timer_tick(void) {
    timer_ticks++;
    current_tick++;
    
    pcb_t* current = get_current_process();
    if (current && current->pid != NULL_PID) {
        // Check if time quantum expired
        if (config.policy == SCHED_ROUND_ROBIN && 
            current_tick >= config.time_quantum) {
            printf_serial("Time quantum expired for PID %d\n", current->pid);
            schedule();
        }
    }
}

// Change scheduling policy
void set_scheduling_policy(sched_policy_t policy) {
    config.policy = policy;
    printf_serial("Scheduling policy changed\n");
}

// Change time quantum
void set_time_quantum(uint32_t quantum) {
    config.time_quantum = quantum;
    printf_serial("Time quantum set to %u\n", quantum);
}

// Enable/disable aging (bonus feature)
void enable_aging(int enable) {
    config.aging_enabled = enable;
    printf_serial("Aging %s\n", enable ? "enabled" : "disabled");
}

// Display scheduler statistics
void scheduler_stats(void) {
    printf_serial("=== Scheduler Statistics ===\n");
    printf_serial("Total timer ticks: %u\n", timer_ticks);
    printf_serial("Context switches: %u\n", context_switches);
    printf_serial("Processes in ready queue: ");
    
    int count = 0;
    pcb_t* current = ready_queue;
    while (current) {
        count++;
        current = current->next;
    }
    printf_serial("%d\n", count);
    
    printf_serial("Current time quantum: %u\n", config.time_quantum);
    printf_serial("Aging: %s\n", config.aging_enabled ? "ON" : "OFF");
}