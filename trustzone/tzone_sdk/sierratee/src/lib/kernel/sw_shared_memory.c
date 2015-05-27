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
 * Shared Memory implementation
 */
#include <sw_shm.h>
#include <sw_errno.h>
#include <global.h>
#include <mem_mng.h>
#include <sw_buddy.h>
#include <sw_heap.h>
#include <task.h>
#include <sw_mmu_helper.h>
#include <page_table.h>
#include <sw_string_functions.h>
#include <sw_mem_functions.h>

/** @ingroup OS_KernelApi Kernel API for Secure OS
 *  API for Sierra OS Kernel tasks
 *  @{
 */
/*
 * @brief create shared memory
 * @params : shared memory instance name
 * 			 shared memory instance size
 * 			 flags for shared memory
 */
sw_uint _shm_create(char *shm_name, sw_uint size, sw_uint flags){
	struct link *temp;
	sw_shared_memory *shm = NULL;
	sw_uint found = FALSE, id = 1, count = 0, cpsr;
	lock_irq_shared_resource(&global_val.shared_memory_ref_lock, &cpsr);	
	temp = global_val.shared_memory_ref.next;
	while(temp != &global_val.shared_memory_ref){
		shm = temp->data;
		if(sw_strncmp(shm->name ,shm_name, sw_strnlen(shm_name,MAX_NAME_LEN)) == SW_OK){
			found = TRUE;
			break;
		}
		temp = temp->next;
	}
	unlock_irq_shared_resource(&global_val.shared_memory_ref_lock, cpsr);
	if(!found && (flags & shm_flag_create)){
		if(size & (PAGE_SIZE - 1)) 	
			size = (size & (~(PAGE_SIZE - 1))) + PAGE_SIZE;
		while((id < SHARED_MEM_INSTANCE) && ((count * PAGE_SIZE) < size)){
			if(global_val.shm_id_pool[id] == 0x1)
				count = 0;
			if(global_val.shm_id_pool[id] == 0x0)
				count++;
			id++;
		}
		if((size/PAGE_SIZE) != count)
			goto shmget_error;
		id -= count;
		shm = sw_malloc_private(COMMON_HEAP_ID,
				sizeof(sw_shared_memory));
		if((sw_uint)shm == SW_ERROR)
			return SW_ERROR;
		sw_memset(shm, 0, sizeof(sw_shared_memory));
		shm->id = id;
		sw_strncpy(shm->name, shm_name, MAX_NAME_LEN);
		shm->creator_id = get_current_task_id();
		shm->flags = flags;
		shm->phy_addr = sw_phy_page_alloc(size, &global_val.sw_mem_info);
		if(shm->phy_addr == SW_OK)
			goto shmget_error;
		shm->vir_addr = secure_phy_to_vir(shm->phy_addr);
		if(sw_vir_addr_reserve(shm->vir_addr, size, &global_val.sw_mem_info)
				== SW_ERROR)
			goto shmget_error;
		shm->size = size;
		link_init(&shm->head);
		set_link_data(&shm->head, shm);
		lock_irq_shared_resource(&global_val.shared_memory_ref_lock, &cpsr);
		add_link(&global_val.shared_memory_ref, &shm->head, TAIL);
		unlock_irq_shared_resource(&global_val.shared_memory_ref_lock, cpsr);
		shm->status = 0x0;
		while(count){
			global_val.shm_id_pool[id] = 0x1;
			id++; count--;
		}
		return shm->id;
shmget_error:
		if(shm->vir_addr != SW_ERROR)
			sw_vir_free_reserve(shm->vir_addr, size, &global_val.sw_mem_info);
		if(shm->phy_addr != 0)
			sw_phy_addr_free(shm->phy_addr, size, &global_val.sw_mem_info);
		if(shm)
			sw_free_private(COMMON_HEAP_ID, shm);
		return SW_ERROR;
	}else{
		if(found)
			return shm->id;
		else
			return SW_ERROR;
	}
}

/*
 * @brief Control the shared memory instance
 * @params : shared memory instance id
 * 			 command id of the command to be executed on shared memory
 * 			 buffer containing data for the command
 */

sw_uint _shm_control(sw_uint id, sw_uint cmd_id, sw_uint* buffer){
	struct link *temp;
	sw_shared_memory *shm;
	sw_uint found = FALSE, cpsr;
	lock_irq_shared_resource(&global_val.shared_memory_ref_lock, &cpsr);	
	temp = global_val.shared_memory_ref.next;
	while(temp != &global_val.shared_memory_ref){
		shm = temp->data;
		if(shm->id == id){
			found = TRUE;
			break;
		}
		temp = temp->next;
	}
	unlock_irq_shared_resource(&global_val.shared_memory_ref_lock, cpsr);
	if(found){
		switch(cmd_id){
			case shm_cmd_set_cid:
				if((get_current_task_id() == shm->creator_id) 
						|| (shm->flags & shm_flag_ctrl)){
					if(buffer == NULL)
						return SW_ERROR;
					shm->creator_id = *buffer;
					break;
				}
			case shm_cmd_set_lid:
				if((get_current_task_id() == shm->creator_id) 
						|| (shm->flags != 0)){
					if(buffer == NULL)
						return SW_ERROR;
					shm->last_modify = *buffer;
					break;
				}
			case shm_cmd_set_flag:
				if((get_current_task_id() == shm->creator_id) 
						|| (shm->flags & shm_flag_ctrl)){
					if(buffer == NULL)
						return SW_ERROR;
					shm->flags = *buffer;
					break;
				}
			case shm_cmd_set_stat:
				shm->status = *buffer;
				break;
			case shm_cmd_stat:
				*buffer = shm->status;
				break;
			case shm_cmd_destroy:
				lock_irq_shared_resource(&global_val.shared_memory_ref_lock, &cpsr);
				remove_link(temp);
				unlock_irq_shared_resource(&global_val.shared_memory_ref_lock, cpsr);
				if(shm->vir_addr == SW_ERROR)
					sw_vir_free_reserve(shm->vir_addr, shm->size,
							&global_val.sw_mem_info);
				if(shm->phy_addr != 0)
					sw_phy_addr_free(shm->phy_addr, shm->size, 
							&global_val.sw_mem_info);
				while(shm->size){
					global_val.shm_id_pool[id] = 0x0;
					id++;
					shm->size = shm->size - PAGE_SIZE;
				}
				if(shm)
					sw_free_private(COMMON_HEAP_ID, shm);
				break;
		}
		return SW_OK;
	}else{
		return SW_ERROR;
	}
}

/*
 * @brief Attach shared memory to the task user space
 * @params : shared memory instance id
 * 			 address from user space where shm is going to attach
 * 			 flags for attaching the shared memory
 */
sw_uint* _shm_attach(sw_uint id, sw_uint* addr, sw_uint flag){
	struct link *temp = NULL;
	struct sw_task *task = NULL;
	sw_shared_memory *shm = NULL;
	sw_uint found = FALSE, at_addr = NULL, fg = 0, ret_val = SW_ERROR, 
			count = 0, cpsr;
	sw_vir_addr pt_base = NULL;
	lock_irq_shared_resource(&global_val.shared_memory_ref_lock, &cpsr);	
	temp = global_val.shared_memory_ref.next;
	while(temp != &global_val.shared_memory_ref){
		shm = temp->data;
		if(shm->id == id){
			found = TRUE;
			break;
		}
		temp = temp->next;
	}
	unlock_irq_shared_resource(&global_val.shared_memory_ref_lock, cpsr);
	if(found){
		task = get_current_task();
		if(addr != NULL)
			at_addr = (sw_uint)addr;
		else
		{
			pt_base = (sw_vir_addr)get_secure_ptd();
			id = 1;
			if(task->task_id != shm->creator_id){
				while((id < SHARED_MEM_INSTANCE) && 
						((count * PAGE_SIZE) < shm->size)){
					if(global_val.shm_id_pool[id] == 0x1)
						count = 0;
					if(global_val.shm_id_pool[id] == 0x0)
						count++;
					id++;
				}
				if((shm->size/PAGE_SIZE) != count)
					return (sw_uint*)SW_ERROR;
				id -= count;
				at_addr = SHM_AT_ADDR + (id * PAGE_SIZE);
				while(count){
					global_val.shm_id_pool[id] = 0x1;
					id++; count--;
				}
			}else{
				at_addr = SHM_AT_ADDR + (id * PAGE_SIZE);
			}
		}
		fg = shm->flags & flag;
		if(fg == shm_flag_invalid)
			return NULL;
		if(fg & shm_flag_write)
			ret_val = map_user_data_memory(at_addr, shm->phy_addr, shm->size,
					pt_base);
		else
			ret_val = map_user_rodata_memory(at_addr, shm->phy_addr, shm->size, 
					pt_base);
	}
	if(ret_val == SW_OK)
		return (sw_uint*)at_addr;
	else{
		if(task->task_id != shm->creator_id){
			count = shm->size/PAGE_SIZE;
			id = (at_addr - SHM_AT_ADDR)/PAGE_SIZE;
			while(count){
				global_val.shm_id_pool[id] = 0x0;
				id++; count--;
			}
		}
		return (sw_uint*)SW_ERROR;
	}
}

/*
 * @brief Detach shared memory from task user space
 * @params : shared memory instance id
 * 			 address to be unmapped from user space
 */
sw_uint _shm_detach(sw_uint id, sw_uint* addr){
	struct link *temp = NULL;
	sw_shared_memory *shm = NULL;
	struct sw_task *task;
	sw_uint found = FALSE, cpsr, count;
	sw_vir_addr pt_base;
	lock_irq_shared_resource(&global_val.shared_memory_ref_lock, &cpsr);	
	temp = global_val.shared_memory_ref.next;
	while(temp != &global_val.shared_memory_ref){
		shm = temp->data;
		if(shm->id == id){
			found = TRUE;
			break;
		}
		temp = temp->next;
	}
	unlock_irq_shared_resource(&global_val.shared_memory_ref_lock, cpsr);
	if(found){
		task = get_current_task();
		pt_base = (sw_vir_addr)get_secure_ptd();
		if(task->task_id != shm->creator_id){
			count = shm->size/PAGE_SIZE;
			id = ((sw_uint)addr - SHM_AT_ADDR)/PAGE_SIZE;
			while(count){
				global_val.shm_id_pool[id] = 0x0;
				id++; count--;
			}
		}
		return unmap_user_memory((sw_vir_addr)addr, shm->size, pt_base);
	}else
		return SW_ERROR;
}
/** @} */ // end of OS_KernelApi
