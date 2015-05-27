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
 * Helper API's for User applications. This contains the helper functions
 * for user task entry, task exit.
 */

#include <sw_types.h>
#include <sw_debug.h>
#include <sw_syscall.h>
#include <tls.h>
#include <sw_user_app_api.h>
#include <sw_filelib.h>

/** @ingroup OS_UserApi User API for Secure OS
 *  API for Sierra OS User tasks
 *  @{
 */
/**
 * @brief Helper function to handle the task return
 *
 * This function helps to handle the return functionality of task. This function
 * puts the task in to wait or suspend state based on the return value and also
 * helps to set the return value of secure call or IPC call.
 *
 * @param task_id: Task ID
 * @param tls: Task local storage 
 */
static void handle_usr_task_return(int task_id, sw_tls* tls)
{
	exit_usr_task(SW_OK);
}


/**
* @brief Task startup routine
* This function need to be called on task entry. 
* This function intializes the user defined heap in case of 
* memory protection between kernel space  and user task.
*
* @param task_id: Task identifier
* @param tls: Task private data
*/
void task_init(int task_id, void* tls)
{
	if(!((sw_tls*)tls)->task_init_done){
		if(__alloc_user_heap(task_id, ((sw_tls*)tls)->heap_size,
								((sw_tls*)tls)->min_alloc_size) != SW_OK) {
			handle_usr_task_return(task_id, (sw_tls*)tls);
			while(1);
		}
		((sw_tls*)tls)->task_init_done = TRUE;
	}
}

/**
* @brief Task exit routine
* This function need to be called on task exit. 
* This fucntion cleans up the user defined heap in case of 
* memory protection between kernel space and user task.
* This also issues the exit task system call and yields 
* to next available task. 
*
* @param task_id: Task identifier
* @param tls: Task private data
*/
void task_exit(int task_id, void* tls)
{
	handle_usr_task_return(task_id, (sw_tls*)tls);
}
/** @} */ // end of OS_UserApi 
