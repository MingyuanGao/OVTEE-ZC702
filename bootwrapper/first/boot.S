/*
 * boot.S - simple register setup code for stand-alone Linux booting
 *
 * Copyright (C) 2009 ARM Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

    .text

    .globl    _start
_start:
    @
    @ Determine whether MPIDR is available
    @
    mrc    p15, 0, r0, c0, c0, 0  @ MIDR
    and    r0, r0, #0xf0000       @ Architecture
    cmp    r0, #0xf0000           @ Uses CPUID scheme? (v7, some v6s)
    bne    2f
    @ 1176 uses CPUID but has no MPIDR, so must check for it
    mrc    p15, 0, r0, c0, c0, 0  @ MIDR
    ldr    r1, =0xb760            @ Part number for 1176
    and    r0, r0, r1
    cmp    r0, r1                 @ Is an 1176?
    beq    2f

    @
    @ CPU initialisation
    @
    mrc    p15, 0, r0, c0, c0, 5  @ MPIDR (ARMv7 and 11MPCore only)
    and    r0, r0, #15            @ CPU number
    cmp    r0, #0                 @ primary CPU?
    beq    2f

    @
    @ Secondary CPUs 
    @

1:  wfe    
    beq    1b
	
	

    @
    @ Kernel parameters
    @
2:    mov    r0, #0
    ldr    r1, =3343            @ Xilinx
    mov	   r2,#0x1000000        @ DTB at fixed address
    mov    r3, #0
    ldr    lr, =kernel
@    orr    lr, lr, #1          @ Thumb-2 kernel
    mov     pc, lr             @ jump to the kernel

