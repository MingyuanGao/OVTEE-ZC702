/* 
 * OpenVirtualization: 
 * For additional details and support contact developer@sierraware.com.
 * Additional documentation can be found at www.openvirtualization.org
 * 
 * Copyright (C) 2011 SierraWare
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

/* 
 * global variables defintions
 */

#ifndef __OTZ_GLOBAL_H__
#define __OTZ_GLOBAL_H__

#define LIBC_HEAP_SIZE (1 << 23) /* 8MB */

#define TASK_ID_START 0x11
#define MAX_ASID 	  256
#define SHARED_MEM_INSTANCE 16
#define MAX_NAME_LEN  255

#include <sw_link.h>
#include <sw_board.h>
#include <cpu_data.h>
#include <otz_common.h>
#include <sw_types.h>
#include <sw_timer.h>
#include <sw_lock.h>
#include <sw_buddy.h>

#include <tzhyp.h>
#include <sw_modinit.h>
#include <mem_mng.h>

/**
 * @brief Task IRQ handler 
 * 
 * @param interrupt: Interrupt number
 * @param data: IRQ handler parameter
 */
typedef void (irq_handler)(sw_uint interrupt, void *data);

/**
 * @brief Global variables structure
 */
struct sw_global {
	/*! next ready to run task ID */
	sw_short_int task_id_pool[MAX_ASID];
	sw_int global_errno;
	sw_uint g_ns_notify_pending;
	sw_uint* pagetable_addr;
	sw_int linux_task_id;
	sw_uint linux_return_flag;
	/*! Global trustzone hypervisor list */
	struct tzhyp_values tzhyp_val;
	/*! Symbol table start address */ 
	sw_uint symtab_start;
	/*! Symbol table size */ 
	sw_uint symtab_size;
	/*! Physical address of loaded 'c' Library */ 
	sw_uint lib_pa;
	/*! Virtual address of loaded 'c' Library */ 
	sw_uint lib_va;
	/*! Loaded 'c' Library size */
	sw_uint lib_size;
	/*! Global task list */
	struct link task_list;
	/*! Global ready to run task list */
	struct link ready_to_run_list;
	/*! Spinlock to lock the task list*/
	struct lock_shared task_list_lock;
	/*! Spinlock to lock the ready to run list*/
	struct lock_shared ready_to_run_lock;
	/*! Page reference list for shared memory tracking */
	struct link page_ref_list;
	/*! Opened file/dev list */
	struct link file_dev_list;
	/*! Current task ID */
	sw_uint current_task_id; 
	/*! Task IRQ handler */
	irq_handler *task_irq_handler[NO_OF_INTERRUPTS_IMPLEMENTED];
	/*! Task IRQ handler data */
	void *task_irq_handler_data[NO_OF_INTERRUPTS_IMPLEMENTED];
	/*! Pointer to device permission list */
	struct link device_acl_list;
	/*! Execution mode flag */
	sw_uint exec_mode;
	/*! Pointer to common heap pool */
	struct sw_heap_info heap_info;
#ifdef NEWLIB_SUPPORT
	/*! LibC heap size */
	sw_uint libc_heap_size;
	/*! LibC heap start */
	void* libc_heap_start;
	/*! LibC heap current */
	sw_uint libc_heap_current;
#endif
	/*! Kernel Heap init done flag */
	sw_short_int heap_init_done;
	
	struct sw_devices_head sw_dev_head;
	struct timer_cpu_info timer_cpu_info;	
	mem_info sw_mem_info;
	struct dispatch_global *g_dispatch_data;
	/*! Spinlock to lock the semaphore list*/
	struct lock_shared semaphore_list_lock;
	/* Semaphore list */
	struct link semaphore_list;
	/*! Spinlock to lock the semaphore list*/
	struct lock_shared mutex_list_lock;
	/* Semaphore list */
	struct link mutex_list;
	/*! Spinlock to lock the shared memory list*/
	struct lock_shared shared_memory_ref_lock;
	/* Shared memory list*/
	struct link shared_memory_ref;
	/*! Shared memory id pool*/
	sw_short_int shm_id_pool[SHARED_MEM_INSTANCE];
	/* Epoch value used in calculating time */
	sw_big_ulong epoch;
};

/**
 * @brief 
 */
typedef struct acl_device{
	struct link head;
	sw_uint did;
	struct link group_head;
}acl_device;

/**
 * @brief 
 */
typedef struct acl_group{
	struct link head;
	sw_uint gid;
	struct link user_head;
}acl_group;

/**
 * @brief 
 */
typedef struct acl_user{
	struct link head;
	sw_uint uid;
}acl_user;

/**
 * @brief Task state constants
 */
enum task_state_e {
	TASK_STATE_SUSPEND = 0,
	TASK_STATE_WAIT,
	TASK_STATE_READY_TO_RUN,
	TASK_STATE_RUNNING
};

/**
 * @brief Global variable structure
 */
extern struct sw_global global_val;

/**
 * @brief Global initialization
 * 
 * This function initializes the global variables structure
 */
void global_init(void);

/**
 * @brief Get the current task
 *
 * This function returns the current task which is running
 *
 * @return : Pointer to the task structure or NULL
 */
struct sw_task* get_current_task(void);

/**
 * @brief Get the next ready to run task
 *
 * This function returns the next task which is ready to run
 *
 * @return : Pointer to the task structure or NULL
 */
struct sw_task* get_next_task(void);

/**
 * @brief Update the current task ID
 *
 * @param task: Pointer to the task structure
 */
void update_current_task(struct sw_task*);

/**
 * @brief 
 *  Returnd the ID of the current task
 * @return 
 */
int get_current_task_id(void);

/**
 * @brief Schedules the task 
 *
 * This function implements the functionality of scheduling the given task
 *
 * @param task: Pointer to task structure
 */
void schedule_task(struct sw_task* task);

/**
 * @brief 
 *      This function adds the task to the ready to run list as the first
 *      element so that the next task scheduled will be this
 * @param task
 *      Pointer to the Next structure
 */

void schedule_task_next(struct sw_task* task);

/**
 * @brief Schedules the task 
 *
 * This function implements the functionality of scheduling the given task
 *
 * @param task_id: ID of the task
 */
void schedule_task_id(int task_id);

/**
 * @brief Suspends the task 
 *
 * This function suspend the given task
 *
 * @param task_id: Task ID
 * @param state: State of the task
 */
void suspend_task(int task_id, int state);

/**
 * @brief Prints the all task names
 */
void print_task_list(void);


/**
 * @brief Register IRQ handler for the specificed interrupt ID 
 *
 * This function register the IRQ handler for the specified interrupt ID. This
 * could be a function from any task.
 *
 * @param interrupt : Interrupt ID
 * @param handler: Function pointer to IRQ handler
 * @param data: IRQ handler data
 */
void register_secure_irq_handler(sw_uint interrupt ,irq_handler handler, void *data);

/**
 * @brief Invoke the registered IRQ handler of specified interrupt number
 *
 * This function invokes the corresponding registered IRQ handler of the
 * specified interrupt ID.
 *
 * @param interrupt: Interrupt ID
 * @param regs: Context registers
 */
void invoke_irq_handler(sw_uint interrupt, struct swi_temp_regs *regs);

/**
 * @brief 
 *
 * @param interrupt
 *
 * @return 
 */
void* get_interrupt_data(sw_uint interrupt);

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
/**
 * @brief Register the notification shared memory data for notification.
 *
 * This function creates the page table entry for sharing the memory data for
 * notification
 *
 * @param notify_data_phys: Physical address of the shared memory used for
 * Notification data
 *
 * @return otz_return_t:
 * OTZ_OK - Shared memory registration success\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int register_notify_data(sw_phy_addr data_phys);

/**
 * @brief Un-register the shared memory used for the notification
 *
 * This function removed the page table entry created for sharing the memory data
 * of notification
 */
void unregister_notify_data(void);

/**
 * @brief Sets the values of Notification
 *
 * This function sets the notification data which is used by non-secure
 * application upon notification from secure world.
 *
 * @param guest_no: Guest number
 * @param service_id: Service ID 
 * @param session_id: Session ID
 * @param enc_id: Encoded context ID
 * @param client_pid: Non-secure application PID
 */
void set_notify_data(int guest_no, int service_id, int session_id, int enc_id, 
		int client_pid, int dev_file_id);
#endif

/* scheduler.c */
/**
 * @brief Scheduler function
 *
 * This function implements the round-robin scheduler
 */
void scheduler(void);


/* cpu_timer.c */
/**
 * @brief Enable secure kernel timer
 * 
 * This function enables the secure kernel timer
 */
void enable_timer(void);

/**
 * @brief Disable secure kernel timer
 * 
 * This function disables the secure kernel timer
 */
void disable_timer(void);


/**
 * @brief Init secure kernel timer
 * 
 * This function initialize the secure kernel timer
 */
void timer_init(void);

/**
 * @brief 
 *  Returnd the ID of the current task
 * @return Returns the current task ID
 */
int get_current_task_id(void);
/**
 * @brief Invoke scheduler
 *
 * This function invokes the scheduler to schedule the next ready task
 */
void schedule(void);

/**
 * @brief 
 *   This function converts the clockcycles to time in seconds and
 *   nanoseconds
 *   This function definition depends on clock used
 * @param clockcycles
 *  Number of clockcycles
 * @return 
 *  The converted time in seconds and nanoseconds
 */
sw_big_ulong clockcycles_to_timeval(sw_uint clockcycles);

/**
 * @brief 
 *      It converts the time (seconds and nanoseconds)
 *      to the number of clockcycles
 * @param time
 *
 * @return 
 */
sw_big_ulong timeval_to_clockcycles(sw_timeval *time);

/**
 * @brief 
 *  It reads and returns the value of the timer
 *  which is used as the free running counter
 * @return 
 */
sw_uint read_freerunning_cntr(void);

/**
 * @brief
 *  This function writes the number of clockcycles to be expired before the
 *  next tick to the tick timer and enables the tick timer
 *
 * @param usecs
 */
void trigger_tick(sw_big_ulong usecs);

/**
 * @brief 
 *  This function returns the maximum time that can be kept track of before it
 *  gets expired.
 *  (Eg : The time taken for running from  0xFFFFFFFF to 0x00000000)
 * @return 
 */
sw_big_ulong get_timer_period(void);

/**
 * @brief
 *  This function returns the resolution which can be obtained with the given
 *  clock
 *
 * @return 
 */
sw_big_ulong get_clock_period(void);

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
/**
 * @brief Invoke non-secure kernel callback handler
 *
 * @param guest_no: Guest No
 * @param svc_id: Service ID
 * @param session_id: Session ID
 * @param enc_id: Encoded context ID
 * @param client_pid: Client process ID
 */
void invoke_ns_callback(sw_int guest_no, sw_uint svc_id, 
		sw_uint session_id, sw_uint enc_id, sw_uint client_pid,
		sw_uint dev_file_id);
#endif

#ifdef NEWLIB_SUPPORT
/**
 * @brief Populate libc heap boundary information in parameters for sbrk
 *
 * @param heap_start: Starting address of heap
 * @param heap_size: Size of heap
 * @param prev_heap_end: End of previous 
 */
void sw_libc_sbrk(sw_uint *heap_start, sw_uint* heap_size, 
		sw_uint* prev_heap_end);

/**
 * @brief Update current end boundary of heap in parameter after sbrk
 *
 * @param current_libc_heap: Current libc heap boundary
 */

void sw_libc_sbrk_update(sw_uint current_libc_heap);
#endif

/**
 * @brief Print the task state
 * This function prints the current tasks state.
 */
void all_task_status(void);

sw_int load_libc_to_memory(void);
sw_int load_user_app_to_memory(void);

/* board.c */

/**
 * @brief Intiate tasks
 */
void run_init_tasks(void);

/* board_test.c */
/**
 * @brief Intiate test tasks
 */
void  run_init_test_tasks(void);

/**
 * @brief Extracts the next free asid value from list
 *
 * @return returns the asid value from list
 */
sw_short_int get_next_asid_val(void);

#endif /* __OTZ_GLOBAL_H__ */
