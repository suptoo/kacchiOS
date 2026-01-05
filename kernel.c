/* kernel.c - Main kernel integrating all components */
#include "types.h"
#include "io.h"
#include "memory.h"
#include "process.h"
#include "scheduler.h"

// Test process functions
void process1(void) {
    printf_serial("Process 1 starting (PID: %d)\n", get_current_pid());
    int count = 0;
    while(count < 5) {
        printf_serial("  [P1] Iteration %d\n", count++);
        // Simulate some work
        for (volatile int i = 0; i < 100000; i++);
    }
    printf_serial("Process 1 completed\n");
    terminate_process(get_current_pid());
}

void process2(void) {
    printf_serial("Process 2 starting (PID: %d)\n", get_current_pid());
    int count = 0;
    while(count < 5) {
        printf_serial("  [P2] Iteration %d\n", count++);
        // Simulate some work
        for (volatile int i = 0; i < 100000; i++);
    }
    printf_serial("Process 2 completed\n");
    terminate_process(get_current_pid());
}

void process3(void) {
    printf_serial("Process 3 starting (PID: %d)\n", get_current_pid());
    int count = 0;
    while(count < 5) {
        printf_serial("  [P3] Iteration %d\n", count++);
        // Simulate some work
        for (volatile int i = 0; i < 100000; i++);
    }
    printf_serial("Process 3 completed\n");
    terminate_process(get_current_pid());
}

void kmain(void) {
    /* Initialize hardware */
    serial_init();
    
    /* Print welcome banner */
    serial_puts("\n");
    serial_puts("========================================\n");
    serial_puts("    kacchiOS - Extended Version\n");
    serial_puts("========================================\n");
    serial_puts("CSE 3202 Operating Systems Project\n");
    serial_puts("Features: Memory, Process, Scheduler\n");
    serial_puts("========================================\n\n");
    
    // Initialize all OS components
    serial_puts("[INIT] Initializing Memory Manager...\n");
    memory_init();
    
    serial_puts("[INIT] Initializing Process Manager...\n");
    process_manager_init();
    
    serial_puts("[INIT] Initializing Scheduler...\n");
    scheduler_init(SCHED_ROUND_ROBIN, 100);
    
    serial_puts("\n[KERNEL] Creating test processes...\n");
    
    // Create test processes
    int pid1 = create_process(process1, "TestProc1");
    int pid2 = create_process(process2, "TestProc2");
    int pid3 = create_process(process3, "TestProc3");
    
    if (pid1 > 0) {
        printf_serial("[KERNEL] Created process PID=%d\n", pid1);
        pcb_t* p1 = get_process(pid1);
        if (p1) add_to_ready_queue(p1);
    }
    
    if (pid2 > 0) {
        printf_serial("[KERNEL] Created process PID=%d\n", pid2);
        pcb_t* p2 = get_process(pid2);
        if (p2) add_to_ready_queue(p2);
    }
    
    if (pid3 > 0) {
        printf_serial("[KERNEL] Created process PID=%d\n", pid3);
        pcb_t* p3 = get_process(pid3);
        if (p3) add_to_ready_queue(p3);
    }
    
    serial_puts("\n[KERNEL] Starting scheduler...\n");
    serial_puts("========================================\n\n");
    
    // Simplified scheduling demonstration
    // In a real OS, this would be interrupt-driven
    serial_puts("[NOTE] This is a simplified scheduler demonstration\n");
    serial_puts("[NOTE] In a real OS, context switches would be interrupt-driven\n\n");
    
    int tick_counter = 0;
    int max_ticks = 500;  // Run for limited time in demo
    
    while(tick_counter < max_ticks) {
        // Simulate timer tick
        timer_tick();
        
        // Perform scheduling
        schedule();
        
        // Display stats periodically
        if (tick_counter % 100 == 0 && tick_counter > 0) {
            printf_serial("\n========================================\n");
            printf_serial("=== System Status (Tick %d) ===\n", tick_counter);
            printf_serial("========================================\n");
            memory_stats();
            serial_puts("\n");
            list_processes();
            serial_puts("\n");
            scheduler_stats();
            serial_puts("========================================\n\n");
        }
        
        tick_counter++;
    }
    
    serial_puts("\n========================================\n");
    serial_puts("=== Final System Statistics ===\n");
    serial_puts("========================================\n");
    memory_stats();
    serial_puts("\n");
    list_processes();
    serial_puts("\n");
    scheduler_stats();
    serial_puts("========================================\n");
    
    serial_puts("\n[KERNEL] Demonstration completed.\n");
    serial_puts("Thank you for using kacchiOS!\n\n");
    
    /* Halt system */
    for (;;) {
        __asm__ volatile ("hlt");
    }
}
