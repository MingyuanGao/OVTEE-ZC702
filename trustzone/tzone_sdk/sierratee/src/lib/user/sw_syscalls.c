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

#include <sw_types.h>
#include <sw_syscall.h>

/* 
 * Private System calls functions specific to user tasks
 */
/**
* @brief System call to map non-secure memory in secure page table.
*
* @param va_addr: Physical address need to be mapped
* @param va_addr: Virtual address need to be mapped
*
* @return 
* SW_OK - Mapping is done successfully. \n
* SW_* - Implementation defined error.\n
*/
int map_to_ns(sw_phy_addr phy_addr, sw_vir_addr *va_addr)
{
	return __asm_map_to_ns(phy_addr, va_addr);
}


/**
* @brief System call to unmap non-secure memory from secure page table.
*
* @param va_addr: Virtual address need to be unmapped
*
* @return 
* SW_OK - Unmapping is done successfully. \n
* SW_* - Implementation defined error.\n
*/
int unmap_from_ns(sw_vir_addr va_addr)
{
	return __asm_unmap_from_ns(va_addr);
}


