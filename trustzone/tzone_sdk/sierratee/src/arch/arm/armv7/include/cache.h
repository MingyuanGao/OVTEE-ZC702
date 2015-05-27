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
 * Header for cache implementation
 */

#ifndef __ARMV7_CACHE_H__
#define __ARMV7_CACHE_H__

#include <sw_types.h>

/**
* @brief 
*/
void enable_l1_cache(void);

/**
* @brief 
*/
void icache_inv_all(void);

/**
* @brief 
*
* @param invalidate_only
*/
void data_cache_clean_invalidate_all(int invalidate_only);	

/**
* @brief 
*/
void flush_all_translation_table_entries(void);

/**
* @brief Invalidate TLB by MVA and ASID
*
* @param va
*/
void inv_translation_table_by_va_asid(sw_uint va);

/**
* @brief Invalidate TLB by ASID
*
* @param asid
*/
void inv_translation_table_by_asid(sw_uint asid);

/**
* @brief 
*/
void flush_icache_and_dcache(void);
#endif
