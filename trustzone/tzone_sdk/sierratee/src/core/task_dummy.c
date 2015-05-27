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

#include <sw_types.h>
#include <tls.h>
#include <svisor_global.h>

struct svisor_global global_val;

/**
 * @brief 
 *      Returns the id of the current running task
 *
 * @return
 *      task id
 */
int get_current_task_id(void)
{
	return -1;
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
	return NULL;
}
