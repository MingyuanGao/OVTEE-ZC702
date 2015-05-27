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
 * Shared Mem test task implementation
 */

#include <sw_types.h>
#include <sw_debug.h>
#include <sw_string_functions.h>
#include <sw_mem_functions.h>
#include <otz_id.h>
#include <sw_shared_memory.h>
#include <sw_semaphore.h>
#include <sw_user_app_api.h>
#include <tls.h>

/**
 * @brief test_shm entry point
 *
 * @param task_id: task identifier
 * @param tls: Pointer to task local storage
 */
void test_shm(int task_id, sw_tls* tls)
{
	if(!tls->task_init_done){
		task_init(task_id, tls);
		tls->task_init_done = TRUE;
	}
	sw_semaphore *sem = sw_semaphore_init("SHARED_MEM_TEST", 1);
	sw_uint *addr = NULL, *buffer, status = 0;
	buffer = sw_malloc(1024);
	sw_uint shm_id = shm_create("SHARED_MEM_TEST", 1024, shm_flag_create |
			shm_flag_read | shm_flag_write);
	if(shm_id == SW_ERROR)
		goto error;
	if((sw_uint)(addr = shm_attach(shm_id, NULL, shm_flag_read)) == SW_ERROR)
		goto error;
	while(1){
		sw_release_semaphore(sem);
		shm_control(shm_id, shm_cmd_stat, &status);
		if(status & shm_flag_write){
			sw_memcpy(buffer, addr, 1024);
			status = shm_flag_read;
			shm_control(shm_id, shm_cmd_set_stat, &status);
			sw_printf("%s\n", buffer);
		}
	}
	sw_free(buffer);
	shm_detach(shm_id, addr);
	shm_control(shm_id, shm_cmd_destroy, NULL);
	sw_delete_semaphore(sem);
error:
	task_exit(task_id, tls);
	tee_panic("test shm - hangs\n");
}
