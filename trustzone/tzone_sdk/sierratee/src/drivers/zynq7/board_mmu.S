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
 * Map device specific uart
 */
#include <sw_device_io.h>	
.global map_init_devices
.func map_init_devices
map_init_devices:
    push    { lr }
    ldr     r3, =SECURE_UART_BASE_PA
    ldr     r4, =SECURE_UART_BASE_PA
    movt    r5, #16
    mov     r11, #1
    mov     r12, #1
    mov     r6, #0
    mov     r9, #0
    bl      init_page_table
    pop     { lr }
    mov     pc, lr
.endfunc
