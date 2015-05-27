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
 * TEE based hypervisor implementation routines.
 */

#include <sw_types.h>
#include <sw_debug.h>
#include <system_context.h>
#include <mem_mng.h>
#include <sw_mem_functions.h>
#include <page_table.h>
#include <tzhyp_global.h>
#include <tzhyp_config.h>
#include <sw_board.h>
#include <task.h>
#include <debug_config.h>

/**
 * @brief Non secure and Secure context pointers
 */
struct system_context *ns_sys_current;

/*
 * The system context for secure wolrd is static, so the word current might be a
 * misnomer. 
 */
struct system_context *s_sys_current; 

/**
 * @brief 
 *
 * @return 
 */
static int tzhyp_nsadmin_init(void)
{
	int error; 
	void *loadaddr = (sw_uint *)NSADMIN_LOAD;
	void *startaddr = (sw_uint *)&tzhyp_nsadmin_start;
	int size = (sw_uint)&tzhyp_nsadmin_end - (sw_uint)&tzhyp_nsadmin_start;

	error = map_nsmemsect_normal((sw_vir_addr)loadaddr, (sw_phy_addr)loadaddr, 
			SECTION_SIZE);
	if (error < 0)
		return error;

	sw_memcpy(loadaddr, startaddr, size);

	unmap_nsmemsect_normal((sw_vir_addr)loadaddr, SECTION_SIZE);

	return SW_OK;
}

/**
 * @brief 
 *
 * @return 
 */
int tzhyp_init(void)
{
	int error;
	
	ns_sys_current = (struct system_context *)global_val.tzhyp_val.ns_world;
	s_sys_current = (struct system_context *)global_val.tzhyp_val.s_world; 

	error = tzhyp_nsadmin_init();
	if (error < 0) {
		sw_printk("tzhyp: nsadmin init failed\n");
		return error;
	}
	error =  tzhyp_guest_init();
	if (error < 0) {
		sw_printk("tzhyp: guest init failed\n");
		return error;
	}

	return SW_OK;
}

/**
 * @brief
 *
 * @param guest_no
 *
 */
void launch_current_guest(void)
{
	static sw_bool invoke = false;
	if(!invoke) {
		invoke = true;
		start_task(global_val.linux_task_id, NULL);
		return;
	}
}
