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
 * Header for secure page table initalization
 */

#ifndef __CPU_PAGETABLE_H__
#define __CPU_PAGETABLE_H__

#include <sw_types.h>
#include <sw_link.h>

#define PAGE_SIZE               4096
#define PAGE_SHIFT              12
#define PAGE_MASK               0xFFFFF000
#define PAGE_OFFSET_MASK        0x00000FFF

#define PAGE_ALIGN_MASK 		(~(PAGE_SIZE - 1))

#define SECTION_SIZE            (1 << 20)
#define SECTION_SHIFT           20
#define SECTION_MASK            0xFFF00000
#define SECTION_OFFSET_MASK     0x000FFFFF

#define PAGE_TABLE_ENTRIES 4096
#define PAGE_TABLE_ENTRY_WIDTH 4
#define PAGE_TABLE_SIZE (PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRY_WIDTH)

#define L2_PAGE_TABLE_ENTRIES 256
#define L2_PAGE_TABLE_SIZE (PAGE_TABLE_ENTRY_WIDTH * L2_PAGE_TABLE_ENTRIES)

#define SECURE_ACCESS_DOMAIN	0

#define PAGE_DESC_AP_MASK  	0x3

#define PAGE_DESC_TYPE_MASK	0x3
#define PAGE_DESC_TYPE_FAULT	0x0

/* XN Bit Values */
#define PAGE_DESC_XN_NEVER	0x1
#define PAGE_DESC_XN_ALLOW	0x0

#define PAGE_DESC_TYPE_COARSE	0x1
#define PAGE_DESC_TYPE_SECTION	0x2

#define PAGE_DESC_TYPE_SMALL	(0x1 << 1)
#define PAGE_DESC_TYPE_LARGE	0x1

#define PAGE_DESC_PAGE_TBL_MASK	0xFFFFFC00
#define PAGE_DESC_SEC_MASK	0xFFF00000
#define PAGE_DESC_PAGE_MASK	0xFFFFF000
#define PAGE_DESC_LARGE_PAGE_MASK	0xFFFF0000

#define PAGE_DESC_B_SHIFT	2
#define PAGE_DESC_C_SHIFT	3

#define PAGE_DESC_SEC_XN_SHIFT	4
#define PAGE_DESC_PAGE_XN_SHIFT	0	/* For small page */

#define PAGE_DESC_DOMAIN_SHIFT	5
#define PAGE_DESC_DOMAIN_MASK	0xf
#define PAGE_DESC_DOMAIN_MASK_ENTRY	(0xf << PAGE_DESC_DOMAIN_SHIFT)

#define PAGE_DESC_NS_MASK  	0x1 
/* Non secure bit in L1 Page table */
#define PAGE_DESC_L1_PAGE_NS_SHIFT	3 
/* Non secure bit in L1 Section entry */
#define PAGE_DESC_L1_SEC_NS_SHIFT	19
#define PAGE_DESC_L1_SEC_NS_MASK_ENTRY	(0x1 << PAGE_DESC_L1_SEC_NS_SHIFT)

#define PAGE_DESC_L1_AP_SHIFT		10
#define PAGE_DESC_L1_AP_MASK_ENTRY	(0x3 << 10) 
#define PAGE_DESC_L1_AP2_SHIFT    	15
#define PAGE_DESC_L1_AP2_MASK_ENTRY   	(0x1 << 15)

#define PAGE_DESC_L1_S_SHIFT    	16
#define PAGE_DESC_L1_NG_SHIFT    	17
#define PAGE_DESC_L1_TEX_SHIFT    	12

#define PAGE_DESC_L2_AP_SHIFT	4
#define PAGE_DESC_L2_AP_MASK_ENTRY		(0x3 << 4)
#define PAGE_DESC_L2_AP2_SHIFT    	9
#define PAGE_DESC_L2_AP2_MASK_ENTRY   	(0x1 << 9)

#define PAGE_DESC_L2_S_SHIFT    	10
#define PAGE_DESC_L2_NG_SHIFT    	11
#define PAGE_DESC_L2_TEX_SHIFT    	6 /* For small page */

#define FSR_TYPE_MASK           0x40f
#define FSR_ALIGN_FAULT         0x1
#define FSR_EXT_ABORT_L1        0xc
#define FSR_EXT_ABORT_L2        0xe
#define FSR_TRANS_SEC           0x5
#define FSR_TRANS_PAGE          0x7
#define FSR_DOMAIN_SEC          0x9
#define FSR_DOMAIN_PAG          0xb
#define FSR_PERM_SEC            0xd
#define FSR_PERM_PAGE           0xf

#define FSR_DOMAIN_MASK         0xf0
#define FSR_WNR_MASK            0x800
#define FSR_EXT_MASK            0x1000

/* 
 * Page table flags used by memory mapping APIs 
 * 0 - 2 : protection 
 * 3     : exec flag
 * 4 - 31: unused
 */
#define PTF_PROT_KRW   (PRIV_RW_USR_NO)
#define PTF_PROT_KRO   (PRIV_RO_USR_NO)
#define PTF_PROT_URW   (PRIV_RW_USR_RW)
#define PTF_PROT_URO   (PRIV_RW_USR_RO)
#define PTF_EXEC       (1 << PTF_EXEC_SHIFT)


#define PTF_PROT_MASK  0x7
#define PTF_EXEC_MASK  0x8
#define PTF_EXEC_SHIFT 0x3

#define ptf_to_ap(x)   ((x) & PTF_PROT_MASK)
#define ptf_to_xn(x)   ((~(x) & PTF_EXEC_MASK) >> PTF_EXEC_SHIFT)

#define secure_vir_to_phy_offset(x) \
	(SECURE_WORLD_RAM_START + (sw_uint)x - (sw_uint)get_sw_code_start())

/**
* @brief Device map structure to hold information about each device mapping.
*/
struct devmap {
	/*!Device Virtual address */
	sw_vir_addr dv_va;
	/*!Device Physical address */
	sw_phy_addr dv_pa;
	/*!Device map region size */
	sw_uint  dv_size;
};

/**
 * @brief Memory access control constants
 */
enum mem_access_ctrl
{
	/*!Priv no access, usr no access */
	PRIV_NO_USR_NO = 0b000,   
	/*!Priv read/write, usr no access */
	PRIV_RW_USR_NO = 0b001,   
	/*!Priv read/write, usr read only */
	PRIV_RW_USR_RO = 0b010,   
	/*!Priv read/write, usr read/write */
	PRIV_RW_USR_RW = 0b011,   
	/*!Reserved */
	AP_RESERVED = 0b100,
	/*!Priv read only, usr no access */
	PRIV_RO_USR_NO= 0b101,    
	/*!Deprecated */
	DEPRECATED=0b110,         
	/*!Priv read only, usr read only */
	PRIV_RO_USR_RO= 0b111,    
};

/**
 * @brief Section page table entry
 */
struct cpu_pt_entry {
	/*! Virtual address */
	sw_vir_addr va;
	/*! Physical address */
	sw_phy_addr pa;
	/*! Size */
	size_t sz;
	/*! NS bit */
	sw_uint ns:1;
	/*! NG bit */
	sw_uint ng:1;
	/*! Shared bit */
	sw_uint s:1;
	/*! Tex bits */
	sw_uint tex:3;
	/*! AP bit */
	sw_uint ap:3;
	/*! Implementation defined */
	sw_uint imp:1;
	/*! Domain field */
	sw_uint dom:4;
	/*! XN bit */
	sw_uint xn:1;
	/*! Cache bit */
	sw_uint c:1;
	/*! Bufferable bit */
	sw_uint b:1;
	/*! Padding */
	sw_uint pad:15;
};

/**
 * @brief Page table reference
 */
struct sw_page_ref {
	/*! List head */
	struct link head;
	/*! Page table entry */
	sw_uint l1_pte;
	/*! Reference count */
	sw_uint ref_cnt;
};

/*
 * Second level page table entry
 */
typedef struct snd_lvl_page {
	/*! Second level page table base address */
	sw_uint page_base;
	/*! Each 4K page hold four second level page tables */
	sw_uint page[4];
	/*! Page tables used flag*/
	sw_uint blk_used;
	/*! Link node */
	struct link head_ref;
} second_level_pages;

/*
 * Global list containing all allocated L2 pages
 */
struct l2_pg{
	/*! List to hold allocated pages */
	struct link list;
	/*! Allocated blocks */
	sw_uint blk_allocated;
	/*! Used blocks */
	sw_uint blk_used;
} ;

/**
 * @brief Returns the secure page table pointer
 *
 * @return Pointer to page table
 */
sw_uint* get_secure_ptd(void);

/**
 * @brief Initialize page table entries
 *
 * This function creates the initial page table entries for secure kernel
 * @return Pointer to the page table
 */
sw_uint* map_secure_page_table(void);
/**
 * @brief Create shared memory mapping between secure and non-secure kernel
 *
 * This function creates the page table entry with ns bit set. So that 
 * this section of the non-secure memory act like a shared memory.
 *
 * @param phy_addr: Physical address of the non-secure memory
 * @param va_addr:  Virtual address of the shared memory
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */
int __map_to_ns(sw_phy_addr phy_addr, sw_vir_addr *va_addr);

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
int map_to_ns(sw_phy_addr phy_addr, sw_vir_addr *va_addr);

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
 * SW_ERROR* \n
 */
int __unmap_from_ns(sw_vir_addr va_addr);

/**
 * @brief 
 *      Wrapper function for __unmap_from_ns to call from kernel routines.
 * @param va_addr:  Virtual address of the shared memory
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */
int unmap_from_ns(sw_vir_addr va_addr);

/**
 * @brief Map device entry in secure page table
 * 
 * Creates device mapping entry in secure page table.
 * This function assumes that the start address and size are page aligned. 
 *
 * @param va: virtual address
 * @param pa: physical address
 * @size size: size
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */
int map_device(sw_vir_addr va, sw_phy_addr pa, sw_uint size);

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

int map_device_table(const struct devmap *dt);

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

int map_kernel_text_memory(sw_vir_addr va, sw_phy_addr pa, sw_uint size);

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

int map_kernel_data_memory(sw_vir_addr va, sw_phy_addr pa, sw_uint size);

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

int map_user_text_memory(sw_vir_addr va, sw_phy_addr pa, sw_uint size, sw_phy_addr
		user_pt_base);

/**
 * @brief Map a range of memory area as non-global data region in supplied page table
 *
 * This function assumes that the start address and size are page aligned.
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

int map_user_data_memory(sw_vir_addr va, sw_phy_addr pa, sw_uint size, sw_phy_addr
		user_pt_base);

/**
 * @brief Map a range of memory area as non-global data region in supplied page table
 *
 * This function assumes that the start address and size are page aligned.
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

int map_user_rodata_memory(sw_vir_addr va, sw_phy_addr pa, sw_uint size, sw_phy_addr
		user_pt_base);

/**
 * @brief Map a range of memory area as non-global region in supplied page table
 *
 * This function assumes that the start address and size are page aligned.
 *
 * @param va: Virtual address to be mapped 
 * @param pa: Physical address to be mapped
 * @param size: Size of the mapped memory
 * @param ptf: Access flags for user read/write or execute permission
 * @param user_pt_base: Physical address of the page table
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */

int map_user_memory(sw_vir_addr va, sw_phy_addr pa, sw_uint size, 
		sw_uint ptf, sw_phy_addr user_pt_base);

/**
 * @brief Map a range of secure memory area in secure page table
 *
 * This function maps the memory as global entry and permission based on the 
 * access flag. 
 * This function assumes that the start address and size are page aligned. 
 *
 * @param va: Virtual address to be mapped 
 * @param pa: Physical address to be mapped
 * @param size: Size of the mapped memory
 * @param flags: flags for access permission and exec
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */

int map_secure_memory(sw_vir_addr va, sw_phy_addr pa, sw_uint size, sw_uint ptf);

/**
 * @brief Unmap a range of memory which are mapped as non-global and 
 * user mode permission from the supplied page table
 *
 * This function assumes that the start address and size are aligned.
 *
 * @param va: Virtual address to be un-mapped 
 * @param size: Size of the un-mapped memory
 * @param user_pt_base: Physical address of the page table 
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */

int unmap_user_memory(sw_vir_addr va, sw_uint size, sw_phy_addr user_pt_base);

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

int unmap_kernel_memory(sw_vir_addr va, sw_uint size);

/**
 * @brief Unmap a range of memory from secure page table.
 *
 * This function assumes that the start address and size are page aligned. 
 *
 * @param va: Virtual address to be un-mapped 
 * @param size: Size of the un-mapped memory
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */

int unmap_secure_memory(sw_vir_addr va, sw_uint size);

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

int map_nsmemsect_normal(sw_vir_addr, sw_phy_addr, sw_uint);


/**
 * @brief Unmap a range of non secure memory area 
 *
 * This function assumes that the start address and size are section aligned. 
 *
 * @param va: Virtual address to be mapped 
 * @param pa: Physical address to be mapped
 * @param size: Size of the mapped memory
 *
 * @return:
 * SW_OK \n
 */

int unmap_nsmemsect_normal(sw_vir_addr, sw_uint);

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
int map_section_entry(sw_uint *pgd, struct cpu_pt_entry *entry);
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
int map_ns_section_entry(sw_uint *pgd, struct cpu_pt_entry *entry);

/**
 * @brief Unmap the shared section entry from secure page table
 *
 * This function unmaps the secure memory section from secure page table. 
 * i.e. set 'NS' bit to zero
 *
 * @param pgd: Pointer to page table
 * @param va: Virtual address to be unmapped
 *
 * @return:
 * SW_OK \n
 * SW_ERROR* \n
 */
int unmap_ns_section_entry(sw_uint *pgd, sw_vir_addr va);

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
int page_ref_add(sw_uint l1_pte);

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
int page_ref_release(sw_uint l1_pte);

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
int map_small_page_entry(sw_uint* pgd, struct cpu_pt_entry* entry);

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
int unmap_small_page_entry(sw_uint* pgd, sw_vir_addr va);

#endif
