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
 *  Helper functions for Task management.
 */

#include <sw_types.h>
#include <task.h>
#include <sw_mem_functions.h>
#include <sw_string_functions.h>
#include <sw_debug.h>
#include <cpu_data.h>
#include <global.h>
#include <mem_mng.h>
#include <sw_user_mgmt.h>
#include <sw_link.h>
#include <sw_mmu_helper.h>
#include <sw_modinit.h>
#include <sw_heap.h>
#include <sw_addr_config.h>


static sw_uint* alloc_kernel_stack(sw_uint size);

static sw_int free_kernel_stack(sw_uint *sp_ptr, sw_uint size);

static sw_uint* alloc_user_stack(void *ptask);

static sw_int free_user_stack(void *ptask);

static sw_int sw_map_tls(void *tls);

static sw_int sw_unmap_tls(void *param);

static sw_tls* sw_alloc_tls(struct sw_task* task);

static sw_int sw_free_tls(struct sw_task *task);


/** @defgroup taskapi Task Management API
 *  APIs for creating, destroying and IPC APIs
 *  @{
 */

/**
 * @brief Create a task
 *
 * This function helps in task creation. It allocates the task structure, 
 * task local storage, task stack, task heap. 
 * It puts the task in suspend state and adds to global task list.
 *
 * Case 1 - No page table protection
 *
 * a. Kernel task - Map the kernel application code, data, bss, stack, tls 
 * and heap in kernel page table. Permission are based on the mapping area. 
 * e.g. text is mapped as read only, execute rights and data is mapped 
 * as read/write access.
 *
 * b. User task - Map the user application code, data, bss, library, tls 
 * and stack  in kernel page table with user read/write access.
 * 
 * Case 2 - Page table protection
 * Kernel task and user task - Map the application code, data, bss, stack, 
 * tls in task specific page table. 
 * Stack, Application and TLS are mapped in fixed virtual address. 
 * Heap is created from task entry point function to create the heap allocator 
 * to have the context variables in task page table.
 * 
 * Case 3: ELF Loader support
 * User task - It is applicable only for user task. 
 * In this case, map the application code, data, bss with user permissions. 
 * Please refer the above cases for with/without page table protection.
 *
 * @param psa_config: Configuration for task creation
 * @param task_id: Output parameter for the task ID.
 *
 * @return :
 * SW_OK - Task created successfully\n
 * SW_* - An implementation-defined error code for any other error.\n
 */
int create_task(sa_config_t *psa_config, int *task_id)
{
	int ret = SW_OK;
	struct sw_global *pglobal = &global_val;
	struct sw_task  *new_task = NULL;
	sw_uint cpsr,tmp_heap_size,tmp_min_alloc_size;
	sw_short_int map_tls = 0;

#ifdef CONFIG_SW_ELF_LOADER_SUPPORT
	sw_short_int map_lib_elf = 0;
#endif

	*task_id = -1;

	if(pglobal->exec_mode == 1 
			&& get_permission(psa_config->service_uuid) == -1){
		sw_printk("SW: IPC or Task creation denied.\n");
		sw_seterrno(SW_EACCES);
		ret = SW_ERROR;
		goto ret_func;
	}

	if(!psa_config->allow_multiple_instance
			&& check_multiple_instance(psa_config->service_uuid)) {
		sw_printk("SW: Creation of Multiple Instance Is denied.\n");
		sw_seterrno(SW_EACCES);
		ret = SW_ERROR;
		goto ret_func;
	}

	tmp_heap_size = psa_config->heap_size;
	tmp_min_alloc_size = psa_config->min_alloc_size;
	new_task = (struct sw_task  *)sw_malloc_private(COMMON_HEAP_ID,
			sizeof(struct sw_task));

	if(!new_task) {
		sw_printk("SW: task creation failed: malloc failed\n");
		sw_seterrno(SW_ENOMEM);
		ret = SW_ERROR;
		goto ret_func;
	}


	new_task->task_id = psa_config->task_id;
	new_task->service_id = psa_config->service_uuid;
	new_task->entry_addr = psa_config->entry_point;
	new_task->mode = psa_config->mode;
	new_task->guest_no = psa_config->guest_no;

	sw_strncpy(new_task->name, psa_config->service_name,NAME_LENGTH);

	new_task->elf_flag = psa_config->elf_flag;

	new_task->tls = (struct sw_tls  *)sw_alloc_tls(new_task);
	if(!new_task->tls) {
		sw_printk("SW: task creation failed: tls alloc failed for local storage\n");
		sw_seterrno(SW_ENOMEM);
		ret = SW_ERROR;
		goto handle_err1;
	}
	ret = sw_map_tls(new_task);
	if(ret == SW_ERROR){
		goto handle_err1;
	}
	map_tls = 1;

	((struct sw_tls*)new_task->tls)->private_data = psa_config->data;
	((struct sw_tls*)new_task->tls)->process = psa_config->process;
	((struct sw_tls*)new_task->tls)->heap_size = psa_config->heap_size;
	((struct sw_tls*)new_task->tls)->min_alloc_size = psa_config->min_alloc_size;
	((struct sw_tls*)new_task->tls)->task_id = new_task->task_id;			
	new_task->task_sp_size = psa_config->stack_size;
	new_task->task_sp = 0;

	if(new_task->mode == TASK_USER_MODE) {
		new_task->task_sp = alloc_user_stack(new_task); 
	}
	else {
		new_task->task_sp = alloc_kernel_stack(psa_config->stack_size);
	}

	if(!new_task->task_sp) {
		sw_printk("SW: task creation failed: stack allocation\n");
		sw_seterrno(SW_ENOMEM);
		ret = SW_ERROR;
		goto handle_err1;
	}

	if(new_task->mode == TASK_KERN_MODE) {
		ret = alloc_private_heap(psa_config->task_id, (int)tmp_heap_size,
				(int)tmp_min_alloc_size,&global_val.sw_mem_info,
				&new_task->tls->heap_info);
	}
	if(ret == SW_ERROR){
		sw_printk("SW: Private heap allocation failed \n");
		sw_seterrno(SW_ENOMEM);
		ret = SW_ERROR;
		goto handle_err1;

	}

	link_init(&new_task->head);
	link_init(&new_task->ready_head);
	link_init(&new_task->wait_head);
	link_init(&new_task->file_dev_list);
	link_init(&new_task->task_wait_list);

	RESET_BUSY_LOCK(&new_task->task_wait_lock);
	
	lock_irq_shared_resource(&pglobal->task_list_lock, &cpsr);
	set_link_data(&new_task->head, new_task);
	add_link(&pglobal->task_list, &new_task->head, TAIL);
	unlock_irq_shared_resource(&pglobal->task_list_lock, cpsr);

	new_task->state = TASK_STATE_SUSPEND;
	*task_id = new_task->task_id;

	new_task->acl.uid = psa_config->service_uuid;
	new_task->acl.gid = psa_config->gid;

	goto ret_func;

handle_err1:
	if(map_tls)
		sw_unmap_tls(new_task);

	if(new_task->task_sp) {
		if(new_task->mode == TASK_KERN_MODE) 
			free_kernel_stack(new_task->task_sp, new_task->task_sp_size);
		else
			free_user_stack(new_task);
	}

	if(psa_config->data)
		sw_free_private(COMMON_HEAP_ID, psa_config->data);

	if(new_task->tls)
		sw_free_tls(new_task);

	if(new_task)
		sw_free_private(COMMON_HEAP_ID, new_task);

	pglobal->task_id_pool[psa_config->task_id] = 0x0;

ret_func:
	return ret;
}

/**
 * @brief Destroy the created task
 *
 * This function cleans up the created task.
 * It frees the resources which got allocated in task creation.
 * It schedules the waiting tasks to start immediately after this task is destroyed.
 * It removes the task for global task list.
 * 
 * @param task_id: Task ID 
 *
 * @return :
 * SW_OK - Task destroyed successfully\n
 * SW_* - An implementation-defined error code for any other error.\n
 */
int destroy_task(int task_id)
{
	sw_int ret = SW_OK;
	struct link *l, *head;
	struct link *l1, *head1;
	struct link *l2, *head2, *temp_link;
	struct fd_link *fd_data;
	sw_bool found;
	struct sw_task  *task, *ready_task, *waiting_task;
	sw_uint elf_flag;
	struct sw_global *pglobal = &global_val;	
	sw_uint cpsr, cpsr2;

	task= NULL;
	found = FALSE;

	lock_irq_shared_resource(&global_val.task_list_lock, &cpsr);
	if (link_empty(&pglobal->task_list)) {
		sw_seterrno(SW_EINVAL);
		ret = SW_ERROR;
		goto destroy_task_ret;
	}
	task= NULL;
	found = FALSE;

	head= &pglobal->task_list;
	l = head->next;
	while (l != head) {
		task = l->data;
		if(task) {
			if (task->task_id == task_id) {
				found = TRUE;
				break;
			}
		}
		l = l->next;
	}

	if (!found) {
		sw_seterrno(SW_ENODATA);
		ret = SW_ERROR;
		goto destroy_task_ret;
	}

	if (!link_empty(&pglobal->ready_to_run_list)) {
		head1= &pglobal->ready_to_run_list;
		l1 = head1->next;
		while (l1 != head1) {
			ready_task = l1->data;
			if(ready_task) {
				if (ready_task->task_id == task_id) {
					remove_link(&ready_task->ready_head);
					ready_task->ready_head.data = 0;
					break;
				}
			}
			l1 = l1->next;
		}		
	}

	/* Schedule the tasks waiting for this task to complete */
	if(!link_empty(&task->task_wait_list)) {										/* Check if any tasks are waiting for the current task to finish */
		temp_link = &task->task_wait_list;
		do {																	/* Loop through waiting list */
			waiting_task = temp_link->data;
			if(waiting_task) {
				lock_irq_shared_resource(&global_val.ready_to_run_lock, &cpsr2);		
    		    task->state = TASK_STATE_READY_TO_RUN;
    		    schedule_task_next(waiting_task);									/* Schedule task to start immediately after this task */
    		    unlock_irq_shared_resource(&global_val.ready_to_run_lock, cpsr2);
			}
		}while((temp_link=get_next_node(temp_link)) && temp_link->data!=NULL);
	}

	/* Close any opened files or device instances of this task */
	if (!link_empty(&task->file_dev_list)) {
		sw_printk(
				"pending opened file/device for task id %d - Closed forcefully\n", 
				task->task_id);

		head2= &task->file_dev_list;
		l2 = head2->next;
		while (l2 != head2) {
			temp_link = l2->next;
			fd_data = (struct fd_link*)l2->data;
			if(fd_data) {
				if(fd_data->fd != -1) {
					close_device_file(fd_data);
				}
				remove_link(&fd_data->head);
				sw_free_private(COMMON_HEAP_ID, fd_data); 
			}			
			l2= temp_link ;
		}		
	}

	cpu_task_exit(task);

	if(task->mode == TASK_KERN_MODE) {
		ret = free_private_heap(task_id, &global_val.sw_mem_info);
		if(ret == SW_ERROR){
			sw_printk("Heap is not freed\n");
		}
	}
	else
	{
		ret = free_user_heap(task_id, &global_val.sw_mem_info);
		if(ret == SW_ERROR){
			sw_printk("Heap is not freed\n");
		}
	}


	if(task->tls){
		elf_flag = task->elf_flag;
		sw_unmap_tls(task);
		sw_free_tls(task);  
	}

	if(task->mode == TASK_USER_MODE)
		free_user_stack(task);
	else
		free_kernel_stack(task->task_sp, task->task_sp_size);	

	remove_link(&task->head);


	sw_free_private(COMMON_HEAP_ID, task);
	pglobal->task_id_pool[task_id] = 0x0;

	ret = SW_OK;
destroy_task_ret:
	unlock_irq_shared_resource(&global_val.task_list_lock, cpsr);
	return ret;
}

/**
 * @brief 
 *      This functions wakes up a task from sleep.
 *      It is used for async tasks
 * @param task_id
 *      The task to be resumed
 * @return 
 */
void resume_async_task(int task_id)
{
	struct sw_task* task = get_task(task_id);
	global_val.g_ns_notify_pending = 0;
	sw_wakeup(&task->notify_queue, WAKE_UP_IMMEDIATE);
}

/**
 * @brief Start the task
 *
 * This function gets called on command invocation and init the task context 
 * the schedule the task.
 *
 * @param task_id: Task ID
 * @param params: Command parameters
 *
 * @return:
 * SW_OK - Task started successfully\n
 * SW_* - An implementation-defined error code for any other error.\n
 */
int start_task(int task_id, sw_uint* params)
{
	int ret;
	struct link *l, *head;
	struct sw_task  *task;
	struct sw_global *pglobal = &global_val;
	sw_uint cpsr, cpsr1;

	sw_bool found;
	lock_irq_shared_resource(&global_val.task_list_lock, &cpsr1);
	if (link_empty(&pglobal->task_list)) {
		sw_seterrno(SW_EINVAL);
		ret = SW_ERROR;
		goto start_task_ret;
	}

	task= NULL;
	found = FALSE;

	head= &pglobal->task_list;
	l = head->next;
	while (l != head) {
		task = l->data;
		if(task) {
			if (task->task_id == task_id) {
				found = TRUE;
				break;
			}
		}
		l = l->next;
	}

	if (!found) {
		sw_seterrno(SW_ENODATA);
		ret = SW_ERROR;
		goto start_task_ret;
	}

	ret = cpu_task_init((void*)task);
	if(SW_OK != ret) 
		goto start_task_ret;

	if(params) {
		task->tls->params[0] = params[0];
		task->tls->params[1] = params[1];
		task->tls->params[2] = params[2];
		task->tls->params[3] = params[3];
	}
	else {
		sw_memset(task->tls->params, 0, sizeof(task->tls->params));
	}
	lock_irq_shared_resource(&global_val.ready_to_run_lock, &cpsr);
	schedule_task(task);
	unlock_irq_shared_resource(&global_val.ready_to_run_lock,cpsr);

	ret = SW_OK;
start_task_ret:
	unlock_irq_shared_resource(&global_val.task_list_lock, cpsr1);
	return ret;
}

/**
 * @brief Task context switch function
 *
 * This function switches the context between two tasks
 *
 * @param new_task: Task for which context need to be restored
 * @param old_task: Task for which context need to be saved
 */
void task_context_switch(struct sw_task  *new_task, struct sw_task  *old_task)
{
	if(old_task)
		save_task_context_regs(&old_task->regs);
	if(new_task)
		restore_task_context_regs(&new_task->regs);
}

/**
 * @brief Get task state
 *
 * @param task_id: Task ID
 *
 * @return Returns the task state: 
 * TASK_STATE_SUSPEND - Task in suspend state\n
 * TASK_STATE_WAIT - Task in wait state \n
 * TASK_STATE_READY_TO_RUN - Task is in ready to run state \n
 * TASK_STATE_RUNNING - Task is in running state \n
 */
int get_task_state(int task_id)
{
	struct link *l, *head;
	struct sw_task  *task;
	sw_bool found;
	struct sw_global *pglobal = &global_val;
	sw_uint cpsr, ret;

	lock_irq_shared_resource(&global_val.task_list_lock, &cpsr);
	if (link_empty(&pglobal->task_list)) {
		ret = TASK_STATE_SUSPEND; /* Fix the return code */
		goto get_task_state_ret;
	}

	task= NULL;
	found = FALSE;
	head= &pglobal->task_list;
	l = head->next;
	while (l != head) {
		task = l->data;
		if(task) {
			if (task->task_id == task_id) {
				found = TRUE;
				break;
			}
		}
		l = l->next;
	}

	if (!found) {
		ret = TASK_STATE_SUSPEND; /* Fix the return code */
		goto get_task_state_ret;
	}
	ret = task->state;
get_task_state_ret:
	unlock_irq_shared_resource(&global_val.task_list_lock, cpsr);
	return ret;
}

/**
 * @brief Helper function to print the task context
 *
 * @param task_id: Task ID
 *
 * @return :
 * SW_OK - Printed successfully\n
 * SW_* - An implementation-defined error code for any other error.\n
 */
int print_task(int task_id)
{
	struct link *l, *head;
	struct sw_task  *task;
	struct sw_global *pglobal = &global_val;

	sw_bool found;
	if (link_empty(&pglobal->task_list)) {
		sw_seterrno(SW_ENODATA);
		return SW_ERROR;
	}

	task= NULL;
	found = FALSE;

	head= &pglobal->task_list;
	l = head->next;
	while (l != head) {
		task = l->data;
		if(task) {
			if (task->task_id == task_id) {
				found = TRUE;
				break;
			}
		}
		l = l->next;
	}


	if (!found) {
		sw_seterrno(SW_ENODATA);
		return SW_ERROR;
	}
	print_cpu_task_regs(task);
	return SW_OK;
}


/**
 * @brief Get task local storage
 *
 * This helper function returns the task local storage
 *
 * @param task_id: Task ID
 *
 * @return Returns the task local storage pointer or NULL.
 */
sw_tls* get_task_tls(int task_id)
{
	struct link *l, *head;
	struct sw_task  *task;
	struct sw_global *pglobal = &global_val;
	sw_tls *tls;
	sw_uint cpsr;
	sw_bool found;

	lock_irq_shared_resource(&global_val.task_list_lock, &cpsr);

	if (link_empty(&pglobal->task_list)) {
		tls = NULL;
		goto get_task_tls_ret;
	}

	task= NULL;
	found = FALSE;
	head= &pglobal->task_list;
	l = head->next;
	while (l != head) {
		task = l->data;
		if(task) {
			if (task->task_id == task_id) {
				found = TRUE;
				break;
			}
		}
		l = l->next;
	}

	if (!found) {
		tls = NULL;
		goto get_task_tls_ret;
	}

	tls = task->tls;
get_task_tls_ret:
	unlock_irq_shared_resource(&global_val.task_list_lock, cpsr);
	return tls;
}
/**
 * @brief Helper function to return task structure
 *
 * This helper function returns the task structure for the given task ID
 * 
 * @param task_id: Task ID
 *
 * @return Returns the pointer to task structure or NULL
 */
struct sw_task* get_task(int task_id)
{
	struct link *l, *head;
	struct sw_task  *task;
	struct sw_global *pglobal = &global_val;
	sw_uint cpsr;
	sw_bool found;

	lock_irq_shared_resource(&global_val.task_list_lock, &cpsr);

	if (link_empty(&pglobal->task_list)) {
		task = NULL;
		goto get_task_ret;
	}

	task= NULL;
	found = FALSE;
	head= &pglobal->task_list;
	l = head->next;
	while (l != head) {
		task = l->data;
		if(task) {
			if (task->task_id == task_id) {
				found = TRUE;
				break;
			}
		}
		l = l->next;
	}

	if (!found) {
		task = NULL;
		goto get_task_ret;
	}

get_task_ret:
	unlock_irq_shared_resource(&global_val.task_list_lock, cpsr);
	return task;
}

/**
 * @brief Kernel stack allocation routine
 *	This function allocates the required stack size + PAGE SIZE and 
 *	map only the bottom part to the secure page table. 
 *	This is used to detect the stack overflow condition.
 * @param size: Kernel task stack size
 *
 * @return 
 * SW_OK - Kernel stack allocated successfully\n
 * SW_* - An implementation-defined error code for any other error.\n
 */
sw_uint* alloc_kernel_stack(sw_uint size)
{
	sw_uint* sp_ptr = NULL;	
	sw_phy_addr phy_addr = 0, new_phy_addr = 0;
	sw_vir_addr vir_addr = 0, new_vir_addr = 0;
	sw_uint new_size;

	if(size & (PAGE_SIZE - 1)) 	
		size = (size & (~(PAGE_SIZE -1))) + PAGE_SIZE;

	new_size = size + PAGE_SIZE;

	phy_addr = sw_phy_page_alloc(new_size, &global_val.sw_mem_info);
	if(phy_addr == 0) {
		sw_printk("Physical page allocation failed for kernel stack\n");
		goto alloc_kernel_stack_ret;
	}

	vir_addr = secure_phy_to_vir(phy_addr);
	if(sw_vir_addr_reserve(vir_addr,new_size, &global_val.sw_mem_info) != SW_OK) {
		vir_addr = NULL;
		sw_printk("Virtual reservation failed for kernel stack\n");
		goto alloc_kernel_stack_ret;

	}

	/* If you increment it using pointer, 
	   it goes up by 4 for every one increment */
	new_vir_addr = (sw_vir_addr)((sw_uint)vir_addr + PAGE_SIZE); 
	new_phy_addr = phy_addr + PAGE_SIZE;
	if(map_kernel_data_memory(new_vir_addr, new_phy_addr, size) == SW_OK) {
		sp_ptr = (sw_uint*)new_vir_addr;
	}
alloc_kernel_stack_ret:
	if(!sp_ptr) {
		if(vir_addr) {
			sw_vir_free_reserve(vir_addr, new_size, &global_val.sw_mem_info);
		}
		if(phy_addr) {
			sw_phy_addr_free(phy_addr, new_size, &global_val.sw_mem_info);
		}
	}
	return sp_ptr;	
}

/**
 * @brief Freeing the stack allocated for kernel task
 *	This function frees the previously allocated memory and 
 * 	unmap the entry from secure page table.
 * @param sp_ptr: Pointer to the kernel task stack
 * @param size: Stack size of the kernel task.
 *
 * @return 
 * SW_OK - Stack freed successfully\n
 * SW_* - An implementation-defined error code for any other error.\n
 */
sw_int free_kernel_stack(sw_uint *sp_ptr, sw_uint size)
{
	sw_uint phy_addr = 0;
	sw_uint vir_addr = 0;
	sw_uint new_size;

	if(size & (PAGE_SIZE - 1)) 	
		size = (size & (~(PAGE_SIZE -1))) + PAGE_SIZE;

	new_size = size + PAGE_SIZE;

	vir_addr = (sw_uint)sp_ptr;
	phy_addr = secure_vir_to_phy(vir_addr);		

	vir_addr -= PAGE_SIZE;
	phy_addr -= PAGE_SIZE;

	if(unmap_kernel_memory((sw_vir_addr)sp_ptr, size) != SW_OK) {
		sw_printk("kernel stack 0x%x unmap failed\n", (sw_uint)sp_ptr);
	}

	if(sw_vir_free_reserve(vir_addr, new_size, &global_val.sw_mem_info) != SW_OK) {
		sw_printk("kernel stack 0x%x freeing virtual page reservation failed\n", 
				(sw_uint)sp_ptr);
	}

	if(sw_phy_addr_free(phy_addr, new_size, &global_val.sw_mem_info) != SW_OK) {
		sw_printk("kernel stack 0x%x freeing physical page 0x%x failed\n", 
				(sw_uint)sp_ptr, phy_addr);
	}
	return SW_OK;
}

/**
 * @brief User stack allocation routine
 *	This function allocates the required stack size + PAGE SIZE and 
 *	map only the bottom part to the page table. 
 *	This is used to detect the stack overflow condition.
 * @param param: Pointer to task 
 *
 * @return 
 * SW_OK - User stack allocated successfully\n
 * SW_* - An implementation-defined error code for any other error.\n
 */
sw_uint* alloc_user_stack(void *param)
{
	struct sw_task *task;
	sw_uint* sp_ptr = NULL;	
	sw_phy_addr phy_addr;
	sw_vir_addr vir_addr = 0, vir_addr_user;
	sw_uint new_size, size;
	sw_phy_addr pt;

	task = (struct sw_task*)param;
	size = task->task_sp_size;
	if(size & (PAGE_SIZE - 1)) 	
		size = (size & (~(PAGE_SIZE -1))) + PAGE_SIZE;

	new_size = size + PAGE_SIZE;

	phy_addr = sw_phy_page_alloc(new_size, &global_val.sw_mem_info);
	if(phy_addr == 0) {
		sw_printk("Physical page allocation failed for user stack\n");
		goto alloc_user_stack_ret;
	}

	vir_addr = (sw_vir_addr) secure_phy_to_vir(phy_addr);
	if(sw_vir_addr_reserve(vir_addr,new_size, &global_val.sw_mem_info) != SW_OK) {
		vir_addr = 0;
		sw_printk("Virtual reservation failed for usr stack\n");
		goto alloc_user_stack_ret;

	}

	task->user_info.sp_phy_addr = phy_addr;
	task->user_info.sp_kern_addr = vir_addr;

	/* If you increment it using pointer, 
	   it goes up by 4 for every one increment */
	phy_addr += PAGE_SIZE;

	vir_addr_user = vir_addr;
	pt = (sw_phy_addr)get_secure_ptd();
	if(task->mode == TASK_USER_MODE) {
		if(map_user_data_memory(vir_addr_user, phy_addr, size, 
					pt) == SW_OK) {
			sp_ptr = (sw_uint*) (vir_addr_user);
		}
	}
	else {
		if(map_user_memory(vir_addr_user, phy_addr, size, PTF_PROT_KRW, 
					pt) == SW_OK) {
			sp_ptr = (sw_uint*) (vir_addr_user);
		}
	}

alloc_user_stack_ret:
	if(!sp_ptr) {
		if(vir_addr) {
			sw_vir_free_reserve(vir_addr, new_size, &global_val.sw_mem_info);
		}
		if(phy_addr) {
			sw_phy_addr_free(phy_addr, new_size, &global_val.sw_mem_info);
		}
	}
	return sp_ptr;	
}

/**
 * @brief Freeing the stack allocated for user task
 *	This function frees the previously allocated memory and 
 * 	unmap the entry from page table.
 * @param param: Pointer to the task
 * @return 
 * @return 
 * SW_OK - User stack freed successfully\n
 * SW_* - An implementation-defined error code for any other error.\n
 */
static sw_int free_user_stack(void *param)
{
	struct sw_task *ptask;
	sw_phy_addr phy_addr = 0, new_phy_addr;
	sw_vir_addr kern_vir_addr = 0;
	sw_uint new_size, size;
	sw_phy_addr pt;

	ptask = (struct sw_task *)param;
	size = ptask->task_sp_size;

	if(size & (PAGE_SIZE - 1)) 	
		size = (size & (~(PAGE_SIZE -1))) + PAGE_SIZE;

	new_size = size + PAGE_SIZE;

	kern_vir_addr = (sw_vir_addr)ptask->user_info.sp_kern_addr;
	phy_addr = (sw_vir_addr)ptask->user_info.sp_phy_addr;	

	pt = (sw_phy_addr)get_secure_ptd();

	new_phy_addr = phy_addr - PAGE_SIZE;

	if(unmap_user_memory((sw_vir_addr)ptask->task_sp, size, 
				pt) != SW_OK) {
		sw_printk("user stack 0x%x unmap failed\n", (sw_uint)ptask->task_sp);
	}

	if(sw_vir_free_reserve(kern_vir_addr, new_size, &global_val.sw_mem_info)
			!= SW_OK) {
		sw_printk("user stack 0x%x freeing virtual page reservation failed\n", 
				(sw_uint)ptask->task_sp);
	}

	if(sw_phy_addr_free(phy_addr, new_size, &global_val.sw_mem_info) != SW_OK) {
		sw_printk("user stack 0x%x freeing physical pagefailed\n", 
				(sw_uint)ptask->task_sp);
	}
	return SW_OK;
}

/**
 * @brief Task TLS allocation routine
 *	This function allocates the PAGE SIZE
 *
 * SW_OK - TLS allocated successfully\n
 * SW_* - An implementation-defined error code for any other error.\n
 */
static sw_tls* sw_alloc_tls(struct sw_task* task)
{
	sw_tls* tls = NULL;	
	sw_phy_addr phy_addr = 0;
	sw_vir_addr vir_addr = 0;

	phy_addr = sw_phy_page_alloc(PAGE_SIZE, &global_val.sw_mem_info);
	if(phy_addr == 0) {
		sw_printk("Physical page allocation failed for TLS\n");
		goto sw_alloc_tls_ret;
	}

	vir_addr = (sw_vir_addr) secure_phy_to_vir(phy_addr);
	if(sw_vir_addr_reserve(vir_addr,PAGE_SIZE, &global_val.sw_mem_info) != SW_OK) {
		vir_addr = 0;
		sw_printk("Virtual reservation failed for TLS\n");
		goto sw_alloc_tls_ret;

	}

	tls = (sw_tls*)vir_addr;
	task->tls_phy_addr = phy_addr;
sw_alloc_tls_ret:
	return tls;	
}

/**
 * @brief Task TLS Free routine
 *	This function de-allocates the TLS data
 *
 * SW_OK - TLS freed successfully\n
 * SW_* - An implementation-defined error code for any other error.\n
 */
static sw_int sw_free_tls(struct sw_task *task)
{
	sw_int ret_val = SW_OK;
	sw_phy_addr phy_addr;

	phy_addr = task->tls_phy_addr;

	if(sw_vir_free_reserve((sw_vir_addr)task->tls, PAGE_SIZE, &global_val.sw_mem_info) != SW_OK) {
		sw_printk("TLS 0x%x freeing virtual page reservation failed\n", 
				(sw_uint)task->tls);
	}

	if(sw_phy_addr_free(phy_addr, PAGE_SIZE, &global_val.sw_mem_info) != SW_OK) {
		sw_printk("TLS 0x%x freeing physical pagefailed\n", 
				(sw_uint)task->tls);
	}

	return ret_val;	
}

/**
 * @brief TLS mapped to user memory on secure or task specific page table
 *
 * @param param: Pointer to task 
 *
 * SW_OK - TLS mapped successfully\n
 * SW_* - An implementation-defined error code for any other error.\n
 */
static sw_int sw_map_tls(void *param)
{
	struct sw_task *task;
	sw_int ret_val = SW_OK;
	sw_phy_addr tls_phy_addr;
	sw_vir_addr map_addr;
	sw_phy_addr pt;

	task = (struct sw_task*)param;
	tls_phy_addr = task->tls_phy_addr;


	map_addr = (sw_vir_addr)task->tls;
	pt = (sw_phy_addr)get_secure_ptd();


	if(pt != 0) {
		if(task->mode == TASK_USER_MODE) {
			if(map_user_data_memory(map_addr, tls_phy_addr, PAGE_SIZE,
						pt) != SW_OK) {
				sw_printk("mapping to user memory failed for tls page\n");
				ret_val = SW_ERROR;
				goto tls_to_usr_ret;
			}
		}
		else {
			if(map_user_memory(map_addr, tls_phy_addr, PAGE_SIZE, PTF_PROT_KRW, 
						pt) != SW_OK) {
				sw_printk("mapping to user memory failed for tls page\n");
				ret_val = SW_ERROR;
				goto tls_to_usr_ret;
			}
		}
		sw_memset(task->tls, 0, sizeof(sw_tls));		
	}
	else {
		sw_printk("invalid user page table\n");
		ret_val = SW_ERROR;
		sw_seterrno(SW_EINVAL);
	}	
tls_to_usr_ret:	
	return ret_val;
}


/**
 * @brief Unmap the tls from task page table
 *
 * @param param: Pointer to structure
 *
 * SW_OK - TLS unmapped successfully\n
 * SW_* - An implementation-defined error code for any other error.\n
 */
static sw_int sw_unmap_tls(void *param)
{
	struct sw_task *task;
	sw_int ret_val = SW_OK;
	sw_vir_addr map_addr;
	sw_phy_addr pt;

	task = (struct sw_task*)param;

	map_addr = (sw_vir_addr)task->tls;
	pt = (sw_phy_addr)get_secure_ptd();	

	if(pt != 0) {
		if(unmap_user_memory(map_addr, PAGE_SIZE,
					pt) != SW_OK) {
			sw_printk("unmapping from user page table failed for tls page\n");
			ret_val = SW_ERROR;
		}
	}
	else {
		sw_printk("invalid user page table\n");
		ret_val = SW_ERROR;
		sw_seterrno(SW_EINVAL);
	}	
	return ret_val;
}

/**
 * @brief	Find out Instance already created or not
 * 			for current running Application service
 *          
 * @param service_id service_id of the current running
 * 						Application service
 *
 * @return returns 1 if Application service is already running.
 * 					Otherwise 0
 */
int check_multiple_instance(int service_id) {
	struct link *l, *head;
	struct sw_task *task;
	sw_uint cpsr;
	int found = 0;

	lock_irq_shared_resource(&global_val.task_list_lock, &cpsr);

	l = head = &global_val.task_list;
	l = head->next;
	while (l != head) {
		task = l->data;
		if(task->service_id == service_id) {
			found = 1;
			sw_printk("Task is already run\r\n");
			goto check_multiple_instance;
		}
		l = l->next;
	}
check_multiple_instance:
	unlock_irq_shared_resource(&global_val.task_list_lock, cpsr);
	return found;
}

/**
* @brief Add a task to the waiting list of another task.
*
* This implies that a task is waiting for another task to complete before it can start again.
* The waiting tasks will be scheduled to start at the destruction(destroy_task()) of the running task.
*
* @param task: The task to whose list the waiting task is added
* @param tmp_task: The task to be added to the list
*
* @return 
*/
void add_to_wait_list(struct sw_task *task, struct sw_task *tmp_task) {
	sw_uint cpsr;
	
	if(!task || !tmp_task) {
		sw_printf("Invalid task. Cannot add to wait list.\n");
		return;
		}
	lock_irq_shared_resource(&task->task_wait_lock, &cpsr);
	set_link_data(&task->wait_head, tmp_task);
	add_link(&task->task_wait_list, &task->wait_head, HEAD);
	unlock_irq_shared_resource(&task->task_wait_lock, cpsr);
	}
/** @} */ // end of taskapi
