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
 * Heap functions
 */

#include <sw_addr_config.h>
#include <sw_debug.h>
#include <sw_buddy.h>
#include <sw_mem_functions.h>
#include <sw_user_heap.h>
#include <tls.h>
#include <sw_syscall.h>

/**
 * @brief Getting different tls structure for
 *  USER_PAGE_TABLE_ISOLATION enable/disable
 *  cases
 *
 * @return 
 */
sw_tls* get_user_tls() 
{
	sw_tls *tls;
	tls = __sw_get_tls();
	return tls;
}
/**
 * @brief 
 *
 * @param ptr
 *
 * @return 
 */
sw_uint get_usr_ptr_size(void* ptr)
{
	sw_int task_id;
	struct sw_heap_info *heap_info;
	sw_tls *tls;
	
	tls = get_user_tls();

	task_id = tls->task_id;

	heap_info = &tls->heap_info;
	if(!heap_info) {
		return 0;
	}

	return get_buddy_ptr_size(&heap_info->buddy_pool, ptr);
}


/**
 * @brief
 *
 * @param size
 *
 * @return
 */
void *sw_malloc(sw_uint size)
{
	void *addr = NULL;
	sw_int task_id;
	struct sw_heap_info *heap_info;
	sw_tls *tls;
	
	tls = get_user_tls();
	
	task_id = tls->task_id;

	heap_info = &tls->heap_info;
	if(!heap_info) {
		goto sw_usr_malloc_ret;
	}

	addr = (void *)sw_buddy_alloc(&heap_info->buddy_pool, size);
	if(addr)
		heap_info->num_blocks_alloc++;

	if(addr == 0) {
		sw_printf("sw_malloc failed for task id 0x%x and size %d\n", 
				task_id, size);
	}
sw_usr_malloc_ret:
	return addr;
}


/**
 * @brief 
 *
 * @param ptr
 * @param size
 *
 * @return 
 * */
void* sw_realloc(void* ptr, sw_uint size)
{
    void* local_ptr = NULL;
    sw_uint old_size = 0;

    if(ptr == NULL) {
        return(sw_malloc(size));
    }
    if((size == 0) && (ptr != NULL)) {
        sw_free(ptr);
        return(NULL);
    }
    local_ptr = sw_malloc(size);
    old_size = get_usr_ptr_size(ptr);
    sw_memcpy(local_ptr,ptr, min(old_size,size));
    sw_free(ptr);
    return(local_ptr);
}

/**
 * @brief
 *
 * @param pointer
 */
void sw_free(void *pointer)
{
	struct sw_heap_info *heap_info;
	sw_int task_id;
	sw_tls *tls;
	
	tls = get_user_tls();
	
	if(!pointer)
		return;

	task_id = tls->task_id;

	heap_info = &tls->heap_info;
	if(!heap_info) {
		return;
	}
	sw_buddy_free(&heap_info->buddy_pool, pointer);
	heap_info->num_blocks_alloc--;
	
	return;
}

