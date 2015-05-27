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
 * module initialization implementation functions
 */

#include <sw_types.h>
#include <cpu_data.h>
#include <sw_modinit.h>
#include <global.h>

/**
 * @brief 
 *      Initializes the list head which will hold the list of registered devices
 */
static void sw_dev_head_init(void)
{
	link_init(&global_val.sw_dev_head.dev_list);
}

/**
 * @brief 
 *      This function is called by each module for registration
 *
 * @param sw_dev
 *      File operations structure of the device
 */
void sw_device_register(struct sw_file_operations* sw_dev)
{
	set_link_data(&sw_dev->head, sw_dev);
	add_link(&global_val.sw_dev_head.dev_list, &sw_dev->head, TAIL);
}

/**
 * @brief 
 *      This function is called by each module to unregister them 
 * @param sw_dev
 *      File operations structure of the device
 */

void sw_device_unregister(struct sw_file_operations* sw_dev)
{
	remove_link(&sw_dev->head);    
}

/**
 * @brief 
 *  This function calls the initialization function of all the modules
 */
void modules_init(void)
{
	sw_dev_head_init();
	mod_init_fn_call init_fn;
	sw_uint *init_fn_addr, init_start_addr, init_end_addr;
	init_start_addr = (sw_uint)get_mod_init_start_addr();
	init_end_addr = (sw_uint)get_mod_init_end_addr();
	for(init_fn_addr = (sw_uint *)init_start_addr;
			init_fn_addr < (sw_uint *)init_end_addr; init_fn_addr++){
		init_fn = (mod_init_fn_call) *(init_fn_addr);
		if(init_fn != NULL){
			init_fn();
		}
	}
}
