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
 *  Helper function declarations of task management
 */

#ifndef __OTZ_TASK_H__
#define __OTZ_TASK_H__

#include <sw_types.h>
#include <sw_link.h>
#include <global.h>
#include <cpu_task.h>
#include <sw_wait.h>
#include <sw_buddy.h>
#include <tls.h>

#define NAME_LENGTH 32
#define SERVICE_NAME_LEN 32
#define FILE_PATH_LEN 255
#define PROCESS_NAME_LEN 33
#define ENTRY_FUNC_LEN 32

/** @addtogroup taskapi Task Management API
 *  APIs for creating, destroying and IPC APIs
 *  @{
 */

/**
 * @brief Elf loader information for task 
 */
struct elf_info
{
	sw_uint base_va;
	sw_uint base_pa;
	sw_uint total_alloc_sz;
	sw_uint map_pa;
	sw_uint map_va;
};


/**
 * @brief Task information to maintain task page table and stack
 */
struct usr_task_info
{

	/*! task usr stack physical addr !*/
	sw_phy_addr sp_phy_addr;
	/*! task usr stack kernel virtual addr !*/
	sw_vir_addr sp_kern_addr;
};


/**
 * @brief
 Secure API configuration details for task
 */
typedef struct sa_config_t
{
	/*! guest no */
	int       guest_no;
	/*! Service UUID */
	int       service_uuid;
	/*! Task id !*/
	sw_uint task_id;
	/*! Service Name */
	char      service_name[SERVICE_NAME_LEN];
	/*! Stack size of the task */
	sw_uint       stack_size;
	/*! Heap size of the task */
	sw_uint       heap_size;
	/*! Min_alloc_size of the task */
	sw_uint		  min_alloc_size;
	/*! Mode in which the task runs  */
	sw_uint       mode;
	/*  Name of the entry function */
	char entry_func[ENTRY_FUNC_LEN];
	/*! Entry point for the task */
	sw_uint       entry_point;
#ifdef CONFIG_SW_LIB_SUPPORT
	sw_uint lib_enabled;
#endif 
	/*! flag to indicate cleanup and the presence of loader support */
	sw_uint elf_flag;
	/*        */
	/*! file path*/
	char file_path[FILE_PATH_LEN];
	/*! process name */
	char process_name[PROCESS_NAME_LEN];
	/*! service func pointer */
	process_fn process;
	/*! Task data */
	void*     data;
	/*!Allow_multiple_instance */
	sw_uint allow_multiple_instance;
	/*! Task Group ID */
	sw_uint gid;
}sa_config_t;


/**
 * @brief Task type 
 * Task type specifies the kernel or user mode task. 
 */
enum task_mode {
	TASK_USER_MODE = 0,
	TASK_KERN_MODE
}; 

/**
 * @brief User access control information
 */
typedef struct user_access_control{
	/*! Task name */
	char *username; 
	/*! User ID */
	sw_uint uid; 
	/*! Task Group ID */
	sw_uint gid;
}acl_t;


/**
 * File descriptor link node
 **/
struct fd_link{
	struct link head;
	sw_int fd;
	struct sw_file_operations *dev_info;
};

/**
 * @brief Task structure
 */
struct sw_task {
	struct link head;
	struct link ready_head;
	struct link wait_head;
	/*! Opened file/dev list */
	struct link file_dev_list;
	/*! Task ID */
	sw_uint task_id;
	/*! Service ID */           
	sw_uint service_id;         
	/*! Task entry address */
	sw_uint entry_addr;     
	/*! Task name */    
	char name[NAME_LENGTH];          
	/*! Task state */
	sw_uint state;
	/*! Task run mode */
	sw_uint mode;
	/*! Task stack pointer */
	void* task_sp;
	/*! Global wait task list */
	struct link task_wait_list;
	/*! Spinlock to lock the wait list*/
	struct lock_shared task_wait_lock;
	/*! Task stack size */
	sw_uint task_sp_size;
	
	struct usr_task_info user_info; 
	/*!  elf flag */
	int elf_flag;

	/*! task local storage */
	sw_tls* tls;
	/*! task local storage physical address */
	sw_phy_addr tls_phy_addr;
	/*! Registers of the task */
	struct sw_task_cpu_regs regs;
	/*! Pending notification queue */
	struct task_pending_queue notify_queue;
	/*! User Access Control List */
	acl_t acl;
	/*! guest no */
	sw_int guest_no;
};


/**
 * @brief Create a task
 *
 * This function helps in task creation. It allocates the task structure, 
 * task local storage, task stack. It puts the task in suspend state and 
 * adds to global task list.
 *
 * @param psa_config: Configuration for task creation
 * @param task_id: Output parameter for the task ID.
 *
 * @return :
 * SW_OK - Task created successfully\n
 * SW_* - An implementation-defined error code for any other error.\n
 */
int create_task(sa_config_t *psa_config, int *task_id);

/**
 * @brief Destroy the created task
 *
 * This function cleans up the created task. It frees the resources which 
 * got allocated in task creation. It removes the task for global task list
 * 
 * @param task_id: Task ID 
 *
 * @return :
 * SW_OK - Task destroyed successfully\n
 * SW_* - An implementation-defined error code for any other error.\n
 */
int destroy_task(int task_id);

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
int start_task(int task_id, sw_uint* params);

/**
 * @brief Task context switch function
 *
 * This function switches the context between two tasks
 *
 * @param new_task: Task for which context need to be restored
 * @param old_task: Task for which context need to be saved
 */
void task_context_switch(struct sw_task *new_task, struct sw_task *old_task);

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
int get_task_state(int task_id);


/**
 * @brief Helper function to print the task context
 *
 * @param task_id: Task ID
 *
 * @return :
 * SW_OK - Printed successfully\n
 * SW_* - An implementation-defined error code for any other error.\n
 */
int print_task(int task_id);

/**
 * @brief 
 *      This functions wakes up a task from sleep.
 *      It is used for async tasks
 * @param task_id
 *      The task to be resumed
 * @return 
 */
void resume_async_task(int task_id);

/**
 * @brief Get task local storage
 *
 * This helper function returns the task local storage
 *
 * @param task_id: Task ID
 *
 * @return Returns the task local storage pointer or NULL.
 */
sw_tls* get_task_tls(int task_id);

/**
 * @brief Helper function to return task structure
 *
 * This helper function returns the task structure for the given task ID
 * 
 * @param task_id: Task ID
 *
 * @return Returns the pointer to task structure or NULL
 */
struct sw_task* get_task(int task_id);

/**
 * @brief Close device/file using descriptor data 
 *
 * @param fd_data: Device/File descriptor data
 *
 * @return 
 */
sw_int  close_device_file(struct fd_link *fd_data);

/**
 * @brief   Find out Instance already created or not
 *          for current running Application service
 *          
 * @param service_id service_id of the current running
 *                      Application service
 *
 * @return returns 1 if Application service is already running
 *                  Otherwise 0
 */
int check_multiple_instance(int service_id);

/**
 * @brief Add a task to the waiting list of another task.
 *
 * This implies that a task is waiting for another task to
 * complete before it can start again.
 * The waiting tasks will be scheduled to start at the
 * destruction(destroy_task()) of the running task.
 *
 * @param task: The task to whose list the waiting task is added
 * @param tmp_task: The task to be added to the list
 *
 * @return 
 */
void add_to_wait_list(struct sw_task *task, struct sw_task *tmp_task);

/** @} */ // end of taskapi
#endif /* __OTZ_TASK_H__ */
