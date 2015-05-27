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

#include <sw_debug.h>
#include <sw_mem_functions.h>
#include <sw_buddy.h>
#include <sw_heap.h>

#ifndef CONFIG_SVISOR_SUPPORT
#include <global.h>
#include <task.h>
#endif

#include <sw_addr_config.h>
#include <sw_mmu_helper.h>
#include <debug_config.h>

#ifdef CONFIG_SVISOR_SUPPORT
extern struct sw_heap_info *get_svisor_heap_info(sw_int heap_id);
static struct sw_heap_info* get_heap_info(sw_int heap_id) 
{
	return get_svisor_heap_info(heap_id);
}
#else
static struct sw_heap_info* get_heap_info(sw_int heap_id) 
{
	struct sw_tls *tls = NULL;
	struct sw_heap_info *heap_info = NULL;

	if(heap_id != COMMON_HEAP_ID) {
		tls = get_task_tls(heap_id);
		if(tls == NULL) {
			sw_seterrno(SW_EINVAL);
			return NULL;
		}
		heap_info = &tls->heap_info;
	}
	else {
		heap_info = &global_val.heap_info;
	}
	return heap_info;
}
#endif


/**
 * @brief 
 *
 * @param ptr
 *
 * @return 
 */
static sw_uint get_ptr_size(void* ptr)
{
	struct sw_heap_info *heap_info;
	int task_id = -1;

	task_id = get_current_task_id();
	heap_info = get_heap_info(task_id);

	if(!heap_info) 
		return NULL;

	return get_buddy_ptr_size(&heap_info->buddy_pool, ptr);
}

/**
 * @brief 
 *
 * @param heap_id
 * @param size
 *
 * @return 
 */
void *sw_malloc_private(int heap_id ,sw_uint size)
{ 
	sw_uint addr = 0;

	struct sw_heap_info *heap_info;
	heap_info = get_heap_info(heap_id);

	if(!heap_info) {
		goto sw_malloc_private_ret;
	}

	addr = (sw_uint)sw_buddy_alloc(&heap_info->buddy_pool, size);
	if(addr > 0)
		heap_info->num_blocks_alloc++;

	if(addr == 0) {
		sw_printk("malloc private failed for heap id 0x%x and size %d\n", 
				heap_id, size);
	}
sw_malloc_private_ret:
	return (sw_uint*)addr;
}


/**
 * @brief 
 *
 * @param heap_id
 * @param pointer
 */
void sw_free_private(int heap_id, void *pointer)
{
	struct sw_heap_info *heap_info;
	heap_info = get_heap_info(heap_id);

	if(!heap_info) {
		goto sw_free_private_ret;
	}

	sw_buddy_free(&heap_info->buddy_pool, pointer);
	heap_info->num_blocks_alloc--;
	
sw_free_private_ret:
	return;
}

#ifndef CONFIG_SVISOR_SUPPORT
/**
 * @brief 
 *
 * @param heap_id
 * @param size
 * @param min_alloc_size
 * @param info
 *
 * @return 
 */
int alloc_user_heap(int heap_id, int size, int min_alloc_size,
					mem_info *info)
{
	sw_uint heap_start=0;
	size_t heap_size=0;
	sw_uint manage_area = 0;
	sw_phy_addr heap_phy_addr = 0;
	sw_vir_addr heap_kern_vir_addr = 0;
	sw_vir_addr map_addr;
	sw_phy_addr pt=NULL;

	struct sw_task *task = NULL;
	int ret_val = SW_OK;
	struct sw_heap_info *heap_info;

	heap_info = get_heap_info(heap_id);
	
	if(!heap_info) {
		ret_val = SW_ERROR;
		goto alloc_user_heap_ret;
	}

	heap_size = size;

	manage_area = get_manage_area(heap_size, (sw_uint)min_alloc_size);

	heap_phy_addr = sw_phy_page_alloc(heap_size + manage_area, info);
	
	if(heap_phy_addr == 0) {
		sw_printk("Physical page allocation failed for user heap alloc\n");
		ret_val = SW_ERROR;
		goto alloc_user_heap_ret;
	}
	
	task = get_task(heap_id);
	if(!task) {
		sw_printk("Invalid task\n");
		ret_val = SW_ERROR;
		goto alloc_user_heap_ret;
	}
	
	heap_kern_vir_addr = secure_phy_to_vir(heap_phy_addr);
	if(sw_vir_addr_reserve(heap_kern_vir_addr, 
			heap_size + manage_area, info) != SW_OK) {
		sw_printk("Virtual page reserve failed for user heap alloc\n");
		heap_kern_vir_addr = 0;
		ret_val = SW_ERROR;
		goto alloc_user_heap_ret;
	}

	map_addr = heap_kern_vir_addr;
	pt = (sw_phy_addr)get_secure_ptd();

	if(task->mode == TASK_USER_MODE) {			
		if(map_user_data_memory(map_addr, heap_phy_addr, heap_size + manage_area,
				pt) != SW_OK) {
			sw_printk("mapping to user memory failed for user heap alloc\n");
			heap_start = 0;
			ret_val = SW_ERROR;
			goto alloc_user_heap_ret;
		}
	}
	else {
		if(map_user_memory(map_addr, heap_phy_addr, heap_size + manage_area,
				PTF_PROT_KRW, pt) != SW_OK) {
			sw_printk("mapping to user memory failed for user heap alloc\n");
			heap_start = 0;
			ret_val = SW_ERROR;
			goto alloc_user_heap_ret;
		}

	}
	heap_start = map_addr;

	if(size < min_alloc_size)
		size = min_alloc_size;
	
	if(sw_buddy_init(&heap_info->buddy_pool, heap_start,
				(sw_big_ulong)(heap_size +manage_area),
				(sw_uint)min_alloc_size, heap_size) != SW_OK) {

		sw_printk("buddy init failed for user heap alloc\n");
		ret_val = SW_ERROR;
		goto alloc_user_heap_ret;

	}
	
	heap_info->heap_vir_addr = heap_kern_vir_addr;
	heap_info->heap_phy_addr = heap_phy_addr;
alloc_user_heap_ret:
	if(ret_val != SW_OK) {
		if(heap_start) {
			unmap_user_memory(heap_start,
				heap_size + manage_area, pt);
		}
		if(heap_kern_vir_addr) {
			sw_vir_free_reserve(heap_kern_vir_addr,
				heap_size + manage_area, info);
		}
		if(heap_phy_addr) {
			sw_phy_addr_free(heap_phy_addr, heap_size + manage_area, info);
		}
		sw_printk("SW: Heap allocation for task : %x failed \n", heap_id);
	}
	return ret_val;
}
#endif /* CONFIG_SVISOR_SUPPORT */

/**
 * @brief 
 *
 * @param heap_id
 * @param size
 * @param min_alloc_size
 * @param info
 * @param heap_info
 *
 * @return 
 */
int alloc_private_heap(int heap_id, int size, int min_alloc_size,
				mem_info *info, struct sw_heap_info *heap_info)
{
	int ret_val = SW_OK;
	sw_uint heap_start = 0;
	size_t heap_size;
	sw_uint manage_area;

	heap_size = size;
	manage_area = get_manage_area(heap_size, (sw_uint)min_alloc_size);
	
	heap_start = (sw_uint)sw_vir_page_alloc(heap_size+manage_area, info);

	if(!heap_start) {
		sw_printk("virtual page alloc failed in heap init 0x%x\n", heap_id);
		ret_val = SW_ERROR;
		goto alloc_private_heap_ret;
	}

	if(size < min_alloc_size)
		size = min_alloc_size;
	if(sw_buddy_init(&heap_info->buddy_pool, heap_start, 
				(sw_big_ulong)(heap_size+manage_area),
				(sw_uint)min_alloc_size, heap_size) != SW_OK) {
		sw_printk("buddy init failed in heap init 0x%x\n", heap_id);
		ret_val = SW_ERROR;
		goto alloc_private_heap_ret;

	}
	heap_info->num_blocks_alloc = 0;
	heap_info->heap_vir_addr = heap_start;
alloc_private_heap_ret:
	if(ret_val != SW_OK) {
		sw_printk("SW: Heap allocation for task : %x failed \n", heap_id);

		if(heap_start)
			sw_vir_addr_free(heap_start, heap_size+manage_area, info);
	}
	return ret_val;
}

#ifndef CONFIG_SVISOR_SUPPORT
/**
 * @brief 
 *
 * @param heap_id
 *
 * @return 
 */
int free_user_heap(int heap_id, mem_info *info)
{
	int ret_val = SW_OK;
	sw_phy_addr heap_phy_addr;
	struct sw_heap_info *heap_info;
	struct sw_task *task = NULL;
	sw_vir_addr map_addr;
	sw_phy_addr pt;

	heap_info = get_heap_info(heap_id);
	
	if(!heap_info) {
		ret_val = SW_ERROR;
		goto free_user_heap_ret;
	}

    if(heap_info->num_blocks_alloc != 0){
            sw_printk("SW: Error : This heap not to be freed 0x%x and \
                            pending free 0x%x\n", 
                            heap_id, heap_info->num_blocks_alloc);
            sw_seterrno(SW_EBUSY);
            ret_val = SW_ERROR;
    }

	task = get_task(heap_id);
	if(!task) {
		sw_printk("Invalid task\n");
		ret_val = SW_ERROR;
	}
	
	
	heap_phy_addr = heap_info->heap_phy_addr;
	if(heap_phy_addr == 0)
		goto free_user_heap_ret;
	
	map_addr = heap_info->heap_vir_addr;
	pt = (sw_phy_addr)get_secure_ptd();
	
	if(task) {
		if(unmap_user_memory(map_addr,
				heap_info->buddy_pool.pool_size, 
				pt) != SW_OK) {
			sw_printk("free_user_heap: unmap user memory failed\n");
			ret_val = SW_ERROR;
		}
	}
	if(sw_vir_free_reserve(heap_info->heap_vir_addr, 
			heap_info->buddy_pool.pool_size, info) != SW_OK) {
		sw_printk("free_user_heap: virtual free reserve failed\n");
		ret_val = SW_ERROR;
	}
	
	if(sw_phy_addr_free(heap_phy_addr, 
			heap_info->buddy_pool.pool_size, info) != SW_OK) {
		sw_printk("free_user_heap: physial address free failed\n");
		ret_val = SW_ERROR;
	}
free_user_heap_ret:

	return ret_val;
}
#endif
/**
 * @brief 
 *
 * @param heap_id
 *
 * @return 
 */
int free_private_heap(int heap_id, mem_info *info)
{
	int ret_val = SW_OK;
	struct sw_heap_info *heap_info;
	heap_info = get_heap_info(heap_id);
	if(!heap_info) {
		ret_val = SW_ERROR;
		goto free_private_heap_ret;
	}

    if(heap_info->num_blocks_alloc != 0){
            sw_printk("SW: Error : This heap not to be freed 0x%x and \
                            pending free 0x%x\n", 
                            heap_id, heap_info->num_blocks_alloc);
            sw_seterrno(SW_EBUSY);
            ret_val = SW_ERROR;
    }
	ret_val = sw_vir_addr_free((sw_vir_addr)heap_info->buddy_pool.pool_start,
					heap_info->buddy_pool.pool_size, info);

free_private_heap_ret:
	return ret_val;
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
	void *addr;
	int task_id = -1;

	task_id = get_current_task_id();
	addr = sw_malloc_private(task_id, size);
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
    old_size = get_ptr_size(ptr);
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
	int task_id = -1;

	if(!pointer)
		return;

	task_id = get_current_task_id();  
	sw_free_private(task_id, pointer);
}


