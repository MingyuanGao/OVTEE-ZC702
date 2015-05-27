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
 * Header for module initialization implementation functions
 */

#ifndef _SW_MODINIT_H_
#define _SW_MODINIT_H_

#include <sw_link.h>
#include <sw_types.h>

#define __init __attribute__((section(".init")))
#define __mod_init __attribute__((section(".mod_init"))) __attribute__((used))
#define __mod_exit __attribute__((section(".mod_exit"))) __attribute__((used))

/**
 * @brief 
 *      Typedef of initialization functions in all the
 *      modules
 * @param 
 *      None
 * @return 
 *      None
 */
typedef void (*mod_init_fn_call) (void);

/**
 * @brief 
 *      Typedef of function called during module exit
 * @param 
 *      None
 * @return 
 *      None
 */
typedef void (*mod_exit_fn_call) (void);

#define  SW_MODULE_INIT(fn) \
	static  __mod_init mod_init_fn_call mod_init_fn = fn

#define  SW_MODULE_EXIT(fn) \
	static  __mod_exit mod_exit_fn_call  mod_exit_fn = fn


/**
 * @brief 
 */
struct sw_file_operations{
	struct link head;
	int (*open) ();
	int (*close) ();
	int (*read) ();
	int (*write) ();
	int (*ioctl) ();
	sw_uint dev_id;
	char sw_dev_name[20];
};

/**
 * @brief 
 *  This contains the list of devices registered
 *  during Initialisation
 */
struct sw_devices_head{
	struct link dev_list;
};

/**
 * @brief 
 *      This function is called by each module for registration
 *
 * @param sw_dev
 *      File operations structure of the device
 */
void sw_device_register(struct sw_file_operations* sw_dev);

/**
 * @brief 
 *      This function is called by each module to unregister them 
 * @param sw_dev
 *      File operations structure of the device
 */
void sw_device_unregister(struct sw_file_operations* sw_dev);

/**
 * @brief 
 *  This function calls the initialization function of all the modules
 */
void modules_init(void);

/**
 * @brief Find device structure based on device name
 *
 * This function is used by kernel routines to invoke device functions.
 * 
 * @param name
 *
 * @return 
 */
struct sw_file_operations * find_device(char* name);
#endif
