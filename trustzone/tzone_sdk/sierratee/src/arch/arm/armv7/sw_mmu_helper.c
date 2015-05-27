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
 * MMU helper functions  
 */
#include <cache.h>
#include <cpu.h>
#include <page_table.h>
#include <sw_mmu_helper.h>
#include <sw_mem_functions.h>
#include <cpu_data.h>
#include <global.h>

/**
 * @brief MMU initialization routine
 *
 */
void mmu_init(void)
{
	invoke_dmb();
	icache_inv_all();
	data_cache_clean_invalidate_all(0); /* Invalidate data cache */
	flush_all_translation_table_entries();
	invoke_dmb();
	invoke_isb();
	set_cp15_ttbcr(0);
}

/**
 * @brief Set access domain
 *
 * @param domain: Domain 
 * @param access: Domain access type 
 */
void set_domain(sw_short_int domain, access_type access)
{
	/* Domain is a two bit field 
	   00 = no access, 
	   01=client, 
	   10=reserved, 
	   11=manager 
	   */
	sw_uint value;
	value = get_cp15_dacr();  
	/* clear the current domain */
	sw_uint mask = ~(0x3 << (domain*2));
	value = value & mask;
	/* Set the domain */
	value = value | (access << ((domain)*2));
	set_cp15_dacr(value);
}

/**
 * @brief Set TTBR0 register
 *
 * @param addr Value to set for TTBR0 register
 */
void mmu_insert_pt0(sw_uint addr)
{
	if(addr)
		addr |= TTBR_FLAGS;
	set_cp15_ttbr0(addr);
}

/**
 * @brief Set TTBR1 register
 *
 * @param addr Value to set for TTBR1 register
 */
void mmu_insert_pt1(sw_uint addr)
{
	if(addr)
		addr |= TTBR_FLAGS;
	set_cp15_ttbr1((sw_uint)addr);
}


/**
 * @brief Get TTBR0 register value
 *
 * @return TTBR0 register value
 */
sw_uint* mmu_get_pt0()
{
	sw_uint regval = 0;
	regval = get_cp15_ttbr0();
	return (sw_uint*)regval;
}

/**
 * @brief Get TTBR1 register value
 *
 * @return TTBR0 register value
 */
sw_uint* mmu_get_pt1()
{
	sw_uint regval = 0;
	regval = get_cp15_ttbr1();
	return (sw_uint*)regval;
}


/**
 * @brief Enable virtual address space for secure kernel
 */
void mmu_enable_virt_addr()
{
	sw_uint reg_val;
	reg_val = get_cp15_sctlr();
	reg_val |= SCTLR_MMU_BIT;
	reg_val |= SCTLR_DCACHE_BIT;
	reg_val |= SCTLR_ICACHE_BIT;
	set_cp15_sctlr(reg_val);
}

/**
 * @brief Disable virtual address space of secure kernel
 */
void mmu_disable_virt_addr()
{
	sw_uint reg_val, mask;
	reg_val = get_cp15_sctlr();

	/* clear MMU, I-Cache and D-Cache */    
	mask = (SCTLR_MMU_BIT | SCTLR_DCACHE_BIT | SCTLR_ICACHE_BIT);
	reg_val = reg_val & (~mask);
	set_cp15_sctlr(reg_val);
}

/**
 * @brief Returns whether MMU is enabled or not
 *
 * @return MMU enabled flag
 */
sw_bool is_mmu_enabled()
{
	sw_uint reg_val;
	reg_val = get_cp15_sctlr();
	return (reg_val & SCTLR_MMU_BIT) ? TRUE : FALSE;
}

/**
 * @brief Returns the physical address of virtual address based on secure world
 * page table
 *
 * @param va: Virtual address  
 * @return Physical address
 */
sw_phy_addr secure_vir_to_phy(sw_vir_addr va)
{
	sw_phy_addr pa;
	asm volatile("mcr p15, 0, %0, c7, c8, 0\n"::"r"(va):"memory", "cc");


	asm volatile("isb \n\t" 
			" mrc p15, 0, %0, c7, c4, 0\n\t" 
			: "=r" (pa) : : "memory", "cc"); 

	return (pa & 0xfffff000) | (va & 0xfff);
}

/**
 * @brief Returns the physical address of virtual address based on  
 * non-secure world page table
 *
 * @param va: Virtual address  
 * @return Physical address
 */
sw_phy_addr ns_vir_to_phy(sw_vir_addr va)
{
	sw_phy_addr pa;
	asm volatile("mcr p15, 0, %0, c7, c8, 4\n"::"r"(va):"memory", "cc");


	asm volatile("isb \n\t" 
			" mrc p15, 0, %0, c7, c4, 0\n\t" 
			: "=r" (pa) : : "memory", "cc"); 

	return (pa & 0xfffff000) | (va & 0xfff);
}


sw_vir_addr secure_phy_to_vir(sw_phy_addr pa)
{
	sw_phy_addr pa_start = get_ram_start_addr();
	sw_vir_addr va_start = (sw_vir_addr )get_sw_code_start();
	return (sw_vir_addr)(va_start + (pa - pa_start));
}

/**
 * @brief Enable MMU for TEE
 * This function sets up the secure page table in TTBR0 or TTBR1 based on 
 * page table protection support. 
 * In case of page table protection support, set the ttbcr size.
 *
 */
void cpu_mmu_enable(void)
{

	sw_phy_addr pg_tbl_phy, tmp_tbl_phy;
	
	pg_tbl_phy = secure_vir_to_phy((sw_vir_addr)global_val.pagetable_addr);
	tmp_tbl_phy = secure_vir_to_phy((sw_vir_addr)tmp_page_table);
	mmu_init();
	mmu_insert_pt0(pg_tbl_phy);
	/*set the domain (access control) for the secure pages */
	set_domain(SECURE_ACCESS_DOMAIN, client);
	invoke_dsb();
	invoke_isb();
	mmu_enable_virt_addr();

}

/*
 * @Brief Set ASID on CP15
 */
void mmu_insert_asid(sw_uint asid){
	set_cp15_asid(asid);
}
