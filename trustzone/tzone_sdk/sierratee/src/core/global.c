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
 * Global variables handling functions
 */

#include <sw_types.h>
#include <task.h>
#include <sw_debug.h>
#include <cpu_data.h>
#include <cpu.h>
#include <sw_mem_functions.h>
#include <sw_timer.h>
#include <mem_mng.h>
#include <sw_board.h>
#include <sw_heap.h>
#include <secure_api.h>
#include <sw_addr_config.h>
#include <tzhyp.h>
#include <tzhyp_global.h>
#include <tzhyp_config.h>
#include <system_context.h>

/**
 * @brief TEE global instance variable
 */
struct sw_global global_val;
/**
 * @brief Global initialization
 * 
 * This function initializes the global variables structure
 */
void global_init(void)
{
#ifdef NEWLIB_SUPPORT
	sw_phy_addr libc_phy_addr;
	sw_phy_addr libc_va_addr;
#endif

	sw_uint i = 1;
	global_val.linux_return_flag = 0;
	global_val.linux_task_id = 0;
	global_val.g_dispatch_data = NULL;
	global_val.g_ns_notify_pending = 0;

	link_init(&global_val.task_list);	
	link_init(&global_val.ready_to_run_list);
	link_init(&global_val.page_ref_list);
	link_init(&global_val.file_dev_list);	
	link_init(&global_val.shared_memory_ref);
	link_init(&global_val.semaphore_list);
	link_init(&global_val.mutex_list);

	RESET_BUSY_LOCK(&global_val.ready_to_run_lock);
	RESET_BUSY_LOCK(&global_val.task_list_lock);
	link_init(&global_val.device_acl_list);

	global_val.current_task_id = 0;
	global_val.exec_mode = 0;

#ifdef NEWLIB_SUPPORT
	global_val.libc_heap_size = LIBC_HEAP_SIZE;
#endif

	sw_memset(&global_val.shm_id_pool, 0, sizeof(sw_short_int) *
			SHARED_MEM_INSTANCE);
	sw_memset(&global_val.task_id_pool, 0, sizeof(sw_short_int) * MAX_ASID);
	RESET_BUSY_LOCK(&global_val.semaphore_list_lock);
	RESET_BUSY_LOCK(&global_val.shared_memory_ref_lock);
	RESET_BUSY_LOCK(&global_val.mutex_list_lock);
	while(i <= TASK_ID_START - 1)
		global_val.task_id_pool[i++] = 0x1;
	sw_memset(params_stack, 0, sizeof(params_stack));
	if(alloc_private_heap(COMMON_HEAP_ID,COMMON_HEAP_SIZE, 
				MIN_ALLOC_SIZE,&global_val.sw_mem_info, &global_val.heap_info) == -1){
		tee_panic("heap allocation failed\n");
	}

#ifdef NEWLIB_SUPPORT	
	if(global_val.libc_heap_size > 0) {
		libc_phy_addr = sw_phy_page_alloc(global_val.libc_heap_size, &global_val.sw_mem_info);
		if(libc_phy_addr == 0) {
			tee_panic("Physical page allocation failed for libc heap\n");
			return;
		}

		libc_va_addr = (sw_vir_addr) secure_phy_to_vir(libc_phy_addr);
		if(sw_vir_addr_reserve(libc_va_addr,global_val.libc_heap_size, 
					&global_val.sw_mem_info) != SW_OK) {
			tee_panic("Virtual reservation failed for libc heap\n");
			return;
		}

		if(map_user_data_memory(LIBC_HEAP_VA, libc_phy_addr, 
					global_val.libc_heap_size, 
					(sw_phy_addr)get_secure_ptd()) != SW_OK) {
			tee_panic("Mapping failed for libc heap\n");
			return;
		}
		global_val.libc_heap_start = LIBC_HEAP_VA;
	}
	else {
		global_val.libc_heap_start = NULL;
	}
	global_val.libc_heap_current = 0;
#endif

	global_val.heap_init_done = 1;
	global_val.tzhyp_val.ns_world = sw_malloc(MAX_CORES * GUESTS_NO
			* sizeof(struct system_context));
	global_val.tzhyp_val.s_world = sw_malloc(MAX_CORES
			* sizeof(struct system_context));
}

/**
 * @brief 
 *
 * This function initializes the structure used in gui manager
 *
 * @return SW_OK if success
 * 		   SW_ERROR if failure
 */
sw_int gui_context_init()
{
	return SW_ERROR;
}

/**
 * @brief 
 *
 * This function frees up the memory used for the structure gui_info
 *
 */
void gui_context_exit()
{
}

/**
 * @brief 
 *
 * This function returns the initialized gui structure pointer
 *
 * @return 
 */
struct gui_info *get_gui_info()
{
	return NULL;
}

/**
 * @brief Schedules the task 
 *
 * This function implements the functionality of scheduling the given task
 *
 * @param task: Pointer to task structure
 */
void schedule_task(struct sw_task* task)
{
	struct link *l, *head;
	struct sw_task *temp_task;

	head= &global_val.ready_to_run_list;
	l = head->next;
	while (l != head) {
		temp_task = l->data;
		if(temp_task) {
			if(temp_task->task_id == task->task_id)
				return;
		}
		l = l->next;
	}
	set_link_data(&task->ready_head, task);
	add_link(&global_val.ready_to_run_list, &task->ready_head, TAIL);
}

/**
 * @brief 
 *      This function adds the task to the ready to run list as the first
 *      element so that the next task scheduled will be this
 * @param task
 *      Pointer to the Next structure
 */
void schedule_task_next(struct sw_task* task)
{
	set_link_data(&task->ready_head, task);
	add_link(&global_val.ready_to_run_list, &task->ready_head, HEAD);
}

/**
 * @brief Enqueue in scheudler ready queue list
 *
 * @param task_id: Task ID to be added in the ready list
 */
void schedule_task_id(int task_id)
{
	struct sw_task* task;
	sw_uint cpsr;

	task = get_task(task_id);
	if(task){

		lock_irq_shared_resource(&global_val.ready_to_run_lock, &cpsr);
		task->state = TASK_STATE_READY_TO_RUN;
		schedule_task(task);
		unlock_irq_shared_resource(&global_val.ready_to_run_lock, cpsr);

	}
	return;
}

/**
 * @brief Suspends the task 
 *
 * This function suspend the given task
 *
 * @param task_id: Task ID
 * @param state: State of the task
 */
void suspend_task(int task_id, int state)
{
	struct link *l, *head;
	struct sw_task  *task = NULL;
	sw_bool found;
	sw_uint cpsr;

	lock_irq_shared_resource(&global_val.task_list_lock, &cpsr);

	if (link_empty(&global_val.task_list)) {
		goto suspend_task_ret; 
	}

	found = FALSE;

	head= &global_val.task_list;
	l = head->next;
	while (l != head) {
		task = l->data;
		if(task) {
			if(task->task_id == task_id) {
				found = TRUE;
				break;
			}
		}
		l = l->next;
	}

	if(found) {
		task->state = state;

		if(global_val.current_task_id == task_id && 
				task->state == TASK_STATE_SUSPEND)
			global_val.current_task_id = -1;
	}
suspend_task_ret:
	unlock_irq_shared_resource(&global_val.task_list_lock, cpsr);
	return;
}

/**
 * @brief Update the current task ID
 *
 * @param task: Pointer to the task structure
 */
void update_current_task(struct sw_task* task)
{
	if(task != NULL)
		global_val.current_task_id = task->task_id;
}

/**
 * @brief 
 *      Returns the id of the current running task
 *
 * @return
 *      task id
 */
int get_current_task_id(void)
{
	return global_val.current_task_id;
}
/**
 * @brief Get the current task
 *
 * This function returns the current task which is running
 *
 * @return : Pointer to the task structure or NULL
 */
struct sw_task* get_current_task(void)
{
	struct link *l, *head;
	struct sw_task  *task = NULL;
	sw_uint cpsr;
	sw_bool found;

	lock_irq_shared_resource(&global_val.task_list_lock, &cpsr);
	if (link_empty(&global_val.task_list)) {
		goto get_current_task_ret;
	}

	found = FALSE;

	l = head= &global_val.task_list;
	l = head->next;
	while (l != head) {
		task = l->data;
		if(task) {
			if(task->task_id == global_val.current_task_id) {
				found = TRUE;
				break;
			}
		}
		l = l->next;
	}

	if(!found)
		task = NULL;

get_current_task_ret:
	unlock_irq_shared_resource(&global_val.task_list_lock, cpsr);
	return task;

}
/**
 * @brief Get the next ready to run task
 *
 * This function returns the next task which is ready to run
 *
 * @return : Pointer to the task structure or NULL
 */
struct sw_task* get_next_task(void)
{
	struct link *l, *head;
	struct sw_task  *current_task = get_current_task();
	struct sw_task  *next_task;
	struct sw_task  *temp_task;
	sw_uint cpsr;

	lock_irq_shared_resource(&global_val.ready_to_run_lock, &cpsr);

	/* No pending tasks in ready to run list */
	if (link_empty(&global_val.ready_to_run_list)) {
		if(current_task != NULL && current_task->state != TASK_STATE_WAIT && 
				current_task->state != TASK_STATE_SUSPEND 
#ifdef CONFIG_MULTI_GUESTS_SUPPORT				
				&& (current_task->guest_no == -1 || 
					current_task->guest_no == get_current_guest_no() )
#endif	
		  ){
			next_task = current_task;
			goto func_return;
		}
		else{
			next_task = NULL;
			goto func_return;
		}
	}

	next_task = NULL;
	head= &global_val.ready_to_run_list;
	l = head->next;
	if (l != head) {
		temp_task = l->data;
#ifdef CONFIG_MULTI_GUESTS_SUPPORT		
		if(temp_task->guest_no == -1 ||
				(temp_task->guest_no == get_current_guest_no())) {
#endif		
			remove_link(l);
			next_task = temp_task;
#ifdef CONFIG_MULTI_GUESTS_SUPPORT		
		}
#endif		
	}

	if(current_task) {
		head= &global_val.ready_to_run_list;
		l = head->next;
		while (l != head) {
			temp_task = l->data;
			if(temp_task) {
				if(temp_task->task_id == current_task->task_id)
					goto func_return;
			}
			l = l->next;
		}
	}

	if(current_task != NULL && current_task->state != TASK_STATE_WAIT && 
			current_task->state != TASK_STATE_SUSPEND
#ifdef CONFIG_MULTI_GUESTS_SUPPORT				
			&& (current_task->guest_no == -1 || 
				current_task->guest_no == get_current_guest_no() )
#endif	
	  ){

		schedule_task(current_task);        
	}

func_return:
	unlock_irq_shared_resource(&global_val.ready_to_run_lock, cpsr);
	return next_task;
}

/**
 * @brief Prints the all task names
 */
void print_task_list(void)
{
	struct link *l, *head;
	struct sw_task  *task;

	if (link_empty(&global_val.task_list)) {
		return;
	}

	l = head= &global_val.task_list;
	l = head->next;
	while (l != head) {
		task = l->data;
		if(task) {
			sw_printk("SW: task name %s\n", task->name);
		}
		l = l->next;
	}	

	return;
}


/**
 * @brief Register IRQ handler for the specificed interrupt ID 
 *
 * This function register the IRQ handler for the specified interrupt ID. This
 * could be a function from any task.
 *
 * @param interrupt: Interrupt ID
 * @param handler: Function pointer to IRQ handler
 * @param data: IRQ handler data
 */
void register_secure_irq_handler(sw_uint interrupt, irq_handler handler, void *data)
{
	if (interrupt > NO_OF_INTERRUPTS_IMPLEMENTED) {
		return;
	}
	global_val.task_irq_handler[interrupt] = handler;
	global_val.task_irq_handler_data[interrupt] = data;
}

/**
 * @brief Invoke the registered IRQ handler of specified interrupt number
 *
 * This function invokes the corresponding registered IRQ handler of the
 * specified interrupt ID.
 *
 * @param interrupt: Interrupt ID
 * @param regs: Context registers
 */
void invoke_irq_handler(sw_uint interrupt, struct swi_temp_regs *regs)
{
	if(global_val.task_irq_handler[interrupt]) {
		global_val.task_irq_handler[interrupt](interrupt, 
				global_val.task_irq_handler_data[interrupt]);
	}
}

/**
 * @brief 
 *
 * @param interrupt
 *
 * @return 
 */
void* get_interrupt_data(sw_uint interrupt)
{
	return global_val.task_irq_handler_data[interrupt];
}

#ifdef NEWLIB_SUPPORT	
/**
 * @brief Populate libc heap boundary information in parameters for sbrk
 *
 * @param heap_start: Starting address of heap
 * @param heap_size: Size of heap
 * @param prev_heap_end: End of previous 
 */
void sw_libc_sbrk(sw_uint *heap_start, sw_uint* heap_size, 
		sw_uint* prev_heap_end)
{
	if(global_val.libc_heap_start == NULL) {
		*heap_start = 0;
		*heap_size = 0;
		return;
	}

	*heap_start = (sw_uint)global_val.libc_heap_start;
	*heap_size = global_val.libc_heap_size;	
	*prev_heap_end = global_val.libc_heap_current;
}

/**
 * @brief Update current end boundary of heap in parameter after sbrk
 *
 * @param current_libc_heap: Current libc heap boundary
 */
void sw_libc_sbrk_update(sw_uint current_libc_heap)
{
	global_val.libc_heap_current = current_libc_heap;
}

#endif

/**
 * @brief Print the task state
 * This function prints the current tasks state.
 */
void all_task_status(void)
{
	struct link *l, *head;
	struct sw_task *task;
	sw_uint cpsr;

	lock_irq_shared_resource(&global_val.task_list_lock, &cpsr);
	if (link_empty(&global_val.task_list)) {
		sw_printk("Task list is empty\r\n");
		goto all_task_status_ret;
	}

	l = head = &global_val.task_list;
	l = head->next;
	while (l != head) {
		task = l->data;
		if(task) {
			sw_printk("%d\t%d\t   ", task->task_id, task->service_id);
			if(task->state == TASK_STATE_SUSPEND)
				sw_printk("Suspended\t");
			else if(task->state == TASK_STATE_WAIT)
				sw_printk("Waiting\t");
			else if(task->state == TASK_STATE_READY_TO_RUN)
				sw_printk("Read to run\t");
			else
				sw_printk("Running\t");
			sw_printk("%s\r\n",task->name);
		}
		l = l->next;
	}

all_task_status_ret:
	unlock_irq_shared_resource(&global_val.task_list_lock, cpsr);
	return;
	sw_printk("Not implemented\r\n");
	return;
}

/**
 * @brief Extracts the next free asid value from list
 *
 * @return returns the asid value from list
 */
sw_short_int get_next_asid_val(void) {
	sw_short_int asid = TASK_ID_START;
	sw_uint cpsr;

	lock_irq_shared_resource(&global_val.task_list_lock, &cpsr);
	while(global_val.task_id_pool[asid] != 0x0) {
		asid++;
		if(asid > MAX_ASID)
			asid = TASK_ID_START;
	}
	global_val.task_id_pool[asid] = 0x1;
	unlock_irq_shared_resource(&global_val.task_list_lock, cpsr);

	return asid;
}

