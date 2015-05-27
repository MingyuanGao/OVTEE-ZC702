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

#include <cpu_asm.h>
#include <sw_board_asm.h>
#include <asm_macros.h>	

.global secondary_main
secondary_main:	
ldr	r0,=sec_core_start_stat_addr
ldr 	r0, [r0]

mov	r1, #SECBOOT_STAGE_1
str	r1,[r0]   /* acknowledge stage 1 */
dsb

ldr	r0,=sec_core_start_addr
ldr 	r0, [r0]
mov r1, #0
str r1,[r0]

bl	sw_gic_cpu_interface_init

/* Secondary core now waits for an event from primary core */
1:	wfe 
ldr	r0, =sec_core_start_addr

ldr	r0, [r0]
dsb

ldr	r1, [r0]
cmp	r1, #0   /* check if the address is set */
beq	1b

ldr	r0,=sec_core_start_stat_addr
ldr 	r0, [r0]
mov	r2, #SECBOOT_STAGE_2 
str	r2, [r0]   /* acknowledge stage 2 */

dsb
isb

/*
 * We are ready to move to NS world
 * Now update the ns context link register
 * and do the switching
 */

GET_CORE_CONTEXT	ns_sys_current
/* Update the context */
str	r1, [r0, #LR_MON_OFFSET]

dsb
isb

b	invoke_ns_kernel

.global go_to_wfe
.func go_to_wfe
go_to_wfe:
1:  wfe
ldr r0,=sec_core_start_addr
add r0,r0,r10
/*   GET_CPU_ID  r1
	 ldr r0, [r0, r1, lsl #2]  */
dsb
ldr r1, [r0]
cmp r1, #0   /* check if the address is set */
bxne r1
b   1b
.endfunc

