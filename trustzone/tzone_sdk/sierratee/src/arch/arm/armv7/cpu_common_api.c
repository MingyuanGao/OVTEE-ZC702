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
 * Common API's specific to ARM architecture. This architecture API's used in
 * both kernel and user based applications. 
 */
 
#include <sw_types.h> 

/**
* @brief System call to exit the user task
*
* @param task_id: Task ID
* @param tls: Pointer to Task local storage
*/
void exit_usr_task(int exit_status)
{
	register sw_uint r1 asm("r1") = exit_status;
	do {
		asm volatile("swi #0xffe6 \n"
		:: "r" (r1));
	} while (0);

	return;
}


/**
 * @brief Invoke scheduler
 *
 * This function invokes the scheduler to schedule the next ready task
 */
void schedule(void)
{
	asm volatile("swi #0xbb");
}
