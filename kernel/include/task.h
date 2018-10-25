/*
 * Copyright (c) 2018 Jie Zheng
 */

#ifndef _TASK_H
#define _TASK_H
#include <kernel/include/kernel.h>
#include <lib/include/types.h>
#include <x86/include/interrupt.h>
#include <kernel/include/printk.h>
#include <lib/include/list.h>
#include <kernel/include/userspace_vma.h>
#include <kernel/include/timer.h>

enum task_state {
    TASK_STATE_ZOMBIE = 0,
    TASK_STATE_RUNNING,
    TASK_STATE_INTERRUPTIBLE,
    TASK_STATE_EXITING
};

struct task {
    struct list_elem list;
    enum task_state state;
    /*
     * The x86 cpu state, please refer to x86/include/interrupt.h
     * including cpu state in signaled context.
     */
    struct x86_cpustate * cpu;
    struct x86_cpustate * signaled_cpu;
    /*
     * per-task VMAs and  page Directory
     */
    struct list_elem vma_list;

    /*
     * If the task is in PL3 context, before switching task, we should 
     * invoke enable_task_paging().
     */
    uint32_t * page_directory;
    /*
     * stack at PL 0 and 3, when privilege_level is DPL_0,
     * privilege_level3_stack is NULL.
     */
    void * privilege_level0_stack;
    void * signaled_privilege_level0_stack;
    /*
     * each time the PL3 code traps into PL0, the priviege level 0 stack
     * top is switched to. 
     * This requires that before the task is scheduled, the 
     * privilege_level0_stack_top is put into TSS(loaded into task register)'s
     * esp0
     */
    uint32_t privilege_level0_stack_top;
    uint32_t signaled_privilege_level0_stack_top;

    // Indicator of signal context.
    // when the signal handler is invoked, it must be set atomically, and
    // cleared after signal handler returns to kernel land.
    uint8_t signal_scheduled;
    // The Userland/kernel land  entry point.
    uint32_t entry;
    /*
     * this field specifies in which PL the task runs
     * it often in [DPL_0, DPL_3]
     */
    uint32_t privilege_level:2;
    uint32_t exit_code;
    
    // Point to the timer which often on the stack. in case we release a running
    // task, we need to delete the timer entry first
    struct timer_entry * current_timer;
};
extern struct task * current;
#define IS_TASK_KERNEL_TYPE (_task) ((_task)->privilege_level == DPL_0)

#define disable_preempt() cli()
#define enable_preempt() sti()

#define push_cpu_state(__cpu) {\
    ASSERT(current); \
    (__cpu) = current->cpu; \
}

#define pop_cpu_state(__cpu) {\
    ASSERT(current); \
    current->cpu = (__cpu); \
}

void
transit_state(struct task * task, enum task_state target_state);


struct list_elem * get_task_list_head(void);
struct task * malloc_task(void);
void free_task(struct task * _task);


uint32_t schedule(struct x86_cpustate * cpu);
void task_init(void);
void task_put(struct task * _task);
struct task * task_get(void);

void schedule_enable(void);
void schedule_disable(void);
int ready_to_schedule(void);

int vma_in_task(struct task * task, struct vm_area * vma);

void dump_task_vm_areas(struct task * _task);
int userspace_map_vm_area(struct task * task, struct vm_area * vma);
int
userspace_map_page(struct task * task,
    uint32_t virt_addr,
    uint32_t phy_addr,
    uint8_t write_permission,
    uint8_t page_writethrough,
    uint8_t page_cachedisable);

int userspace_remap_vm_area(struct task * task, struct vm_area * vma);

int userspace_evict_vma(struct task * task, struct vm_area * vma);
int
userspace_evict_page(struct task * task,
    uint32_t virt_addr,
    int _reclaim_page_table);
int reclaim_page_table(struct task * task, uint32_t virt_addr);

uint32_t reclaim_task(struct task * task);
int enable_task_paging(struct task * task);
void dump_tasks(void);

uint32_t
handle_userspace_page_fault(struct task * task,
    struct x86_cpustate * cpu,
    uint32_t linear_addr);


void
yield_cpu(void);

uint32_t
create_kernel_task(void (*entry)(void), struct task ** task_ptr);

void
yield_cpu(void);

void
sleep(uint32_t milisecond);

void
task_misc_init(void);

#endif
