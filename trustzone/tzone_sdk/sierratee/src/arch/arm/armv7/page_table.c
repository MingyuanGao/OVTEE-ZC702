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
 * It contains the implementation of page table helper routines.
 * Routines to create the section, small page entries in page table.
 * Routines to create the mapping to access non-secure memory.
 * Routines to create the secure kernel or user access mapping routines
 */

#include <sw_types.h>
#include <sw_debug.h>
#include <sw_mem_functions.h>
#include <sw_heap.h>
#include <cache.h>
#include <sw_mmu_helper.h>
#include <page_table.h>
#include <mem_mng.h>
#include <global.h>
#include <sw_link.h>
#include <uart.h>
#include <sw_board.h>
#include <cpu_data.h>
#include <debug_config.h>

/* Static 2nd level page table entries
 * This is needed before we setup the memory pool manager and kernel heap
 * This entries need to be adjusted based on the initial page table entries
 * which we create before the allocator. 
 * After the creation of kernel heap, second level page tables are allocated 
 * from kernel heap.
 */
#define MAX_STATIC_L2_CNT 8
static second_level_pages static_l2_ref[MAX_STATIC_L2_CNT];
static sw_short_int static_l2_page[MAX_STATIC_L2_CNT][PAGE_SIZE] __attribute__((aligned(1024)));
static sw_short_int static_l2_next_idx = 0;
struct l2_pg l2_pages;
/**
 * @brief Get secure page table pointer
 *
 * This function returns the page table pointer
 *
 * @return Pointer to the page table
 */
sw_uint* get_secure_ptd(void)
{
	return (sw_uint*)secure_page_table;
}


/**
* @brief Get temp page table pointer
* 
* This function returns the temp page table pointer
*
* @return Pointer to the temp page table
*/
sw_uint* get_tmp_ptd(void)
{
	return (sw_uint*)tmp_page_table;
}

/**
 * @brief Initialize page table entries
 *
 * This function creates the initial page table entries for secure kernel
 * @return Pointer to the page table
 */
sw_uint* map_secure_page_table(void)
{
	link_init(&l2_pages.list);
	/* Statically allocated l2 pages */
	l2_pages.blk_allocated = PAGE_SIZE/L2_PAGE_TABLE_SIZE;
	l2_pages.blk_used = 0;
	sw_memset((void*)secure_page_table, 0, sizeof(secure_page_table));
	board_map_secure_page_table(secure_page_table);
	return (sw_uint*)secure_page_table;
}


/**
 * @brief Create shared memory mapping between secure and non-secure kernel
 *
 * This function creates the page table entry with ns bit set. So that 
 * this section of the non-secure memory act like a shared memory.
 *
 * @param phy_addr: Physical address of the non-secure memory
 * @param va_addr:  Virtual address of the shared memory
 *
 * @return 
 * SW_OK \n
 * SW_ERROR* \n
 */
int __map_to_ns(sw_phy_addr phy_addr, sw_vir_addr *va_addr)
{
	int ret_val;
	struct cpu_pt_entry desc;
	desc.va =  phy_addr;
	desc.pa =  phy_addr;
	desc.dom = SECURE_ACCESS_DOMAIN;
	desc.ap = PRIV_RW_USR_RW;
	desc.xn = PAGE_DESC_XN_ALLOW;
	desc.tex = 0x1; 
	desc.c = 0x1;
	desc.b = 0x1;  
	desc.ns = 1;
	desc.ng = 0;
#ifdef CONFIG_SW_MULTICORE
	desc.s = 1;
#else
	desc.s = 0;
#endif

	ret_val = map_ns_section_entry(get_secure_ptd(), &desc);
	if(!ret_val) {
		*va_addr = desc.va;
		return SW_OK;
	}
	return SW_ERROR;
}


/**
 * @brief 
 *  Wrapper function for __map_to_ns to call from kernel routines
 * @param phy_addr: Physical address of the non-secure memory
 * @param va_addr:  Virtual address of the shared memory
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */
int map_to_ns(sw_phy_addr phy_addr, sw_vir_addr *va_addr)
{
	return  __map_to_ns(phy_addr,va_addr);
}

/**
 * @brief Map device entry in secure page table
 * 
 * Creates device mapping entry in secure page table.
 * This function assumes that the start address and size are page aligned. 
 * It the size of the mapping greater than or equal to section size and
 * the address is aligned to section, create the section entries and rest
 * of them are created as small page mapping. 
 *
 * @param va: virtual address
 * @param pa: physical address
 * @size size: size
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */
int map_device(sw_vir_addr va, sw_phy_addr pa, sw_uint size)
{
	struct cpu_pt_entry desc;
	desc.dom = SECURE_ACCESS_DOMAIN;
	desc.ap = PRIV_RW_USR_NO;
	desc.xn = PAGE_DESC_XN_NEVER;
	/* 
	 * Device 
	 */
	desc.tex = 0x0; 
	desc.c = 0x0;
	desc.b = 0x1;  
	desc.ns = 0;
	desc.ng = 0;
#ifdef CONFIG_SW_MULTICORE
	desc.s = 1;
#else
	desc.s = 0;
#endif

	desc.va = va;
	desc.pa = pa;
	while(size) {
		desc.va =  va;
		desc.pa =  pa;
		if(size >= SECTION_SIZE && 
				!(va & (SECTION_SIZE - 1)) &&
				!(pa & (SECTION_SIZE - 1))){
			map_section_entry(get_secure_ptd(), &desc);
			va += SECTION_SIZE;
			pa += SECTION_SIZE;
			size -= SECTION_SIZE;
		}else{
			map_small_page_entry(get_secure_ptd(), &desc);
			va += PAGE_SIZE;
			pa += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
	}
	return SW_OK;
}


/**
 * @brief List of device map entry to map in secure page table
 *
 * This function iterates the device table and map each device in secure 
 * page table.
 *
 * @param dt: pointer to array of device mappings
 *            Array has to terminated with zero.
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */

int map_device_table(const struct devmap *dt)
{
	while (*(sw_uint *)dt != 0) {
		map_device(dt->dv_va, dt->dv_pa, dt->dv_size);
		dt++;
	};
	return SW_OK;
}

/**
 * @brief Map a range of secure memory area in secure page table
 *
 * This function maps the memory as global entry and permission based on the 
 * access flag. 
 * This function assumes that the start address and size are page aligned. 
 * It the size of the mapping greater than or equal to section size and
 * the address is aligned to section, create the section entries and rest
 * of them are created as small page mapping. 

 * @param va: Virtual address to be mapped 
 * @param pa: Physical address to be mapped
 * @param size: Size of the mapped memory
 * @param flags: flags for access permission and exec
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */

int map_secure_memory(sw_vir_addr va, sw_phy_addr pa, sw_uint size, sw_uint ptf)
{
	struct cpu_pt_entry desc;

	if (size & (PAGE_SIZE - 1)) {
		sw_seterrno(SW_EINVAL);
		goto error;
	}

	if (va & (PAGE_SIZE - 1)) {
		sw_seterrno(SW_EINVAL);
		goto error;
	}

	if (pa & (PAGE_SIZE -1)) {
		sw_seterrno(SW_EINVAL);
		goto error;
	}


	desc.dom = SECURE_ACCESS_DOMAIN;
	desc.ap = ptf_to_ap(ptf);
	desc.xn = ptf_to_xn(ptf);
	/* Default memory attributes:
	 * - Outer and inner cache write back and write allocate
	 * - Global 
	 * - sharable attribute based on the number of cores
	 */
	desc.tex = 0x1; 
	desc.c = 0x1;
	desc.b = 0x1;  
	desc.ns = 0;
	desc.ng = 0;
#ifdef CONFIG_SW_MULTICORE
	desc.s = 1;
#else
	desc.s = 0;
#endif

	while(size) {
		desc.va =  va;
		desc.pa =  pa;
		if((size >= SECTION_SIZE) && 
				!(va & (SECTION_SIZE - 1)) &&
				!(pa & (SECTION_SIZE - 1))){
			map_section_entry(get_secure_ptd(), &desc);
			va += SECTION_SIZE;
			pa += SECTION_SIZE;
			size -= SECTION_SIZE;
		}else{
			map_small_page_entry(get_secure_ptd(), &desc);
			va += PAGE_SIZE;
			pa += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
	}

	return SW_OK;

error:
	return SW_ERROR;
}

/**
 * @brief Map a range of memory area as kernel text region in secure page table
 *
 * This function assumes that the start address and size are page aligned.
 *
 * @param va: virtual address to be mapped in kernel page table
 * @param pa: physical address to be mapped in kernel page table
 * @param size: size of the mapping
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */

int map_kernel_text_memory(sw_vir_addr va, sw_phy_addr pa, sw_uint size)
{
	return map_secure_memory(va, pa, size, PTF_PROT_KRO| PTF_EXEC);
}

/**
 * @brief Map a range of memory area as kernel data region in secure page table
 *
 * This function assumes that the start address and size are page aligned.
 *
 * @param va: virtual address to be mapped in kernel page table
 * @param pa: physical address to be mapped in kernel page table
 * @param size: size of the mapping
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */

int map_kernel_data_memory(sw_vir_addr va, sw_phy_addr pa, sw_uint size)
{
	return map_secure_memory(va, pa, size, PTF_PROT_KRW);
}


/**
 * @brief Map a range of memory area as non-global text region in supplied page table
 *
 * This function assumes that the start address and size are aligned.
 *
 * @param va: Virtual address to be mapped with user access and execute permission
 * @param pa: Physical address to be mapped
 * @param size: Size of the mapped memory
 * @param user_pt_base: Physical address of the page table
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */

int map_user_text_memory(sw_vir_addr va, sw_phy_addr pa, sw_uint size,
		sw_phy_addr user_pt_base)
{
	return map_user_memory(va, pa, size, PTF_PROT_URO| PTF_EXEC, user_pt_base);
}

/**
 * @brief Map a range of memory area as non-global data region in supplied page table
 *
 * This function assumes that the start address and size are page aligned.
 *
 * @param va: virtual address
 * @param pa: physical address
 * @param size: size to map
 *
 * @return otz_return_t:
 * OTZ_OK \n
 * OTZ_FAIL \n
 */

int map_user_data_memory(sw_vir_addr va, sw_phy_addr pa, sw_uint size,
		sw_phy_addr user_pt_base)
{
	return map_user_memory(va, pa, size, PTF_PROT_URW, user_pt_base);
}

/**
 * @brief Map a range of memory area as non-global data region in supplied page table
 *
 * This function assumes that the start address and size are page aligned.
 *
 * @param va: virtual address
 * @param pa: physical address
 * @param size: size to map
 *
 * @return otz_return_t:
 * OTZ_OK \n
 * OTZ_FAIL \n
 */

int map_user_rodata_memory(sw_vir_addr va, sw_phy_addr pa, sw_uint size,
		sw_phy_addr user_pt_base)
{
	return map_user_memory(va, pa, size, PTF_PROT_URO, user_pt_base);
}

/**
 * @brief Map a range of memory area as non-global data region in supplied page table
 *
 * This function assumes that the start address and size are page aligned.
 * It the size of the mapping greater than or equal to section size and
 * the address is aligned to section, create the section entries and rest
 * of them are created as small page mapping. 
 * 
 * @param va: Virtual address to be mapped with user read/write access permission
 * @param pa: Physical address to be mapped
 * @param size: Size of the mapped memory
 * @param user_pt_base: Physical address of the page table
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */

int map_user_memory(sw_vir_addr va, sw_phy_addr pa, sw_uint size, 
		sw_uint ptf, sw_phy_addr user_pt_base)
{
	struct cpu_pt_entry desc;

	if (size & (PAGE_SIZE - 1)) {
		sw_seterrno(SW_EINVAL);
		goto error;
	}

	if (va & (PAGE_SIZE - 1)) {
		sw_seterrno(SW_EINVAL);
		goto error;
	}

	if (pa & (PAGE_SIZE -1)) {
		sw_seterrno(SW_EINVAL);
		goto error;
	}

	desc.dom = SECURE_ACCESS_DOMAIN;
	desc.ap = ptf_to_ap(ptf);
	desc.xn = ptf_to_xn(ptf);	
	/* Default memory attributes:
	 * - Outer and inner cache write back and write allocate
	 * - Global 
	 * - sharable attribute based on the number of cores
	 */
	desc.tex = 0x1; 
	desc.c = 0x1;
	desc.b = 0x1;  
	desc.ns = 0;
	desc.ng = 1;
#ifdef CONFIG_SW_MULTICORE
	desc.s = 1;
#else
	desc.s = 0;
#endif

	while(size) {
		desc.va =  va;
		desc.pa =  pa;
		if((size >= SECTION_SIZE) && 
				!(va & (SECTION_SIZE - 1)) &&
				!(pa & (SECTION_SIZE - 1))){
			map_section_entry((sw_uint *)user_pt_base, &desc);
			va += SECTION_SIZE;
			pa += SECTION_SIZE;
			size -= SECTION_SIZE;
		}else{
			map_small_page_entry((sw_uint *)user_pt_base, &desc);
			va += PAGE_SIZE;
			pa += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
	}

	return SW_OK;

error:
	return SW_ERROR;
}

/**
 * @brief Map a range of non secure memory area as normal memory with 
 * attributes Outer and Inner write back, write allocate. 
 *
 * This function assumes that the start address and size are section aligned. 
 *
 * @param va: virtual address
 * @param pa: physical address
 * @param size: size
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */

int map_nsmemsect_normal(sw_vir_addr va, sw_phy_addr pa, sw_uint size)
{
	struct cpu_pt_entry desc;

	if (size & (SECTION_SIZE - 1)) {
		sw_seterrno(SW_EINVAL);
		goto error;
	}

	if (va & (SECTION_SIZE - 1)) {
		sw_seterrno(SW_EINVAL);
		goto error;
	}

	if (pa & (SECTION_SIZE -1)) {
		sw_seterrno(SW_EINVAL);
		goto error;
	}

	/* Normal cacheable memory */
	desc.dom = SECURE_ACCESS_DOMAIN;
	desc.ap = PRIV_RW_USR_NO;
	desc.xn = PAGE_DESC_XN_NEVER;
	/* Outer and inner cache write back and write allocate */
	desc.tex = 0x1; 
	desc.c = 0x1;
	desc.b = 0x1;  
	desc.ns = 1;
	desc.ng = 1;
	desc.s = 0;

	while(size) {
		desc.va =  va;
		desc.pa =  pa;

			map_ns_section_entry(get_secure_ptd(), &desc);

		va += SECTION_SIZE;
		pa += SECTION_SIZE;
		size -= SECTION_SIZE;
	}
	return SW_OK;

error:
	return SW_ERROR;

}


/**
 * @brief Unmap a range of non secure memory area 
 *
 * This function assumes that the start address and size are section aligned. 
 *
 * @param va: virtual address
 * @param size: size
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */

int unmap_nsmemsect_normal(sw_vir_addr va, sw_uint size)
{

	if (size & (SECTION_SIZE - 1)) {
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}

	if (va & (SECTION_SIZE - 1)) {
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}

	while(size) {
		/* Normal cacheable memory */
		unmap_ns_section_entry(get_secure_ptd(), va);

		va += SECTION_SIZE;
		size -= SECTION_SIZE;
	}
	return SW_OK;
}

/**
 * @brief Create a section map entry
 *
 * This function creates section map entry in supplied page table
 *
 * @param pgd: Pointer to page table
 * @param entry: Pointer to section entry
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */
int map_section_entry(sw_uint *pgd, struct cpu_pt_entry *entry)
{
	sw_uint l1_index = 0;
	sw_uint *l1_pte;

	l1_index = entry->va >> 20;
	l1_pte = pgd + l1_index;

	switch ((*l1_pte) & PAGE_DESC_TYPE_MASK) {
		case (PAGE_DESC_TYPE_FAULT):
			*l1_pte = ( entry->pa & PAGE_DESC_SEC_MASK) | PAGE_DESC_TYPE_SECTION;
			*l1_pte |= (entry->dom << PAGE_DESC_DOMAIN_SHIFT);
			*l1_pte |= (entry->b << PAGE_DESC_B_SHIFT);
			*l1_pte |= (entry->c << PAGE_DESC_C_SHIFT);
			*l1_pte |= (entry->tex << PAGE_DESC_L1_TEX_SHIFT);
			*l1_pte |= (entry->xn << PAGE_DESC_SEC_XN_SHIFT);
			*l1_pte |= ((entry->ap & PAGE_DESC_AP_MASK) << PAGE_DESC_L1_AP_SHIFT);
			*l1_pte |= (((entry->ap >> 2) & 0x1) << PAGE_DESC_L1_AP2_SHIFT);
			*l1_pte |= (entry->s << PAGE_DESC_L1_S_SHIFT);
			*l1_pte |= (entry->ng << PAGE_DESC_L1_NG_SHIFT);
			*l1_pte |= (entry->ns << PAGE_DESC_L1_SEC_NS_SHIFT);

			/*		
				flush is not needed - page table is cacheable 
			 */
			break;
		case (PAGE_DESC_TYPE_SECTION):
			/*
			   Change in flags need to be handled  
			 */
			break;
		default:
			sw_printk("SW: section_map_hsw_vir_addro_hpa: This function supports "
					"only section mappings.\n");
			sw_printk("SW:   L1 descriptor: %08x\n", *l1_pte);
			sw_seterrno(SW_EINVAL);
			return SW_ERROR;
	}

	return SW_OK;
}

/**
 * @brief Map the non-secure memory in secure page table for shared memory 
 * access
 *
 * This function creates the section mapping with 'NS' bit set to create a 
 * shared memory access.
 *
 * @param pgd: Pointer to page table
 * @param entry: Pointer to section entry
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */
int map_ns_section_entry(sw_uint *pgd, struct cpu_pt_entry *entry)
{
	sw_uint l1_index = 0;
	sw_uint *l1_pte;

	l1_index = entry->va >> 20;
	l1_pte = pgd + l1_index;

	switch ((*l1_pte) & PAGE_DESC_TYPE_MASK) {
		case (PAGE_DESC_TYPE_FAULT):
			*l1_pte = ( entry->pa & PAGE_DESC_SEC_MASK) | PAGE_DESC_TYPE_SECTION;
			*l1_pte |= (entry->dom << PAGE_DESC_DOMAIN_SHIFT);
			*l1_pte |= (entry->b << PAGE_DESC_B_SHIFT);
			*l1_pte |= (entry->c << PAGE_DESC_C_SHIFT);
			*l1_pte |= (entry->tex << PAGE_DESC_L1_TEX_SHIFT);
			*l1_pte |= (entry->xn << PAGE_DESC_SEC_XN_SHIFT);
			*l1_pte |= ((entry->ap & PAGE_DESC_AP_MASK) << PAGE_DESC_L1_AP_SHIFT);
			*l1_pte |= (((entry->ap >> 2) & 0x1) << PAGE_DESC_L1_AP2_SHIFT);
			*l1_pte |= (entry->s << PAGE_DESC_L1_S_SHIFT);
			*l1_pte |= (entry->ng << PAGE_DESC_L1_NG_SHIFT);
			*l1_pte |= (entry->ns << PAGE_DESC_L1_SEC_NS_SHIFT);
					
			/*		
				flush is not needed - page table is cacheable 
			 */
			page_ref_add((sw_uint)l1_pte);
			break;
		case (PAGE_DESC_TYPE_SECTION):
			if(!(*l1_pte & PAGE_DESC_L1_SEC_NS_MASK_ENTRY)) {
				*l1_pte |= (1 << PAGE_DESC_L1_SEC_NS_SHIFT);
				/*		
					flush is not needed - page table is cacheable 
				*/
			}
			page_ref_add((sw_uint)l1_pte);
			break;
		default:
			sw_printk("SW: section_map_hsw_vir_addro_hpa: This function supports "
					"only section mappings.\n");
			sw_printk("SW:   L1 descriptor: %08x\n", *l1_pte);
			sw_seterrno(SW_EINVAL);
			return SW_ERROR;
	}

	return SW_OK;
}

/**
 * @brief Unmap section entry from the supplied page table
 *
 * @param pgd: Pointer to the page table
 * @param va: Virtual address need to be unmapped
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */
static int unmap_section_entry(sw_uint* pgd, sw_vir_addr va)
{
	sw_uint l1_index = 0;
	sw_uint* l1_pte;

	l1_index = va >> 20;

	l1_pte = pgd + l1_index;

	switch((*l1_pte) & PAGE_DESC_TYPE_MASK) {
		case (PAGE_DESC_TYPE_FAULT):
			break;
		case (PAGE_DESC_TYPE_SECTION):
			*l1_pte = 0;
			/*		
				flush is not needed - page table is cacheable 
			 */
			inv_translation_table_by_va_asid(va);			 
			break;
		default :
			break;
	}
	return SW_OK;
}

/**
 * @brief Unmap the shared section entry from secure page table
 *
 * This function unmaps the secure memory section from secure page table. 
 * i.e. set 'NS' bit to zero
 *
 * @param pgd: Pointer to page table
 * @param va: Virtual address to be unmapped
 *
 * @return otz_return_t:
 * @return:
 * SW_OK \n
 */
int unmap_ns_section_entry(sw_uint *pgd, sw_vir_addr va)
{
	sw_uint l1_index = 0;
	sw_uint *l1_pte;

	l1_index = va >> 20;
	l1_pte = pgd + l1_index;

	switch ((*l1_pte) & PAGE_DESC_TYPE_MASK) {
		case (PAGE_DESC_TYPE_FAULT):
			break;
		case (PAGE_DESC_TYPE_SECTION):
			if(page_ref_release((sw_uint)l1_pte) == 0) {
				if((*l1_pte & PAGE_DESC_L1_SEC_NS_MASK_ENTRY)) {
					*l1_pte = 0;
					/*		
						flush is not needed - page table is cacheable 
					 */
				}
			}
			break;
		default:
			break;
	}
	return SW_OK;
}


/**
 * @brief Unmap a range of kernel memory area
 *
 * This function unmaps the region which are mapped as global in secure page table.
 * This function assumes that the start address and size are aligned.
 *
 * @param va: Virtual address to be un-mapped 
 * @param size: Size of the un-mapped memory
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */

int unmap_kernel_memory(sw_vir_addr va, sw_uint size)
{
	return unmap_secure_memory(va, size);
}

/**
 * @brief Unmap a range of memory from secure page table.
 *
 * This function assumes that the start address and size are page aligned. 
 * It the size of the mapping greater than or equal to section size and
 * the address is aligned to section, invoke the section unmap routine
 * otherwise invoke the small page unmap routine.
 *
 * @param va: Virtual address to be un-mapped 
 * @param size: Size of the un-mapped memory
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */

int unmap_secure_memory(sw_vir_addr va, sw_uint size)
{

	if (size & (PAGE_SIZE - 1)) {
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}

	if (va & (PAGE_SIZE - 1)) {
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}

	while(size) {
		if((size >= SECTION_SIZE) && 
				!(va & (SECTION_SIZE - 1))){
			unmap_section_entry(get_secure_ptd(), va);
			va += SECTION_SIZE;
			size -= SECTION_SIZE;
		}else{
			unmap_small_page_entry(get_secure_ptd(), va);
			va += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
	}
	return SW_OK;
}

/**
 * @brief Unmap a range of memory which are mapped as non-global and 
 * user mode permission from the supplied page table
 *
 * This function assumes that the start address and size are aligned.
 * It the size of the mapping greater than or equal to section size and
 * the address is aligned to section, invoke the section unmap routine
 * otherwise invoke the small page unmap routine.
 
 * @param va: Virtual address to be un-mapped 
 * @param size: Size of the un-mapped memory
 * @param user_pt_base: Physical address of the page table 
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */

int unmap_user_memory(sw_vir_addr va, sw_uint size, sw_phy_addr user_pt_base)
{

	if (size & (PAGE_SIZE - 1)) {
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}

	if (va & (PAGE_SIZE - 1)) {
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}

	while(size) {
		if((size >= SECTION_SIZE) && 
				!(va & (SECTION_SIZE - 1))){
			unmap_section_entry((sw_uint *)user_pt_base, va);
			va += SECTION_SIZE;
			size -= SECTION_SIZE;
		}else{
			unmap_small_page_entry((sw_uint *)user_pt_base, va);
			va += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
	}
	return SW_OK;
}

/**
 * @brief Unmap the non-secure memory from secure page table
 *
 * This function unmaps the non-secure memory from secure page table. 
 * i.e. set 'NS' bit to zero
 *
 * @param va_addr: Virtual address to be unmapped
 *
 * @return:
 * SW_OK \n
 */
int __unmap_from_ns(sw_vir_addr va_addr)
{
	int ret_val;
	ret_val = unmap_ns_section_entry(get_secure_ptd(), va_addr);
	return ret_val;
}

/**
 * @brief 
 *      Wrapper function for __unmap_from_ns to call from kernel routines.
 * @param va_addr:  Virtual address of the shared memory
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */
int unmap_from_ns(sw_vir_addr va_addr)
{
	return __unmap_from_ns(va_addr);
}


/**
 * @brief Increment page table reference of shared memory
 *
 * This function increment the reference count of mapped shared memory
 *
 * @param l1_pte: Page table entry
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */
int page_ref_add(sw_uint l1_pte)
{
	struct link *l, *head;
	struct sw_global *pglobal = &global_val;
	struct sw_page_ref *page_ref = NULL;
	sw_bool found = FALSE;


	if (!link_empty(&pglobal->page_ref_list)) {
		head= &pglobal->page_ref_list;
		l = head->next;
		while (l != head) {
			page_ref = l->data;
			if(page_ref) {
				if (page_ref->l1_pte == l1_pte) {
					found = TRUE;
					break;
				}
			}
			l = l->next;
		}
	}

	if (!found) {
		page_ref = (struct sw_page_ref*)sw_malloc_private(COMMON_HEAP_ID,
				sizeof(struct sw_page_ref));
		if(!page_ref) {
			sw_printk("SW: page reference allocation: malloc failed\n");
			sw_seterrno(SW_ENOMEM);
			return SW_ERROR; 
		}
		page_ref->l1_pte = l1_pte;
		page_ref->ref_cnt = 0;
		link_init(&page_ref->head);
		set_link_data(&page_ref->head, page_ref);
		add_link(&pglobal->page_ref_list, &page_ref->head, TAIL);
	} 

	page_ref->ref_cnt++;
	return SW_OK;
}

/**
 * @brief Decrement page table reference of shared memory
 *
 * This function decrement the reference count of mapped shared memory
 *
 * @param l1_pte: Page table entry
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */
int page_ref_release(sw_uint l1_pte)
{   
	int ref_cnt;
	struct link *l, *head;
	struct sw_global *pglobal = &global_val;
	struct sw_page_ref *page_ref = NULL;
	sw_bool found = FALSE;

	if (!link_empty(&pglobal->page_ref_list)) {
		head= &pglobal->page_ref_list;
		l = head->next;
		while (l != head) {
			page_ref = l->data;
			if(page_ref) {
				if (page_ref->l1_pte == l1_pte) {
					found = TRUE;
					break;
				}
			}
			l = l->next;
		}
	}

	if (!found) {
		sw_printk("SW: no page reference found\n");
		sw_seterrno(SW_ENODATA);
		return SW_ERROR;
	} 

	page_ref->ref_cnt--;

	ref_cnt = page_ref->ref_cnt;

	if(page_ref->ref_cnt == 0)
	{
		remove_link(&page_ref->head);  
		sw_free_private(COMMON_HEAP_ID, page_ref);
	}
	return ref_cnt;   
}

/**
* @brief Dump the virtual address
*
* @param i: entry
*/
static void dump_virt_addr(sw_uint i)
{
	sw_printk("%08x", (i << 20));
}

/**
* @brief Dump super section entry
*
* @param sd: Super section entry
*/
static void dump_super_section(sw_uint* sd)
{
	sw_printk("SUPERSECTION\n");
	tee_panic("UNIMPLEMENTED: dump_super_section");
}

/**
* @brief Dump 4K page entry
*
* @param sd: Small page descriptor
*/
static void dump_small_page(sw_uint* sd)
{
	sw_uint entry = *sd;
	sw_uint access_bits = 
		(((entry & PAGE_DESC_L2_AP2_MASK_ENTRY) >> 
			PAGE_DESC_L2_AP2_SHIFT) << 2) | 
		((entry & PAGE_DESC_L2_AP_MASK_ENTRY) >> PAGE_DESC_L2_AP_SHIFT);
	sw_printk("SMALL PAGE Phys Addr: %08x c: %x, b: %x, access bits: %x \
shared bit 0x%x\n",
			(entry & 0xfffff000), (entry & 0x8) >> 3, (entry & 0x4) >> 2, 
			access_bits, (entry & 0x400));
}

/**
* @brief Dump the 64K page entry
*
* @param ld: Long page descriptor
*/
static void dump_large_page(sw_uint* ld)
{
	sw_uint entry = *ld;
	sw_uint access_bits = 
		(((entry & PAGE_DESC_L2_AP2_MASK_ENTRY) >> 
			PAGE_DESC_L2_AP2_SHIFT) << 2) | 
		((entry & PAGE_DESC_L2_AP_MASK_ENTRY) >> PAGE_DESC_L2_AP_SHIFT);
	sw_printk("LARGE PAGE Phys Addr: %08x access bits: %x\n", 
			(entry & 0xffff0000), access_bits);
}

/**
* @brief Dump second level page table virtual address
*
* @param virtual
* @param i
* @param page_size
*/
static void dump_second_virt_addr(sw_uint virtual, sw_uint i, sw_uint page_size)
{

	switch (page_size) {
		case 1:
			virtual += i << 16; // 16 bits -> 64KB    
			break;
		case 2:
			virtual += i << 12; // 12 bits -> 4KB 
			break;
		case 3:
			virtual += i << 10; // 10 bits -> 1KB
			break;
		default:
			tee_panic("dump_second_virt_addr: invalid page size.");
	} 
	sw_printk("    %08x ", virtual);
}

/**
* @brief Dumps the second level page table
*
* @param ptd: Pointer to second level page table
* @param virtual: Virtual address 
*/
static void dump_second_level(sw_uint* ptd, sw_uint virtual)
{
	sw_uint *current_ptd;
	sw_uint l2_entry;
	sw_uint base_addr = (*ptd) & 0xFFFFFC00;
	sw_printk("    SUB PAGE TABLE ptd 0x%x\n", (*ptd) >> 10);
	sw_printk("    SUB PAGE TABLE domain 0x%x base addr 0x%x\n", 
			(((*ptd) & 0x1e0) >> 5), base_addr);

	sw_printk("    Virtual Addr | TYPE | DETAILS\n");
	sw_printk("    -----------------------------\n");
	current_ptd = (sw_uint*)secure_phy_to_vir(base_addr);
	int i = 0;
	for(i = 0; i < 256 ; i++) {
		l2_entry = *current_ptd;
		switch(l2_entry & PAGE_DESC_TYPE_MASK) {
			case PAGE_DESC_TYPE_FAULT:
				/* no entry here */
				break;
			case PAGE_DESC_TYPE_LARGE:
				dump_second_virt_addr(virtual, i, 1);
				dump_large_page(current_ptd);
				break;
			case PAGE_DESC_TYPE_SMALL:
			case 0x3:
				dump_second_virt_addr(virtual, i, 2); //have to change
				dump_small_page(current_ptd);
				break;
			case 0x11:
				sw_printk("tiny small page is not supported\n");
				break;
			default:
				tee_panic("dump_second_level: INVALID pt entry type.");
		}
		current_ptd++;
	}
}

/**
* @brief Dump the section entries of the page table
*
* @param sd: Section entry
*/
static void dump_section(sw_uint* sd)
{
	sw_uint entry = *sd;
	if((entry & 0x40000) != 0) {
		dump_super_section(sd);
		return;
	}

	sw_uint access_bits = (((entry & PAGE_DESC_L1_AP2_MASK_ENTRY) 
				>> PAGE_DESC_L1_AP2_SHIFT) << 2) | 
		((entry & PAGE_DESC_L1_AP_MASK_ENTRY) 
		 >> PAGE_DESC_L1_AP_SHIFT);
	sw_printk("SECTION PhysAddr: %08x domain: %x, accessBits: %x, nG: %x\n",
			(entry & 0xfff00000), (entry & PAGE_DESC_DOMAIN_MASK) >> PAGE_DESC_DOMAIN_SHIFT, 
			access_bits, ((entry & 0x20000) >> 17));
}

/**
* @brief Dump page table entries of the supplied page table
*
* @param ptd: Pointer to page table
*/
void dump_page_table(sw_uint* ptd)
{
#ifndef DEBUG_PAGETABLE
	sw_printk("page table entries: %x\n", PAGE_TABLE_ENTRIES);
	sw_printk("VIRTUAL ADDR | TYPE | DETAILS\n");
	sw_printk("-----------------------------\n");
	sw_printk("ptd addr: %08x\n", (sw_uint)ptd);

	ptd = (sw_uint*)secure_phy_to_vir((sw_phy_addr )ptd);

	sw_uint* temp_ptd = ptd;
	sw_uint i;
	sw_uint l1_entry;

	for(i=0; i < PAGE_TABLE_ENTRIES; i++){
		l1_entry = *(temp_ptd);
		sw_uint l1_type = l1_entry & PAGE_DESC_TYPE_MASK;
		switch(l1_type)
		{
			case PAGE_DESC_TYPE_FAULT:
				break;
			case PAGE_DESC_TYPE_COARSE:
				dump_virt_addr(i);
				sw_printk("-%08x Entry for 2nd level table: %08x\n", 
						(((i+1) << 20)-1), *(sw_uint*)temp_ptd);
				dump_second_level(temp_ptd, (i << 20));
				break;
			case PAGE_DESC_TYPE_SECTION:
				dump_virt_addr(i);
				sw_printk(" ");
				dump_section(temp_ptd);
				break;
			case 0x3:
				dump_virt_addr(i);
				sw_printk(" RESERVED\n");
				break;
			default:
				tee_panic("dump_page_table: INVALID pt entry type.");
		}
		temp_ptd++;
	}
#endif
}


/**
 * @brief Create second level page table
 *
 * This function creates the second level page table either from 
 * static page table or from kernel heap.
 * This also update the tracking list of second level page tables.
 *
 * @param va
 * @param l2_pt
 * @param l2_pt_vir
 */
void alloc_l2_secure_page_table(sw_uint va, sw_uint* l2_pt, sw_uint* l2_pt_vir)
{
	sw_uint index, found = FALSE;
	second_level_pages *l2_ref = NULL;
	struct link *l, *head;
	
	head= &l2_pages.list;
	l = head->next;
	while (l != head) {
		l2_ref = l->data;
		if(l2_ref && l2_ref->blk_used != (PAGE_SIZE/L2_PAGE_TABLE_SIZE)) {
			for(index = 0; index < (PAGE_SIZE/L2_PAGE_TABLE_SIZE); index++) {
				if(l2_ref->page[index] == 0){
					found = TRUE;
					l2_ref->blk_used++;
					l2_pages.blk_used++;
					break;
				}
			}
		}
		if(found == TRUE)
			break;
		l = l->next;
	}
	
	if(l2_ref == NULL){
		l2_ref = (second_level_pages *)&static_l2_ref;
		sw_memset((void*)l2_ref, 0, sizeof(second_level_pages));
		l2_ref->page_base = (sw_uint)&static_l2_page[0];
		link_init(&l2_ref->head_ref);
		l2_ref->blk_used++;
		l2_pages.blk_used++;
		set_link_data(&l2_ref->head_ref, l2_ref);
		add_link(&l2_pages.list, &l2_ref->head_ref, TAIL);
		index = 0;
		static_l2_next_idx++;
	}

	*l2_pt_vir = (sw_uint)(l2_ref->page_base + (index * L2_PAGE_TABLE_SIZE));

	if(is_mmu_enabled()){
		*l2_pt = secure_vir_to_phy(*l2_pt_vir);
	}else
		*l2_pt = *l2_pt_vir;

	sw_memset((void*)*l2_pt_vir, 0, L2_PAGE_TABLE_SIZE);
}

/**
 * @brief Map the second level page table entry in supplied page table
 *
 * Create a second level page entry in the supplied page table.
 * @param pgd: Pointer to the page table
 * @param entry: Page table entry structure based on the short descriptor format
 *
 * @return 
 * SW_OK \n
 * SW_ERROR* \n
 */
int map_small_page_entry(sw_uint *pgd, struct cpu_pt_entry *entry){
	sw_uint l1_index = 0;
	sw_uint *l1_pte, l2_base, l2_base_vir, *l2_pte;
	sw_uint index=0, found = FALSE;
	second_level_pages *l2_ref = NULL;
	struct link *l, *head;
	static sw_uint skip = FALSE;

	l1_index = entry->va >> 20;
	l1_pte = pgd + l1_index;

	switch ((*l1_pte) & PAGE_DESC_TYPE_MASK) {
		case PAGE_DESC_TYPE_FAULT:
			alloc_l2_secure_page_table(entry->va, &l2_base, &l2_base_vir);

			*l1_pte = l2_base;
			*l1_pte = (*l1_pte & PAGE_DESC_PAGE_TBL_MASK) | PAGE_DESC_TYPE_COARSE;
			*l1_pte |= (entry->dom << PAGE_DESC_DOMAIN_SHIFT);
			*l1_pte |= (entry->ns  << PAGE_DESC_L1_PAGE_NS_SHIFT);
			break;
		case PAGE_DESC_TYPE_COARSE:
			/* Update the domain of the L1 mapping */

			*l1_pte &= ~PAGE_DESC_DOMAIN_MASK_ENTRY;
			*l1_pte |= (entry->dom << PAGE_DESC_DOMAIN_SHIFT);

			if(((*l1_pte >> PAGE_DESC_L1_PAGE_NS_SHIFT)&0x1)!=entry->ns)
				sw_printk("Error: Allocating Pages \n");

			l2_base = (*l1_pte & PAGE_DESC_PAGE_TBL_MASK);
			l2_base_vir = (sw_uint)secure_phy_to_vir(l2_base);

			break;
		default:
			sw_printk("map_small_page_entry: This function supports "
					"only coarse mappings.\n");
			sw_printk("  L1 descriptor: %08x\n", *l1_pte);
			sw_seterrno(SW_EINVAL);
			return SW_ERROR;
	}

	l2_pte = (sw_uint*)(l2_base_vir + ( ( (entry->va >> 12) & 0xFF ) << 2 ));
	
	head= &l2_pages.list;
	l = head->next;
	while (l != head) {
		l2_ref = l->data;
		if(l2_ref) {
			if(l2_base_vir >= l2_ref->page_base && l2_base_vir <
					(l2_ref->page_base + PAGE_SIZE)){
				index = (l2_base_vir - l2_ref->page_base)/L2_PAGE_TABLE_SIZE;
				found = TRUE;
			}
		}
		if(found)
			break;
		l = l->next;
	}
	l2_ref->page[index] = l2_ref->page[index] + 1;

	*l2_pte = (entry->pa & PAGE_DESC_PAGE_MASK) | PAGE_DESC_TYPE_SMALL;
	*l2_pte |= (entry->b << PAGE_DESC_B_SHIFT);
	*l2_pte |= (entry->c << PAGE_DESC_C_SHIFT);
	*l2_pte |= (entry->tex << PAGE_DESC_L2_TEX_SHIFT );
	*l2_pte |= (entry->xn << PAGE_DESC_PAGE_XN_SHIFT);
	*l2_pte |= ((entry->ap & PAGE_DESC_AP_MASK) << PAGE_DESC_L2_AP_SHIFT);
	*l2_pte |= (((entry->ap >> 2) & 0x1) << PAGE_DESC_L2_AP2_SHIFT);
	*l2_pte |= (entry->s << PAGE_DESC_L2_S_SHIFT);
	*l2_pte |= (entry->ng << PAGE_DESC_L2_NG_SHIFT);
	inv_translation_table_by_va_asid(entry->va);
	
	if(l2_pages.blk_allocated - l2_pages.blk_used <= 2 && skip == FALSE){
		skip = TRUE;
		second_level_pages *temp = NULL;
		if(global_val.heap_init_done) {
			temp = (second_level_pages*)sw_malloc_private
				(COMMON_HEAP_ID, sizeof(second_level_pages));
		}
		else {
			if(static_l2_next_idx < MAX_STATIC_L2_CNT) {
				temp = &static_l2_ref[static_l2_next_idx];
			}
			else 
				tee_panic("Not enough static L2 tables");
		}
		sw_memset(temp, 0, sizeof(second_level_pages));
		if(global_val.heap_init_done) {		
			temp->page_base = (sw_uint)sw_vir_page_alloc(PAGE_SIZE,
					&global_val.sw_mem_info);
		}
		else {
			temp->page_base = (sw_uint)
								&static_l2_page[static_l2_next_idx];
			static_l2_next_idx++;
		}
		link_init(&temp->head_ref);
		set_link_data(&temp->head_ref, temp);
		add_link(&l2_pages.list, &temp->head_ref, TAIL);
		l2_pages.blk_allocated += (PAGE_SIZE/L2_PAGE_TABLE_SIZE);
		skip = FALSE;
	}

	return SW_OK;
}

/**
 * @brief Unmap the second level page table entry from supplied page table
 *
 * @param pgd: Pointer to the page table
 * @param va: Virtual address which need to be unmapped
 *
 * @return 
 * SW_OK \n
 * SW_ERROR* \n
 */

int unmap_small_page_entry(sw_uint* pgd, sw_vir_addr va){
	sw_uint l1_index = 0;
	sw_uint loop_cnt;
	sw_uint *l1_pte, *l2_base, l2_base_vir, *l2_pte;
	sw_uint index, empty, found, free;
	second_level_pages *l2_ref;
	
	struct link *l, *head;
	l1_index = va >> 20;
	l1_pte = pgd + l1_index;

	switch ((*l1_pte) & PAGE_DESC_TYPE_MASK) {
		case PAGE_DESC_TYPE_FAULT:
			sw_printk("0x%x Memory is not mapped\n", *l1_pte & PAGE_DESC_PAGE_TBL_MASK);
			sw_seterrno(SW_EINVAL);
			return SW_ERROR;
		case PAGE_DESC_TYPE_COARSE:
			l2_base = (sw_uint *)(*l1_pte & PAGE_DESC_PAGE_TBL_MASK);
			l2_base_vir = secure_phy_to_vir((sw_phy_addr)l2_base);
			break;
		default:
			sw_printk("unmap_small_page_entry: This function supports "
					"only coarse unmappings.\n");
			sw_seterrno(SW_EINVAL);
			return SW_ERROR;
	}
	l2_pte = (sw_uint* )l2_base_vir + ((va >> 12) & 0xFF);
	*l2_pte = 0;
	inv_translation_table_by_va_asid(va);

	empty = FALSE; found = FALSE; free = FALSE;
	
	head= &l2_pages.list;
	l = head->next;
	while (l != head) {
		l2_ref = l->data;
		if(l2_ref) {
			if(l2_base_vir >= l2_ref->page_base && l2_base_vir <
					(l2_ref->page_base + PAGE_SIZE)){
				index = (l2_base_vir - l2_ref->page_base)/L2_PAGE_TABLE_SIZE;
				found = TRUE;
			}
			if(found){
				l2_ref->page[index] = l2_ref->page[index] - 1;
				if(l2_ref->page[index] == 0){
					*l1_pte = 0;
					l2_ref->blk_used--;
					l2_pages.blk_used--;
				}
				if(l2_ref->blk_used == 0)
					empty = TRUE;
				if(empty){
					if(l2_pages.blk_allocated - l2_pages.blk_used 
							> (PAGE_SIZE/L2_PAGE_TABLE_SIZE) + 1){
						remove_link(&l2_ref->head_ref);
						l2_pages.blk_allocated -=
							(PAGE_SIZE/L2_PAGE_TABLE_SIZE);
						free = TRUE;
						for(loop_cnt = 0; loop_cnt <= static_l2_next_idx;
								loop_cnt++) 
							if(l2_ref == &static_l2_ref[loop_cnt]) {
								free = FALSE;
								break;
							}
						if(free) {
							sw_vir_addr_free((sw_uint)l2_ref->page_base,
								PAGE_SIZE, &global_val.sw_mem_info);
							sw_free_private(COMMON_HEAP_ID, l2_ref);
						}
						
					}
				}
				return SW_OK;
			}
		}
		l = l->next;
	}	
	sw_seterrno(SW_ENOMEM);
	return SW_ERROR;
}
