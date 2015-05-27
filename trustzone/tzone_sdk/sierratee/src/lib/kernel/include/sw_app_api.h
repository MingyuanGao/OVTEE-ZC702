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
   Application routines for kernel application tasks. 
 */
#ifndef __SW_KERNEL_APP_API_H_
#define __SW_KERNEL_APP_API_H_

#include <tls.h>


/**
* @brief Task startup routine
* This function need to be called on task entry. 
* This function intializes the user defined heap in case of 
* memory protection between kernel space  and user task.
*
* @param task_id: Task identifier
* @param tls: Task private data
*/
void task_init(int task_id, void* tls);


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
void task_exit(int task_id, void* tls);
#endif
