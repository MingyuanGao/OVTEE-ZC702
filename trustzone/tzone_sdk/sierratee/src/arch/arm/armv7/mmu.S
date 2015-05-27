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
 * This file has the implementation of identity page table mapping before we 
 * switch to original page table entries based on the board configuration.
 */

#include <cpu_asm.h>
#include <sw_platform_asm.h>
#include <asm_macros.h>	
.global create_page_table
.func create_page_table
create_page_table:
	push { lr }
	push { r0 - r12 }
	ldr	 r2, =tmp_page_table
	add	 r2, r2, r10
#memset the entire page table to zero
	mov	 r3, r2
	add	 r4, r3, #16384
	mov	 r5, #0
2:	str	 r5, [r3], #4
	cmp	 r3, r4
	blo	 2b

#create section entries in page table
####Entire section
	ldr	 r3, =_SW_CODE_START
	ldr	 r5, =_total_sw_size
	add	 r4, r3, r10
	mov	 r11, #01
	mov	 r12, #0
	mov	 r6, #1
	mov	 r9, #1
	bl	 init_page_table
	cmp	 r3, r4
	beq	 1f
	mov	 r3, r4
	bl	 init_page_table
1:
	bl   map_init_devices
	pop	 { r0 - r12 }
	pop	 { lr }
	bx   lr
.endfunc
