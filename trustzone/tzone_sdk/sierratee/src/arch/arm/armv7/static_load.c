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
 * Implementation of static 'C' library and user application load.
 *
 * This file has the implementation of 'C' library load routines.
 * Load 'C' library from image or file system into a secure memory.
 * It also implements the loading of user application from image to 
 * memory.
 * Enabling ELF loader support disables the static 'C' library and 
 * application loading.
 *  
 */
#include <sw_types.h>
#include <sw_board.h>
#include <page_table.h>
#include <global.h>
#include <cpu_data.h>
#include <sw_mem_functions.h>
#include <sw_debug.h>

/**
 * @brief Load static 'C' library to memory
 * Load embedded 'C' library from image and map with user read/write 
 * permission. 
 *
 * @return 
 */
sw_int load_libc_to_memory(void)
{
	sw_int error;
	sw_uint aligned_size, libc_start;
	sw_phy_addr libc_phy;

	libc_start = (sw_uint)get_libc_section_start();
	
	libc_phy =  secure_vir_to_phy_offset(libc_start);
	aligned_size = (sw_uint)get_libc_size();
	/* Align it to the next PAGE SIZE */
	if (aligned_size & (PAGE_SIZE - 1)) { 
		aligned_size = (aligned_size & PAGE_ALIGN_MASK) + PAGE_SIZE;
	}

	/* start address is assumed to be aligned */
	error = map_secure_memory(libc_start,
			libc_phy, aligned_size, 
			PTF_PROT_URW| PTF_EXEC);
	if (error < 0) {
		sw_printk("libc load : mapping failed\n");
		return error;
	}

	global_val.lib_va = libc_start;
	global_val.lib_pa = libc_phy;
	global_val.lib_size = aligned_size;
	return SW_OK;
}


/**
 * @brief Load user application embedded in the image  to memory
 * Load embedded user application from image and map the text
 * section with user read/execute permission. Map the data and bss
 * section with user read/write permission. Initialize the 
 * user application bss section to zero. 
 *
 * @return 
*/
sw_int load_user_app_to_memory(void)
{
	sw_int error;
	sw_uint aligned_size, app_start;

	app_start = (sw_uint)get_app_text_start();
	aligned_size = (sw_uint)get_app_text_size();
	/* Align it to the next PAGE SIZE */
	if (aligned_size & (PAGE_SIZE - 1)) { 
		aligned_size = (aligned_size & PAGE_ALIGN_MASK) + PAGE_SIZE;
	}

	/* user application text section mapping */
	error = map_secure_memory(app_start,
			secure_vir_to_phy_offset(app_start), aligned_size, 
			PTF_PROT_URO| PTF_EXEC);
	if (error < 0) {
		sw_printk("user application load: text mapping failed\n");
		return error;
	}

	app_start = (sw_uint)get_app_data_start();
	aligned_size = (sw_uint)get_app_data_size();
	/* Align it to the next PAGE SIZE */
	if (aligned_size & (PAGE_SIZE - 1)) { 
		aligned_size = (aligned_size & PAGE_ALIGN_MASK) + PAGE_SIZE;
	}

	/* user application data section mapping */
	error = map_secure_memory(app_start,
			secure_vir_to_phy_offset(app_start), aligned_size, 
			PTF_PROT_URW);
	if (error < 0) {
		sw_printk("user application load: data mapping failed\n");
		return error;
	}

	app_start = (sw_uint)get_app_bss_start();
	aligned_size = (sw_uint)get_app_bss_size();
	/* Align it to the next PAGE SIZE */
	if (aligned_size & (PAGE_SIZE - 1)) { 
		aligned_size = (aligned_size & PAGE_ALIGN_MASK) + PAGE_SIZE;
	}

	/* user application data section mapping */
	error = map_secure_memory(app_start,
			secure_vir_to_phy_offset(app_start), aligned_size, 
			PTF_PROT_URW);
	if (error < 0) {
		sw_printk("user application load: bss mapping failed\n");
		return error;
	}

	sw_memset((void*)app_start, 0, aligned_size);
	return SW_OK;
}

