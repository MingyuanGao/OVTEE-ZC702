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
 * Non secure Kernel Loader routines
 */


#include <nsk_boot.h>
#include <sw_types.h>
#include <page_table.h>
#include <mem_mng.h>
#include <sw_mem_functions.h>
#include <sw_debug.h>
#include <system_context.h>
#include <monitor.h>
#include <cpu.h>
#include <cache.h>
#include <cpu_data.h>

#define MAP_SIZE PAGE_SIZE
#define ALIGN_MASK (~(MAP_SIZE - 1))

#define MAP_SECTION_SIZE SECTION_SIZE
#define ALIGN_SECTION_MASK (~(MAP_SECTION_SIZE - 1))

static void nsk_initrd_load(struct nsk_boot_info *);

/**
 * @brief 
 *
 * @return 
 */
int nsk_load(struct nsk_boot_info *ni)
{
	int error;
	sw_uint ns_map_size, s_map_size, aligned_size;

	aligned_size = ni->nskbi_size;
	/* Align it to the next PAGE SIZE */
	if (aligned_size & (MAP_SIZE - 1)) { 
		aligned_size = (aligned_size & ALIGN_MASK) + MAP_SIZE;
	}

	/* Align it to the next SECTION SIZE */
	ns_map_size = aligned_size;
	if (ns_map_size & (MAP_SECTION_SIZE - 1)) { 
		ns_map_size = (ns_map_size & ALIGN_SECTION_MASK) + MAP_SECTION_SIZE;
	}

	if (ni->nskbi_loadaddr & (MAP_SECTION_SIZE - 1)) {
		ns_map_size += MAP_SECTION_SIZE;
	}

	/* map the non secure area */
	error = map_nsmemsect_normal(ni->nskbi_loadaddr & ALIGN_SECTION_MASK,
			ni->nskbi_loadaddr & ALIGN_SECTION_MASK,
			ns_map_size);
	if (error < 0) {
		sw_printk("nsk_load: mapping failed\n");
		return error;
	}

	s_map_size = aligned_size;
	/* start address is assumed to be aligned */
	error = map_secure_memory(ni->nskbi_srcaddr,
			secure_vir_to_phy_offset(ni->nskbi_srcaddr), s_map_size, PTF_PROT_KRW);
	if (error < 0) {
		sw_printk("nsk_load: mapping failed\n");
		return error;
	}

	/* Move the ns kernel to the load address */
	sw_memcpy((void*)ni->nskbi_loadaddr, (void*)ni->nskbi_srcaddr, ni->nskbi_size);

	invoke_dsb();

	flush_icache_and_dcache();

	/* Unmap the non secure area */
	unmap_nsmemsect_normal(ni->nskbi_loadaddr & ALIGN_SECTION_MASK, ns_map_size);

	unmap_secure_memory(ni->nskbi_srcaddr, s_map_size);

	/* Load initrd */
	if (ni->nskbi_initrd_flag) {
		nsk_initrd_load(ni);
	}

	return SW_OK;
}

/**
 * @brief 
 *
 * @return 
 */
static void nsk_initrd_load(struct nsk_boot_info *ni)
{
	int error;
	sw_uint aligned_size, ns_aligned_size;

	aligned_size = ni->nskbi_initrd_size;
	/* Align it to the next PAGE SIZE */
	if (aligned_size & (MAP_SIZE - 1)) { 
		aligned_size = (aligned_size & ALIGN_MASK) + MAP_SIZE;
	}

	/* Align it to the next SECTION SIZE */
	ns_aligned_size = aligned_size;
	if (ns_aligned_size & (MAP_SECTION_SIZE - 1)) { 
		ns_aligned_size = (ns_aligned_size & ALIGN_SECTION_MASK) 
							+ MAP_SECTION_SIZE;
	}

	if (ni->nskbi_initrd_la & (MAP_SECTION_SIZE - 1)) {
		ns_aligned_size += MAP_SECTION_SIZE;
	}

	error = map_nsmemsect_normal(ni->nskbi_initrd_la & ALIGN_SECTION_MASK, 
			ni->nskbi_initrd_la & ALIGN_SECTION_MASK, 
			ns_aligned_size);
	if (error < 0) {
		sw_printk("nsk_load: mapping failed\n");
		return;
	}

	error = map_secure_memory(ni->nskbi_initrd_sa, 
			secure_vir_to_phy_offset(ni->nskbi_initrd_sa),
			aligned_size, PTF_PROT_KRW);
	if (error < 0) {
		sw_printk("nsk_load: mapping failed\n");

		unmap_nsmemsect_normal(ni->nskbi_initrd_la, 
				ns_aligned_size);

		return;
	}

	/* Move the initrd to the load address */
	sw_memcpy((void*)ni->nskbi_initrd_la, 
			(void*)ni->nskbi_initrd_sa,
			ni->nskbi_initrd_size);

	unmap_nsmemsect_normal(ni->nskbi_initrd_la, 
			ns_aligned_size);

	unmap_secure_memory(ni->nskbi_initrd_sa, aligned_size);
}
