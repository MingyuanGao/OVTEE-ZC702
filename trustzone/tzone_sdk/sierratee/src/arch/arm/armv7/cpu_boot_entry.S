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
 * cpu_boot_entry implementation
 */

/* Entry point */
#include <cpu_asm.h>
#include <sw_device_io.h>
#include <sw_platform_asm.h>
#include <sw_arm_defines.h>

.text
.align 4
.globl _start_boot
_start_boot:
	b 	_start_boot_1


.align 12
.global secure_boot_exception_vectors
secure_boot_exception_vectors:
/*
Vector table 
*/
	b	_boot_reset_handler
	b	_boot_undefined_handler
	b	_boot_swi_handler
	b	_boot_prefetch_handler
	b	_boot_abort_handler
	b	_boot_reserved_vector
	b	_boot_irq_handler
	b 	_boot_fiq_handler

_boot_reset_handler:
	b	_boot_reset_handler

_boot_reserved_vector:
	b	_boot_reserved_vector

_boot_fiq_handler:
	b	_boot_fiq_handler

_boot_undefined_handler:
	b	_boot_undefined_handler

_boot_swi_handler:
	b	_boot_undefined_handler

_boot_prefetch_handler:
	b	_boot_prefetch_handler

_boot_abort_handler:
	b	_boot_abort_handler

_boot_irq_handler:
	b 	_boot_irq_handler


.globl _start_boot_1
_start_boot_1:
@	b 	_start_boot_1
	bl	get_cpu_id		@ Returns CPU ID in r0
	cmp	r0, #0			@ Is this CPU0?
	blne	busy_loop

__boot_secure_core:
@ Set VBAR
	ldr	r1, =secure_boot_exception_vectors	
	mcr	p15, 0, r1, c12, c0, 0

	msr	cpsr_c,#(ARCH_SVC_MODE | IRQ_BIT | FIQ_BIT)
	ldr	sp, =_IRAM_STACK

	mov 	r3, pc
	ldr 	r4, =SECURE_BL_START_ADDR
	ldr	r5, =0xffff0000
	and	r3, r3, r5
	and	r4, r4, r5
	cmp	r3, r4
	bne	cont_init		
	ldr	r0, =SECURE_BL_START_ADDR
	ldr	r1, =SECURE_BL_RELOCATE_ADDR
	ldr	r2, =_code_size
	bl	relocate_copy
	ldr	r1, =SECURE_BL_RELOCATE_ADDR
	mov	pc, r1

cont_init:
	/* Call CPU init function */
	bl	init_cpu_state
	
	bl	c_boot_env_init
	
	bl	secure_boot
	/* We should never reach here */
	b	.


relocate_copy:
	mov	r3, r2
cp_start:
	cmp	r3, #0
	beq	cp_done
	cmp	r3, #16
	bge	cp_byte
cp_word:
	ldmia	r0!, {r4}
	stmia	r1!, {r4}
	sub	r3, r3, #0x4
	b	cp_start
cp_byte:
	ldmia	r0!, {r4 - r7}
	stmia	r1!, {r4 - r7}
	sub	r3, r3, #0x10
	b	cp_start
cp_done:
	mov	r0, r2
	bx	lr
cp_end:



.global c_boot_env_init
c_boot_env_init:
	/* bss init */
	ldr	r1, =_SW_BOOT_BSS_START
	ldr	r2, =_SW_BOOT_CODE_END
	mov	r0, #0	
2:	cmp	r1, r2	
	bgt	1f
	str	r0,	[r1], #4
	b	2b	
1:	mov	pc, lr		

.global jump_to
jump_to:
	ldr	r0, =SECURE_OS_ENTRY_ADDR
	mov	pc, r0
	
