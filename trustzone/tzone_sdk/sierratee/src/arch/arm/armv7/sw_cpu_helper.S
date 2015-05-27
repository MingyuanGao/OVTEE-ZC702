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
 * Generic ARM Helper routines 
 */
#include <sw_arm_defines.h>

.global jump_to_sys_mode
.func jump_to_sys_mode
jump_to_sys_mode:
	@void jump_to_sys_mode(void)
	mov r0, lr
	msr CPSR_c, #(ARCH_SYS_MODE | IRQ_BIT)
	mov pc, r0
.endfunc

.global get_cp15_mpid
.func get_cp15_mpid
get_cp15_mpid:
	@ sw_uint get_cp15_mpid(void)
	mrc	p15, 0, r0, c0, c0, 5
	bx	lr
.endfunc


.global get_cp15_sctlr
.func get_cp15_sctlr
get_cp15_sctlr:
	@ sw_uint get_cp15_sctlr(void)
	mrc	p15, 0, r0, c1, c0, 0
	bx	lr
.endfunc

.global set_cp15_sctlr
.func set_cp15_sctlr
set_cp15_sctlr:
	@ void set_cp15_sctlr(sw_uint)
	mcr 	p15, 0, r0, c1, c0, 0
	isb
	bx	lr
.endfunc

.global get_cp15_dfsr
.func get_cp15_dfsr
get_cp15_dfsr:
	@ sw_uint get_cp15_dfsr(void)
	mrc 	p15, 0, r0, c5, c0, 0
	bx 	lr
.endfunc

.global get_cp15_dfar
.func get_cp15_dfar
get_cp15_dfar:
	@ sw_uint get_cp15_dfar(void)
	mrc 	p15, 0, r0, c6, c0, 0
	bx 	lr
.endfunc


.global get_cp15_ifsr
.func get_cp15_ifsr
get_cp15_ifsr:
	@ sw_uint get_cp15_ifsr(void)
	mrc 	p15, 0, r0, c5, c0, 1
	bx 	lr
.endfunc

.global get_cp15_ifar
.func get_cp15_ifar
get_cp15_ifar:
	@ sw_uint get_cp15_ifar(void)
	mrc 	p15, 0, r0, c6, c0, 2
	bx 	lr
.endfunc

.global set_cp15_ttbcr
.func set_cp15_ttbcr
set_cp15_ttbcr:
	@ void set_cp15_ttbcr(sw_uint)
	mcr 	p15, 0, r0, c2, c0, 2
	isb
	bx 	lr
.endfunc

.global get_cp15_ttbcr
.func get_cp15_ttbcr
get_cp15_ttbcr:
	@ sw_uint get_cp15_ttbcr(void)
	mrc 	p15, 0, r0, c2, c0, 2
	isb
	bx 	lr
.endfunc

.global get_cp15_dacr
.func get_cp15_dacr
get_cp15_dacr:
	@ sw_uint get_cp15_dacr(void)
	mrc 	p15, 0, r0, c3, c0, 0
	bx 	lr
.endfunc

.global set_cp15_dacr
.func set_cp15_dacr
set_cp15_dacr:
	@ void set_cp15_dacr(sw_uint)
	mcr 	p15, 0, r0, c3, c0, 0
	isb
	bx 	lr
.endfunc


.global get_cp15_ttbr0
.func get_cp15_ttbr0
get_cp15_ttbr0:
	@ sw_uint get_cp15_ttbr0(void)
	mrc 	p15, 0, r0, c2, c0, 0
	bx 	lr
.endfunc

.global set_cp15_ttbr0
.func set_cp15_ttbr0
set_cp15_ttbr0:
	@ void set_cp15_ttbr0(sw_uint)
	mcr 	p15, 0, r0, c2, c0, 0
	isb
	bx 	lr
.endfunc

.global get_cp15_ttbr1
.func get_cp15_ttbr1
get_cp15_ttbr1:
	@ sw_uint get_cp15_ttbr1(void)
	mrc 	p15, 0, r0, c2, c0, 1
	bx 	lr
.endfunc

.global set_cp15_ttbr1
.func set_cp15_ttbr1
set_cp15_ttbr1:
	@ void set_cp15_ttbr1(sw_uint)
	mcr 	p15, 0, r0, c2, c0, 1
	isb
	bx 	lr
.endfunc

.global set_cp15_asid
.func set_cp15_asid
set_cp15_asid:
	@ void set_cp15_asid(sw_uint)
	mcr p15, 0, r0, c13, c0, 1	
	isb
	bx 	lr
.endfunc

.global branch_predictor_inv_all
.func branch_predictor_inv_all
branch_predictor_inv_all:
	@ void branch_predictor_inv_all(void)
	push 	{r0}
	mov	r0, #0
	mcr 	p15, 0, r0, c7, c5, 6
	isb
	pop	{r0}
	bx 	lr
.endfunc

.global enable_branch_predictor
.func enable_branch_predictor
enable_branch_predictor:
	@ void enable_branch_predictor(void)
	push 	{r0}
	mrc	p15, 0, r0, c1, c0, 0
	orr 	r1, r1, #SCTLR_BRANCH_PRED_BIT
	mcr 	p15, 0, r0, c1, c0, 0
	isb
	pop	{r0}
	bx 	lr
.endfunc

.global busy_loop
.func busy_loop
busy_loop:
	@ void busy_loop(void)
	wfi
	b	busy_loop
.endfunc


.global cpu_save_irq_state
.func cpu_save_irq_state
cpu_save_irq_state:
	@ sw_uint cpu_save_irq_state(void)
	mrs	r0, cpsr
	cpsid	if
	bx	lr
.endfunc

.global cpu_restore_irq_state
.func cpu_restore_irq_state
cpu_restore_irq_state:
	@ void cpu_restore_irq_state(sw_uint cpsr_irq)
	msr	cpsr, r0
	bx	lr
.endfunc

.global tee_irq_enable
.func tee_irq_enable
tee_irq_enable:
	@ void tee_irq_enable(void)
	cpsie	f
	bx	lr
.endfunc


.global tee_irq_disable
.func tee_irq_disable
tee_irq_disable:
	@ void tee_irq_disable(void)
	cpsid	if
	bx	lr
.endfunc

.global get_cpu_id
.func get_cpu_id
  @ sw_uint get_cpu_id(void)
get_cpu_id:
	mrc	p15, 0, r0, c0, c0, 5
	and	r0, r0, #0x03
	bx	lr
.endfunc

.global init_cpu_state
.func init_cpu_state
init_cpu_state:
	push	{r0, r1, lr}
@ Disable MMU 
	mrc 	p15, 0, r1, c1, c0, 0
	bic 	r1, r1, #0x1
	mcr 	p15, 0, r1, c1, c0, 0

@ Disable L1 Caches 
	mrc 	p15, 0, r1, c1, c0, 0
	bic 	r1, r1, #SCTLR_ICACHE_BIT
	bic 	r1, r1, #SCTLR_DCACHE_BIT
	mcr 	p15, 0, r1, c1, c0, 0

@ Invalidate Instruction cache 
	mov 	r1,#0
	mcr 	p15, 0, r1, c7, c5, 0

@ Invalidate Data caches 
	mov	r0, #1
	bl 	data_cache_clean_invalidate_all

@ Invalidate Branch predictor arrays 
	mov 	r1,#0
	mcr	p15, 0, r1, c7, c5,	6

@ Invalidate TLBs 
	mov 	r1, #0x0
	mcr 	p15, 0, r1, c8, c3, 0	
	
	pop	{r0, r1, lr}
	bx	lr
.endfunc	
