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
 * Page table protection flag supports the Fixed virtual address 
 * for application entry point, Heap, Stack and TLS
 */
#ifndef __SW_CONFIG_H_
#define __SW_CONFIG_H_

/*
LIBC heap 			0xb0000000 - 0xb008ffff - 8 MB

Application Heap	0x00900000 - 0x00afffff - 2MB - 

Stack + TLS			0x00b00000 - 0x00bfffff - 1MB

Library				0x00c00000 - 0x00dfffff - 2MB

Application			0x00e00000 - 0x00ffffff - 2MB
*/

#define LIBC_HEAP_VA			(0xb0000000)

#define USER_HEAP_VA 			(0x00900000)
#define USR_TLS_VA				(0x00BFF000)

#define LIB_LOAD_ADDR 			(0x00C00000)

#define SHM_AT_ADDR				(0x00100000)

#endif
