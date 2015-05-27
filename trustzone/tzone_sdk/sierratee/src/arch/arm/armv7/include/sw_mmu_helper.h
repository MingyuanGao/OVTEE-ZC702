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
 * Header for MMU helper routines
 * Routines to set ttbr0, ttbr1, ttbcr.
 * Routines to enable/disable MMU
 * Routines to setup system before enabling MMU.
 */

#ifndef _SW_MMU_HELPER_H__
#define _SW_MMU_HELPER_H__

#include <sw_types.h>
#include <sw_link.h>
#include <sw_mmu_helper_asm.h>

/**
 * @brief Data abort fault status values
 */
enum data_abort_fault_status
{
	dfs_alignment_fault = 0b00001,
	dfs_debug_event = 0b00010,
	dfs_access_flag_section = 0b00011,
	dfs_icache_maintenance = 0b00100,
	dfs_translation_section = 0b00101,
	dfs_access_flag_page = 0b00110,
	dfs_translation_page = 0b00111,
	dfs_sync_external_abt = 0b01000,
	dfs_domain_section = 0b01001,
	dfs_domain_page = 0b01011,
	dfs_translation_table_walk_lvl1_sync_ext_abt = 0b01100,
	dfs_permission_section = 0b01101,
	dfs_translation_table_walk_lvl2_sync_ext_abt = 0b01110,
	dfs_permission_page = 0b01111,
	dfs_imp_dep_lockdown = 0b10100,
	dfs_async_external_abt = 0b10110,
	dfs_mem_access_async_parity_err = 0b11000,
	dfs_mem_access_async_parity_err2 = 0b11001,
	dfs_imp_dep_coprocessor_abort = 0b11010,
	dfs_translation_table_walk_lvl1_sync_parity_err = 0b11100,
	dfs_translation_table_walk_lvl2_sync_parity_err = 0b11110,
};

/**
 * @brief Instruction abort fault status values
 */
enum instruction_abort_fault_status
{
	ifs_debug_event = 0b00010,
	ifs_access_flag_fault_section = 0b00011,
	ifs_translation_fault_section = 0b00101,
	ifs_access_flag_fault_page = 0b00110,
	ifs_translation_fault_page = 0b00111,
	ifs_synchronous_external_abort = 0b01000,
	ifs_domain_fault_section = 0b01001,
	ifs_domain_fault_page = 0b01011 ,
	ifs_translation_table_walk_lvl1_sync_ext_abt = 0b01100,
	ifs_permission_fault_section = 0b01101,
	ifs_translation_table_walk_lvl2_sync_ext_abt = 0b01110,
	ifs_permission_fault_page = 0b01111,
	ifs_imp_dep_lockdown = 0b10100,
	ifs_memory_access_sync_parity_err = 0b11001,
	ifs_imp_dep_coprocessor_abort = 0b11010,
	ifs_translation_table_walk_lvl1_sync_parity_err = 0b11100,
	ifs_translation_table_walk_lvl2_sync_parity_err = 0b11110,
};

/*Access is a two bit field
 *  00 = no access,
 *  01=client, 
 *  10=reserved, 
 *  11=manager
 */
/**
 * @brief Enum values for domain access type
 */
enum enum_access_type
{
	no_access = 0b00,
	client,
	reserved,
	manager
};
typedef enum enum_access_type access_type;

/**
 * @brief MMU initialization routine
 */
void mmu_init(void);

/**
 * @brief Set TTBR0 register
 *
 * @param addr Value to set for TTBR0 register
 */
void mmu_insert_pt0(sw_uint addr);

/**
 * @brief Set TTBR1 register
 *
 * @param addr Value to set for TTBR1 register
 */
void mmu_insert_pt1(sw_uint addr);

/**
 * @brief Set ASID register
 *
 * @param asid Value to set for ASID register
 */
void mmu_insert_asid(sw_uint asid);

/**
 * @brief Get TTBR0 register value
 *
 * @return TTBR0 register value
 */
sw_uint* mmu_get_pt0(void);

/**
 * @brief Get TTBR1 register value
 *
 * @return TTBR0 register value
 */
sw_uint* mmu_get_pt1(void);

/**
 * @brief Enable virtual address space for secure kernel
 */
void mmu_enable_virt_addr(void);

/**
 * @brief Disable virtual address space of secure kernel
 */
void mmu_disable_virt_addr(void);

/**
 * @brief Returns whether MMU is enabled or not
 *
 * @return MMU enabled flag
 */
sw_bool is_mmu_enabled(void);

/**
 * @brief Set access domain
 *
 * @param domain: Domain 
 * @param access: Domain access type 
 */
void set_domain(sw_short_int domain, access_type access);

/**
 * @brief Returns the physical address of virtual address based on secure world
 * page table
 *
 * @param va: Virtual address  
 * @return Physical address
 */
sw_phy_addr secure_vir_to_phy(sw_vir_addr va);

/**
 * @brief Returns the physical address of virtual address based on  
 * non-secure world page table
 *
 * @param va: Virtual address  
 * @return Physical address
 */
sw_phy_addr ns_vir_to_phy(sw_vir_addr va);

/**
* @brief Returns the virtual  address for the given physical address 
* based on secure world page table
*
* @param pa: Physical address
*
* @return: Mapped virtual address
*/
sw_vir_addr secure_phy_to_vir(sw_phy_addr pa);


/**
 * @brief Enable MMU for TEE
 * This function sets up the secure page table in TTBR0 or TTBR1 based on 
 * page table protection support. 
 * In case of page table protection support, set the ttbcr size.
 *
 */
void cpu_mmu_enable(void);

#endif /** _SW_MMU_HELPER_H */
