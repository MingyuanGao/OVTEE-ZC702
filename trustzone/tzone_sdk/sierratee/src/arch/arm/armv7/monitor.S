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
 * Monitor functions implementation
 */

#include <cpu_asm.h>
#include <asm_macros.h>	
#include <sw_platform_asm.h>
#include <sw_device_io.h>
#include <asm-offsets.h>

.macro exception_return 	
	ldmfd	sp!, {r0}
	msr	spsr, r0
	push	{r0}
	mov	r0, #0
	dsb
	pop	{r0}
	ldmfd	sp!, {r0-r12, pc}^
.endm	

.macro exception_return_arg	
	ldmfd	sp!, {r1}
	msr	spsr, r1
	push	{r1}
	mov	r1, #0
	dsb
	pop	{r1}
	ldmfd	sp!, {r1-r12, pc}^
.endm	

.macro call_c_function func_name
	stmfd	sp!, {r0-r12, lr}
	mrs	r4, spsr
	stmfd	sp!, {r4}
	bl	\func_name
.endm	

.macro call_c_function_arg func_name
	stmfd   sp!, {r1-r12, lr}
	mrs	r4, spsr
	stmfd   sp!, {r4}
	bl	\func_name
.endm


.macro mon_switchto_nsworld_ctx
	GET_CORE_CONTEXT s_sys_current
	bl	save_context
	GET_CORE_CONTEXT ns_sys_current
	bl	restore_context
	@ clear local monitor
	@-------------------
	clrex					
.endm
	

.align 12
.global monitor
monitor: 
monitor_reset:
	b   monitor_reset
monitor_undef:
	b   monitor_undef
	b   smc_handler
monitor_pref:
	b   monitor_pref
monitor_abort:
	b   monitor_abort
monitor_reserv:
	b   monitor_reserv
monitor_irq:
	b   monitor_irq
	b  monitor_fiq_handler

@ FIQ exception handler enters and runs in "monitor" mode
@ (not "FIQ" mode)
@ With SCR.FIQ set, FIQs from both secure and non secure world
@ are routed to monitor mode fiq handler.
monitor_fiq_handler: 
	sub	 lr, lr, #4
	push	{r0}
	mrc	 p15, 0, r0, c1, c1, 0
	tst	 r0, #SCR_NS_BIT
	pop	 {r0}
	beq called_from_secure
	bne called_from_ns
called_from_ns:
	 push	{r4, lr}  /* the corresponding pops happens from
				save_context */
	 push	{r0 - r3}
	 b	  ns_fiq_handler
called_from_secure:
	 call_c_function fiq_c_handler
	 exception_return
	

/* 
	save context 
	r0 - points to the base address where the register context get saved
	Note: r0-r3 has to be at the top of the stack  followed by r4-lr	
 */
.func save_context
.global save_context
save_context:	
	mov	r4, r0 

	pop	{r0 - r3}
	stmia	r4, {r0 - r3}
	mov	r3, r4

/* Update r4 so that the subsequent register context saving is done at the 
   right place */
	add	r4, r4, #SAVE_CTXT_TEMP_OFFSET			   
	mov	r0, r4	

	mov	r2, lr
	
	pop	{r4, lr}
	
	stmia	r0!, {r4-r12}
	
	mrs	r1, spsr
	stmia	r0!, {r1, lr}

	mov	lr, r2
	
	cps #ARCH_SVC_MODE	
	mrs	 r1, spsr
	stmia   r0!, {r1, r13, lr}

	cps #ARCH_SYS_MODE	
	stmia   r0!, {r13, lr}

	cps #ARCH_ABT_MODE	
	mrs	 r1, spsr
	stmia   r0!, {r1, r13, lr}

	cps #ARCH_UNDEF_MODE	
	mrs	 r1, spsr
	stmia   r0!, {r1, r13, lr}

	cps #ARCH_IRQ_MODE	
	mrs	 r1, spsr
	stmia   r0!, {r1, r13, lr}

	cps #ARCH_MON_MODE
#ifdef CONFIG_NEON_SUPPORT
	/* r3 on adding with NEON_OFFSET
	 points to the address where the neon context get saved */
	add	 r3,r3, #NEON_OFFSET	 	
	
	fmrx	r1, FPEXC
	tst	r1, #FPEXC_EN			   
	beq	1f						  
  
	/* Save the registers if vfp unit is enabled */	  
	fmrx	r2, FPSCR
	fmrx	r4, FPSID
	fmrx	r5, MVFR0
	
	/* Save FPEXC,FPSCR,FPSID registers */
	stmia   r3!, {r1,r2,r4}
	/* Save D0- D15 registers */
	vstm	r3!, {d0 - d15}
	
	/* Check support for Advanced SIMD registers */
	/* If required save d16-d31 */
	and	r5,r5, #MVFR0_A_SIMD_MASK
	cmp	r5,#A_SIMD_REG_BANK
	vstmeq	r3!, {d16 - d31}
#endif

1:  	mov	 pc, lr
.endfunc

/* 
	restore context 
	r0 - context pointer
 */
.func restore_context
.global restore_context
restore_context:	
	push	{lr}
#ifdef CONFIG_NEON_SUPPORT
	push	{r0}
   
	/* Add NEON_OFFSET value to the base address in r0. r0 points to the 
	   address from which the neon register context get restored*/
	add	r0,r0, #NEON_OFFSET	   
   
	/* Restore the saved registers. r1 - FPEXC, r2 -FPSCR and r3 - FPSID */
	ldmia   r0!, {r1,r2,r3}		   
	fmxr	FPEXC,r1
	
	/* Restore the context if vfp unit is enabled*/
	tst	r1, #FPEXC_EN			 
	beq	1f
	vldmia	r0!, {d0 - d15}
	
	fmrx	r4, MVFR0
	and	r4,r4, #MVFR0_A_SIMD_MASK
	/* Check support for Advanced SIMD registers */
	/* If required restore d16-d31 */	
	cmp	r4,#A_SIMD_REG_BANK				  
	vldmeq  r0!, {d16 - d31}
	
	fmxr	FPSCR,r2
	fmxr	FPSID,r3

1:  pop	 {r0}
#endif

	mov	r4, r0
	ldmia 	r4!, {r0 - r3}
	
	push 	{r0, r1}
	mov 	r0, r4

	ldmia	r0!, {r4 - r12}
	
	ldmia	r0!, {r1, lr}
	msr	spsr_cxsf, r1
	
	cps 	#ARCH_SVC_MODE	
	ldmia   r0!, {r1, r13, lr} 
	msr 	spsr_cxsf, r1

	cps 	#ARCH_SYS_MODE	
	ldmia   r0!, {r13, lr} 

	cps 	#ARCH_ABT_MODE	
	ldmia	r0!, {r1, r13, lr} 
	msr	spsr_cxsf, r1

	cps 	#ARCH_UNDEF_MODE	
	ldmia	r0!, {r1, r13, lr} 
	msr	spsr_cxsf, r1	

	cps 	#ARCH_IRQ_MODE	
	ldmia	r0!, {r1, r13, lr} 
	msr	spsr_cxsf, r1

	cps 	#ARCH_MON_MODE	
	pop	{r0, r1}
	ldr	pc, [r13], #4
.endfunc

smc_handler:
	cmn	 r0, #0
	blt	 board_smc
	CMP r0, # CALL_TRUSTZONE_API
	beq	 tz_api
	CMP r0, #RET_FROM_SECURE_API
	beq	 ret_from_secure_api
	CMP r0, #TZHYP_NSADMIN_RETURN
	beq	 tzhyp_nsadmin_restore
	CMP r0, #INVOKE_NON_SECURE_KERNEL
	beq	 call_non_secure_kernel  
	CMP r0, #TZHYP_NSCPU_CTXT_INIT
	beq	 tzhyp_cpuctxt_init
	CMP r0, #RET_TO_NON_SECURE_KERNEL
	beq     non_secure_return_path 

	bx	lr

board_smc:
	call_c_function_arg board_smc_handler
	exception_return_arg

smc_error:
	call_c_function smc_error_handler
	exception_return
	
tz_api:
#ifdef CONFIG_SW_MULTICORE
	push	{r0}
	GET_CPU_ID  r0
	cmp	r0, #0
	pop	{r0}
	beq	1f
	b	smc_error
#endif
	
#ifndef CONFIG_SW_DEDICATED_TEE
1:	push	{r4, lr}  /* the corresponding pops happens from
				 save_context */
	push	{r0 - r3}

	/* Copy args to params stack */
	ldr	r4, =params_stack
	stmia  	r4, {r0-r3}

	ldr 	r1, =valid_params_flag
	mov 	r2, #0x1
	str 	r2, [r1]
	
	b 	mon_switchto_sworld
#else
   /* Invalid case */   
#endif	
	
call_non_secure_kernel:
#ifndef CONFIG_SW_DEDICATED_TEE
	push	{r4, lr}  /* the corresponding pops happens from
									 save_context */
	push	{r0 - r3}
	
#ifndef CONFIG_BOOT_SVISOR
	b 	mon_switchto_nsworld
#else
	mon_switchto_nsworld_ctx
	b   switch_to_hyp_mode
#endif  /* CONFIG_BOOT_SVISOR */

#else   /* CONFIG_SW_DEDICATED_TEE */
	push	{lr}
	mov	r0, #0
	bl	start_secondary_linux
	pop	{lr}
	movs	pc, lr
#endif

ret_from_secure_api:
#ifndef CONFIG_SW_DEDICATED_TEE	
	push	{r4, lr}  /* the corresponding pops happens from
									 save_context */
	push	{r0 - r3}

	ldr 	r0, =params_out_stack /* Update return value */
	ldr 	r1, [r0, #0]
	ldr 	r0, =ns_sys_current
	ldr 	r0, [r0]
	str 	r1, [r0, #0]		   @ r0 at offset 0

	b 	mon_switchto_nsworld
#else
	push	{lr}
	bl  	notify_smp_core
	pop	{lr}
	movs 	pc,lr
#endif


tzhyp_cpuctxt_init:
	call_c_function	  mon_nscpu_context_init
	exception_return

tzhyp_nsadmin_restore:
	@ NS bit is already set
	@ restore the SCTRL
	GET_CORE_CONTEXT ns_sys_current
	mov	r1, r0	
	ldr	r0, [r1, #SCTLR_OFFSET]		
	mcr	p15, 0, r0, c1, c0, 0 /* SCTLR */
	
	@ restore SPSR
	ldr	r0, [r1, #SPSR_MON_OFFSET]
	msr	spsr, r0
	
	@ restore lr
	ldr	r0, [r1, #LR_MON_OFFSET]
	mov	lr, r0	

	b	tzhyp_nsadmin_return @ continue to fiq handler return path

	
.func  mon_switchto_nsworld
mon_switchto_nsworld:
	mon_switchto_nsworld_ctx

	push {r0}
	scr_nsbit_set r0	
	
	pop  {r0}

	dsb
	isb

	movs	pc, lr
.endfunc

.func  mon_switchto_sworld
mon_switchto_sworld:
	scr_nsbit_clear r0
	
	GET_CORE_CONTEXT ns_sys_current
	bl	save_context
	GET_CORE_CONTEXT s_sys_current
	bl	restore_context
	clrex						 

	movs	pc, lr
.endfunc

.func switch_to_hyp_mode
switch_to_hyp_mode:
	push    {r0}
	/* 
	CPU has hyper mode? 
	12-15 bits of Processor feature register 1 define virt extensions 
	*/
	mrc     p15, 0, r0, c0, c1, 1
	and     r0, r0, #0xf000
	teq     r0, #0x1000             
	beq     1f
	b       svisor_entry_fail
1:
	/*
        Set HCE and NS bit in SCR register  
	*/
	scr_nsbit_set r0
	scr_hcrbit_set r0

	/* Set Generic Timer Clock frequency */
	ldr     r0, =CNTFRQ_VAL
	mcr     p15, 0, r0, c14, c0, 0
	/*
        Get Current CPSR and change the mode to Hypervisor.
        Save the changed value in SPSR.
        Exception return to Hypervisor mode
	*/

	mrs     r0, cpsr
	bic     r0, r0, #0x1f
	orr     r0, r0, #ARCH_HYP_MODE           
	msr     spsr_cxsf, r0

	pop     {r0}
	
	dsb
	isb
	movs    pc, lr	
svisor_entry_fail:
1:      wfe
        b     1b
.endfunc
	

.func   ns_fiq_handler
ns_fiq_handler:
	@ Move to secure world	
	scr_nsbit_clear r0

	ldr r1, =global_val
	add r0, r1, #TZHYP_NS_PREEMPT_FLAG_OFFSET
	ldr	r1, =NS_PREEMPT_ENABLE	
	str	r1, [r0]
	dsb	

	GET_CORE_CONTEXT ns_sys_current
	bl	save_context
	GET_CORE_CONTEXT s_sys_current
	bl	restore_context
	
	bl	fiq_c_handler

	GET_CORE_CONTEXT s_sys_current
	bl	restore_context
	
	push	{r0, r1}
	ldr r1, =global_val
	add r0, r1, #TZHYP_NS_PREEMPT_FLAG_OFFSET
	ldr	r1, =NS_PREEMPT_DISABLE	
	str	r1, [r0]
	
	ldr r1, =global_val
	add r0, r1, #G_LINUX_RETURN_FLAG_OFFSET
	ldr	r1, =LINUX_RET_SET 
	str	r1, [r0]
	
	clrex
	pop	{r0, r1}
	movs	pc,lr

tzhyp_nsadmin_return:
	@Clear switch flag
	ldr r1, =global_val
	add r0, r1, #TZHYP_NS_SWITCH_FLAG_OFFSET
	ldr	r1, =NS_SWITCH_CLEAR
	str	r1, [r0]
	
skip_nsadmin:	
	pop	{r0, r1}	

	dsb
	isb

	movs	 pc, lr
.endfunc


.func non_secure_return_path
non_secure_return_path:
#ifndef CONFIG_SW_DEDICATED_TEE	
	push	{r4, lr}  /* the corresponding pops happens from
		                         save_context */
	push	{r0 - r3}

	GET_CORE_CONTEXT s_sys_current
	bl	save_context
	GET_CORE_CONTEXT ns_sys_current
	bl	restore_context
	clrex                         
#else
	push	{lr}
	bl	notify_smp_core
	pop	{lr}
#endif
	push	{r0, r1}

	ldr r1, =global_val
	add r0, r1, #G_LINUX_RETURN_FLAG_OFFSET
	ldr	r1, =LINUX_RET_CLEAR	
	str	r1, [r0]

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
	GET_CORE_CONTEXT ns_sys_current
	ldr	r1, [r0, #PENDING_NS_NOTIFY]
	cmp	r1, #1
	bne	skip_pending_notify

	ldr r1, =global_val
	add r1, r1, #G_NS_NOTIFY_PENDING_OFFSET
	cmp r1, #1
	beq	skip_pending_notify
	push {r0-r12, lr}
	mov	r0, #NS_SGI_NOTIFY_INTERRUPT
	mov r1, #SGI_TGT_CPU0
	bl 	sw_generate_soft_int
	pop	{r0-r12, lr}
	mov	r1, #0 
	str	r1, [r0, #PENDING_NS_NOTIFY]
skip_pending_notify:
#endif	

	scr_nsbit_set r0

	@ Left over of tzhyp context switching which cannot be done from
	@ secure world, accomplished by switching to "nsadmin"
	ldr r1, =global_val
	add r0, r1, #TZHYP_NS_SWITCH_FLAG_OFFSET
	ldr     r0, [r0]
	tst     r0, #NS_SWITCH_ACTIVE
	beq     skip_nsadmin

	b       tzhyp_nsadmin
   
.endfunc    

.func tzhyp_nsadmin
@ Switch to "nsadmin"(Non secure admin) for TLB maintainence
tzhyp_nsadmin:
	
	@ NS bit is already set

	ldr     r0, =SCTLR_RESET_VAL
	mcr	    p15, 0, r0, c1, c0, 0 /* SCTLR */

	ldr     r0, =CPSR_RESET_VAL
	msr	    spsr, r0              /* SPSR */

	ldr     lr, =NSADMIN_LOAD     /* lr   */

	dsb
	isb

	movs    pc, lr
.endfunc

@ This section need to be placed in the Non secure area
@ This can be acheived by just copying code from secure area to non secure area	
@ We enter with MMU turned off

/* Fix me: Enabling a new section breaks the startaddr 
 * .section .nsadmin, "a"
 */
.global tzhyp_nsadmin_start
.global tzhyp_nsadmin_end
tzhyp_nsadmin_start:	
	mcr	p15, 0, r0, c8, c7, 0  /* Invalidate entire TLB */
	ldr	r0, nsadmin_retval
	dsb
	isb
#if USE_ARCH_EXTENSION_SEC
.arch_extension sec
#endif
	smc	 #0
nsadmin_retval:	
	.word   TZHYP_NSADMIN_RETURN
tzhyp_nsadmin_end:	
@section end
	
	
