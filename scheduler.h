// scheduler.h
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

// Scheduling policies
typedef enum {
    SCHED_ROUND_ROBIN,
    SCHED_PRIORITY,
    SCHED_FCFS
} sched_policy_t;

// Scheduler configuration
typedef struct {
    sched_policy_t policy;
    uint32_t time_quantum;
    int aging_enabled;      // Bonus feature
    uint32_t max_priority;
} sched_config_t;

// Scheduler API
void scheduler_init(sched_policy_t policy, uint32_t quantum);
void schedule(void);
void context_switch(pcb_t* next);
void add_to_ready_queue(pcb_t* process);
void remove_from_ready_queue(int pid);
void set_scheduling_policy(sched_policy_t policy);
void set_time_quantum(uint32_t quantum);
void enable_aging(int enable);
pcb_t* pick_next_process(void);
void timer_tick(void);
void scheduler_stats(void);

#endif