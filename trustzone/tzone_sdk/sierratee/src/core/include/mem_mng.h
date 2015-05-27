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
 * Simple Memory manager for secure world
 */

#ifndef __MEM_MNG_H_
#define __MEM_MNG_H_

#include <sw_types.h>
#include <tls.h>

#define VIR_ASPACE_SIZE     (1 << 26)
#define USER_PAGE			0x1
#define KERN_PAGE			0x0

/**
 * @brief
 *  This structure has information about the 
 *  memory regions in the secure world
 */
typedef struct secure_world_mem_info{
	/*! Start address of virtual memory bitmap */
	sw_uint* vir_mem_bmap;
	/*! Start address of physical memory bitmap */
	sw_uint* phy_mem_bmap;

	/*!Length of virtual memory bitmap*/
	size_t vir_mem_bmap_len;
	/*!Length of physical memory bitmap*/
	size_t phy_mem_bmap_len;

	/*! Start address of virtual memory region */
	sw_uint* vir_addr_start;
	/*! Size of virtual address region*/
	size_t vir_memregion_size;

	/*! Start address of secure RAM */
	sw_uint phy_addr_start;
	/*! Size of secure RAM */
	size_t phy_memregion_size;

	/*! Number of 4K pages available in virtual memory region configured*/
	sw_uint num_phy_pag_free;
	/*! Number of 4K pages available in RAM */
	sw_uint num_vir_pag_free;
	sw_uint page_scope;
	sw_uint elf_flag;
}mem_info;

/**
 * @brief 
 *
 * @param sw_mem_info
 *
 * @return 
 */
sw_uint* sw_meminfo_init(mem_info* sw_mem_info);

/**
 * @brief 
 *
 * @param size
 *
 * @return 
 */
sw_phy_addr sw_phy_page_alloc(sw_uint size, mem_info *info);

/**
 * @brief 
 *
 * @param size
 *
 * @return 
 */
sw_uint* sw_vir_page_alloc(sw_uint size, mem_info *info);

/**
 * @brief 
 *
 * @param phy_addr
 * @param size
 *
 * @return 
 */
int sw_phy_addr_reserve(sw_phy_addr phy_addr , int size, mem_info *info);

/**
 * @brief 
 *
 * @param vir_addr
 * @param size
 *
 * @return 
 */
int sw_vir_addr_reserve(sw_vir_addr vir_addr , int size, mem_info *info);

/**
 * @brief 
 *
 * @param vir_addr
 * @param size
 *
 * @return 
 */
int sw_vir_free_reserve(sw_vir_addr vir_addr , int size, mem_info *info);

/**
 * @brief 
 *
 * @param phy_addr
 * @param size
 *
 * @return 
 */
int sw_phy_addr_free(sw_phy_addr phy_addr , int size, mem_info *info);

/**
 * @brief 
 *
 * @param vir_addr
 * @param size
 *
 * @return 
 */
int sw_vir_addr_free(sw_vir_addr vir_addr , int size, mem_info *info);

#endif
