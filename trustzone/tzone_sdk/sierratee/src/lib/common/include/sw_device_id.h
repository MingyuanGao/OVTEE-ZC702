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
 * Device identifier and Device names used for Driver framework. 
 * This device names are used to open the device using open system call.
 */

#ifndef __SW_DEVICE_ID_H_
#define __SW_DEVICE_ID_H_

#define UART_DEVICE_NAME "UART"
#define UART_DEVICE_ID 10001

#define IPC_DEVICE_NAME "IPC"
#define IPC_DEVICE_ID 10002

#ifdef CONFIG_GUI_SUPPORT
#define FB_DEVICE_NAME "fb0"
#define FB_DEVICE_ID 2124
#endif

#endif
