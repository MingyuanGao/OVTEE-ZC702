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


#ifndef _SHARED_MEMORY_
#define _SHARED_MEMORY_

#include <sw_types.h>

/*
 * Shared memory header file
 */

/*
 * List of control commands
 */
enum shared_memory_commands {
	shm_cmd_invalid = 0,
	shm_cmd_read,
	shm_cmd_write,
	shm_cmd_lock,
	shm_cmd_unlock,
	shm_cmd_stat,
	shm_cmd_set_cid,
	shm_cmd_set_lid,
	shm_cmd_set_flag,
	shm_cmd_set_stat,
	shm_cmd_destroy
};

/*
 * Shared memory flags
 */
enum shared_memory_flags {
	shm_flag_invalid = 0,
	shm_flag_read = 1,
	shm_flag_write = 2,
	shm_flag_lock = 4,
	shm_flag_ctrl = 8,
	shm_flag_create = 16
};

/*
 * @brief create shared memory
 * @params : shared memory instance name
 * 			 shared memory instance size
 * 			 flags for shared memory
 */
sw_uint shm_create(char *shm_name, sw_uint size, sw_uint flags);

/*
 * @brief Control the shared memory instance
 * @params : shared memory instance id
 * 			 command id of the command to be executed on shared memory
 * 			 buffer containing data for the command
 */
sw_uint shm_control(sw_uint id, sw_uint cmd_id, sw_uint* buffer);

/*
 * @brief Attach shared memory to the task user space
 * @params : shared memory instance id
 * 			 address from user space where shm is going to attach
 * 			 flags for attaching the shared memory
 */
sw_uint* shm_attach(sw_uint id, sw_uint* addr, sw_uint flag);

/*
 * @brief Detach shared memory from task user space
 * @params : shared memory instance id
 * 			 address to be unmapped from user space
 */
sw_uint shm_detach(sw_uint id, sw_uint* addr);

#endif
