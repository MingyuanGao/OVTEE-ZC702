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
 * Header for Kernel Heap management functions
 * This file includes the declaration of kernel based heap management routines.
 */

#ifndef __LIB__SW_HEAP_H__
#define __LIB__SW_HEAP_H__

#include <sw_types.h>
#include <mem_mng.h>
#include <sw_buddy.h>

#define COMMON_HEAP_ID 0
#define MIN_ALLOC_SIZE  256

/**
 * @brief 
 *
 * @param heap_id
 * @param size
 *
 * @return 
 */
void *sw_malloc_private(int heap_id ,sw_uint size);

/**
 * @brief 
 *
 * @param heap_id
 * @param pointer
 */
void sw_free_private(int heap_id, void *pointer);

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
			mem_info *info, struct sw_heap_info *heap_info);

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
int alloc_user_heap(int heap_id, int size, int min_alloc_size,mem_info *info);

/**
 * @brief 
 *
 * @param heap_id
 * @param info
 *
 * @return 
 */
int free_user_heap(int heap_id, mem_info *info);

/**
 * @brief 
 *
 * @param heap_id
 *
 * @return 
 */
int free_private_heap(int heap_id, mem_info *info);

/**
 * @brief
 *
 * @param size
 *
 * @return
 */
void *sw_malloc(sw_uint size);

/**
 * @brief 
 *
 * @param ptr
 * @param size
 *
 * @return 
 * */
void* sw_realloc(void* ptr, sw_uint size);

/**
 * @brief
 *
 * @param pointer
 */
void sw_free(void *pointer);

#endif
