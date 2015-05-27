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
   Buddy allocator declarations
 */
#ifndef __SW_BUDDY_H_
#define __SW_BUDDY_H_

#include <sw_types.h>

#define PAGE_SIZE 4096

#define MAX_BUDDY_BIN_SHIFT     31
#define MAX_BUDDY_ALLOC_SIZE (1 << MAX_BUDDY_BIN_SHIFT)

#define MIN_BUDDY_BIN_SHIFT  12
#define MIN_BUDDY_ALLOC_SIZE (1 << MIN_BUDDY_BIN_SHIFT) 

#define MAX_BINS (MAX_BUDDY_BIN_SHIFT - MIN_BUDDY_BIN_SHIFT + 1)

/**
 * @brief Buddy area used for free and allocation list
 */
struct sw_area {
	struct sw_area *parent;
	struct sw_area *lchild;
	struct sw_area *rchild;
	void *mem_addr;
	sw_uint block_size;
	sw_uint bin;
	sw_uint cnt;
};	


/**
 * @brief Buddy pool manager structure holds information 
 * about the allocator.
 */
struct sw_buddy_pool {
	struct sw_area *area;
	sw_uint total_area_cnt;
	sw_uint next_free_index;
	sw_uint alloc_cnt;
	struct sw_area alloc_tree;
	void *mem_start;
	sw_uint mem_size;
	void *pool_start;
	sw_uint pool_size;
	sw_uint max_alloc_size;
	sw_uint min_alloc_size;
	sw_uint curr_heap_size;
	sw_uint max_bin;
	struct sw_area free_area[MAX_BINS];
};

/**
 * @brief Heap information structure for the corresponding buddy allocator 
 */
struct sw_heap_info {
	struct sw_buddy_pool buddy_pool;
	sw_uint num_blocks_alloc;
	sw_vir_addr heap_vir_addr;	
	sw_phy_addr heap_phy_addr;		
};


/**
 * @brief Init buddy allocator
 * 
 * Initialize the buddy allocator based on the minimum and maximum allocation
 * size. 
 *
 * @param buddy_pool: Pointer to buddy pool
 * @param pool_start: Heap start address
 * @param pool_size: Buddy size include the heap size and management area
 * @param min_alloc_size: Minimum allocation size
 * @param max_alloc_size: Maximum allocation size
 *
 * @return 
 * SW_OK - Buddy initialization done successfully\n
 * SW_* - An implementation-defined error code for any other error.\n
 */
int sw_buddy_init(struct sw_buddy_pool *buddy_pool,
		sw_uint pool_start, sw_big_ulong pool_size, 
		sw_uint min_alloc_size, sw_uint max_alloc_size);
/**
 * @brief Buddy memory allocation routine
 *
 * This function allocates the memory from buddy pool.
 *
 * @param buddy_pool: Pointer to the buddy pool
 * @param size: Size of the memory requested
 *
 * @return : Valid memory address or NULL pointer
 */
void *sw_buddy_alloc(struct sw_buddy_pool *buddy_pool, sw_uint size);

/**
 * @brief Free the previosuly allocated memory from Buddy
 *
 * This function frees the previously allocated memory and re-arrange the
 * buddies
 *
 * @param buddy_pool: Pointer to buddy pool
 * @param ptr: Allocated memory pointer which needs to be freed.
 */
void sw_buddy_free(struct sw_buddy_pool *buddy_pool, void *ptr);

/**
 * @brief Get the size of the pointer
 *
 * This function returns the buddy block size of the pointer
 *
 * @param buddy_pool: Pointer to the buddy pool
 * @param pointer: Memory address for which size is requested
 *
 * @return : Return the allocated size of the pointer
 */
sw_int get_buddy_ptr_size(struct sw_buddy_pool *buddy_pool, void *pointer);

/**
 * @brief Get managed area size for the given heap and minimum allocation size
 *
 * @param heap_size: Heap Size 
 * @param min_alloc_size: Minium allocation size of the buddy
 *
 * @return 
 */
int get_manage_area(size_t heap_size,sw_uint min_alloc_size);
#endif
