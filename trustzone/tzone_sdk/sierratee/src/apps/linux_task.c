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
 * Linux task implementation
 */

#include <linux_task.h>
#include <global.h>
#include <task.h>
#include <sw_types.h>
#include <otz_id.h>
#include <cpu_data.h>
#include <sw_board.h>
#include <sw_mem_functions.h>
#include <sw_string_functions.h>
#include <monitor.h>
#include <secure_api.h>
#include <sw_mmu_helper.h>
#include <sw_heap.h>
#include <tls.h>
#include <sw_app_api.h>

void linux_task(int task_id,sw_tls * tls)
{
	int invoke_value;
	struct sw_task *task;

	task_init(task_id, tls);	
	task = get_task(task_id);

	invoke_value = ((struct linux_global*)task->tls->private_data)->invoke_flag;

	if(!invoke_value){
		invoke_ns_kernel();
		((struct linux_global*)task->tls->private_data)->invoke_flag = 1;
	}
	else
		return_non_secure_kernel();
	task_exit(task_id, tls);
}

