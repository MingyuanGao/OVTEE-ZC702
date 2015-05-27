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

#ifndef __LIB__SW_USR_HEAP_H__
#define __LIB__SW_USR_HEAP_H__

#include <sw_types.h>
#include <tls.h>

/**
 * @brief Getting different tls structure for
 * 	USER_PAGE_TABLE_ISOLATION enable/disable
 * 	cases
 *
 * @return 
 */
sw_tls* get_user_tls();

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
