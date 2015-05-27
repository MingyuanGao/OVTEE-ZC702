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
 * cpu entry implementation
 */

/* Entry point */
#include <cpu_asm.h>
#include <sw_arm_defines.h>
#include <sw_device_io.h>	
#include <sw_platform_asm.h>
#include <asm_macros.h>	
#include <sw_mmu_helper_asm.h>
#include <sw_syscalls_id.h>

.text
.align 4
.globl _start
_start:
	b _start_1


.align 12
.global secure_exception_vectors
secure_exception_vectors:
/*
Vector table 
*/
	b	_tee_reset_handler
	b	_tee_undefined_handler
	b	_tee_swi_handler
	b	_tee_prefetch_handler
	b	_tee_abort_handler
	b	_tee_reserved_vector 
	b	_tee_irq_handler
	b	_tee_fiq_handler

_tee_reset_handler:
	b	_tee_reset_handler

_tee_reserved_vector:
	b	_tee_reserved_vector

_tee_fiq_handler:
	b	_tee_fiq_handler

.global _tee_undefined_handler
_tee_undefined_handler:
	bl	tee_undefined_handler

.global _tee_swi_handler
.func   _tee_swi_handler
_tee_swi_handler:
	push  {r5, r6}
	ldr   r5, [lr, #-4] 
	bic   r5, r5, #0xff000000
/*Assuming we are using only 16 bit syscall id */
	movw  r6, #SW_SYSCALL_EXIT
	cmp   r5, r6
	pop   {r5, r6}
	beq   skip_context_saving
	
	stmfd sp!, {r0-r12, lr}
	mrs   r2, spsr
	stmfd sp!, {r2}
	mov   r1, sp
	
skip_context_saving:
	ldr   r0, [lr, #-4] 
	bic   r0, r0, #0xff000000
	push  {r0}
	mrs   r0, spsr
	and   r0, r0, #0x1F
	cmp   r0, #ARCH_USR_MODE
	pop   {r0}

	msreq CPSR_c, #(ARCH_SYS_MODE | IRQ_BIT)	
	bl    tee_swi_handler
	msr   CPSR_c, #(ARCH_SVC_MODE | IRQ_BIT)	

	ldmfd sp!, {r2}
	msr   spsr, r2
	push  {r0}
	mov   r0, #0
	dsb
	pop   {r0}
	ldmfd sp!, {r0-r12, pc}^ 
.endfunc

.global _tee_prefetch_handler
_tee_prefetch_handler:
	bl	tee_prefetch_abort_handler

.global _tee_abort_handler
_tee_abort_handler:
	sub	 lr, lr, #8
	stmfd	sp!, {r0-r12, lr}
	mrs	r1, spsr
	stmfd   sp!, {r1}
	mov   	r0, sp 
	bl		tee_data_abort_handler
	ldmfd   sp!, {r2}
	msr	spsr, r2
	push	{r0}
	mov	r0, #0
	dsb
	pop	{r0}
	ldmfd	sp!, {r0-r12, pc}^ 

_tee_irq_handler:
	b	_tee_irq_handler

.global get_cpu_cores
.func get_cpu_cores
  @ sw_uint get_cpu_cores(void)
get_cpu_cores:
	mov	r0, #1
#ifdef CONFIG_CORTEX_A15
/* Read L2 control Register and 25:24 is number of processors */
	mrc	p15, 1, r0, c9, c0, 2
	ror	r0, #24
	and	r0, r0, #0x03
	add	r0, r0, #1
#endif
#ifdef CONFIG_CORTEX_A9
	mrc	p15,4,r0,c15,c0,0
	cmp	r0, #0
	beq	ret_cores
	ldr	r0, =__scu_base_reg   
	ldr	r0, [r0]   
	add	r0, r0, #0x4
	ldr	r1, [r0]
	and	r0, r1, #0x3
	add	r0, r0, #1
#endif
ret_cores:
	bx	lr
.endfunc

.globl _start_1
_start_1:
	/* Compute the physical offset between load and execute address */
	ldr	r0, =_start_1
	adr	r9, _start_1
	sub	r10, r9, r0

#ifdef CONFIG_SW_DEDICATED_TEE
/*  In uniprocessor system,  run secure and non-secure os on core 0.
	In multi-processor system,  run secure in core 1 and non-secure os on other 
	cores.
*/
	bl	get_cpu_cores
	cmp	r0, #1
	bne	check_cpu_id
	mov	r0, #0
	b	__start_secure_core
check_cpu_id:
	ldr	r1, =multi_core_mode
	mov	r2, #1
	str	r2, [r1]

/* Other than secure cpu and other core should execute secondary entry point */
	bl	get_cpu_id
	cmp	r0, #1
	blne	_secondary_cpus_start
#else
	GET_CPU_ID	r0
	cmp	r0,#0
#ifdef CONFIG_SW_MULTICORE

#ifdef CONFIG_SW_NOBOOTLOADER
	blne	go_to_wfe
#endif	/* SW_NOBOOTLOADER */
	
#else
	blne	busy_loop
#endif /* SW_MULTICORE */
	
#endif /* SW_DEDICATED_TEE */

__start_secure_core:
/*MMU Enabling steps here*/
/* use temporary stack for initialization */
	msr	CPSR_c,#(ARCH_SVC_MODE | IRQ_BIT | FIQ_BIT)
	ldr	r1,=tee_svc_stack
	add	r1, r1, r10
	add	r1, r1, r0, lsl #STACK_SIZE_SHIFT
	add	sp, r1, #STACK_SIZE

	bl	cpu_init
	
	GET_CPU_ID	r0
	cmp	r0, #0
	bleq	c_env_init
	cmp	r0, #0
	bleq	create_page_table

	bl		enable_mmu
	
	ldr	r3, =virtual_addr
	mov	pc, r3

virtual_addr:

	GET_CPU_ID  r0

/* Set VBAR */
	ldr	r1, =secure_exception_vectors
	mcr	p15, 0, r1, c12, c0, 0

/* Set monitor base address */
	ldr	r1, =monitor
	mcr	p15, 0, r1, c12, c0, 1

/* Setup stacks for all modes */
	msr	CPSR_c,#(ARCH_IRQ_MODE | IRQ_BIT | FIQ_BIT)
	ldr	r1,=tee_irq_stack
	add	r1, r1, r0, lsl #STACK_SIZE_SHIFT
	add		sp, r1, #STACK_SIZE

	msr	CPSR_c,#(ARCH_FIQ_MODE | IRQ_BIT | FIQ_BIT)
	ldr	r1,=tee_fiq_stack
	add	r1, r1, r0, lsl #STACK_SIZE_SHIFT
	add	sp, r1, #STACK_SIZE

	msr	CPSR_c,#(ARCH_ABT_MODE | IRQ_BIT | FIQ_BIT)
	ldr	r1,=tee_abort_stack
	add	r1, r1, r0, lsl #STACK_SIZE_SHIFT
	add	sp, r1, #STACK_SIZE

	msr	CPSR_c,#(ARCH_UNDEF_MODE | IRQ_BIT | FIQ_BIT)
	ldr	r1,=tee_undefined_stack
	add	r1, r1, r0, lsl #STACK_SIZE_SHIFT
	add	sp, r1, #STACK_SIZE

	msr	CPSR_c,#(ARCH_SYS_MODE | IRQ_BIT | FIQ_BIT)
	ldr	r1,=tee_user_stack
	add	r1, r1, r0, lsl #STACK_SIZE_SHIFT
	add	sp, r1, #STACK_SIZE

	msr	CPSR_c,#(ARCH_MON_MODE | IRQ_BIT )
	ldr	r1,=tee_monitor_stack
	add	r1, r1, r0, lsl #STACK_SIZE_SHIFT
	add	sp, r1, #STACK_SIZE

	msr	CPSR_c,#(ARCH_SVC_MODE | IRQ_BIT | FIQ_BIT)
	ldr	r1,=tee_svc_stack
	add	r1, r1, r0, lsl #STACK_SIZE_SHIFT
	add	sp, r1, #STACK_SIZE

#ifdef CONFIG_SW_MULTICORE	
	GET_CPU_ID	r0
	cmp	r0, #0
	blne	secondary_main
#endif

/* 
	Jump to TEE 'c' entry function and system shouldn't come out of 
	this function 
*/	
	bl	secure_main

	b	.

.global c_env_init
c_env_init:
	/* bss init */
	ldr	r1, =_SW_BSS_START
	add	r1, r1, r10
	ldr	r2, =_SW_CODE_END
	add	r2, r2, r10
	mov	r0, #0	
2:  	cmp	r1, r2	
	bgt	1f
	str	r0,	[r1], #4
	b	2b	
1:  	mov	pc, lr		

.global cpu_init
.func cpu_init
cpu_init:
	push {r1, lr}
	
	bl	init_cpu_state
	
	/* Set SCTLR reset value */
	mrc	 p15, 0, r1, c1, c0, 0
	bic	 r1, r1, #0x10000000
	bic	 r1, r1, #0x00002000
	mcr	 p15, 0, r1, c1, c0, 0

	/* Set NSACR reset value */
	mrc	 p15, 0, r1, c1, c1, 2		
	ldr	 r2, =NSACR_REG_VAL
	orr	 r1, r1, r2
	mcr	 p15, 0, r1, c1, c1, 2

	/* Set the FIQ bit so as to route FIQs to monitor mode */
	mrc	 p15, 0, r1, c1, c1, 0		
	ldr	 r2, =SCR_FIQ_BIT			 
	orr	 r1, r1, r2	
	mcr	 p15, 0, r1, c1, c1, 0

	/* set Auxiliary register reset value */
	mrc	 p15, 0, r0, c1, c0, 1	
#if (CONFIG_SW_MULTICORE || CONFIG_ZYNQ7_BOARD)
	ldr	 r1, =AUXREG_SMP
	orr	 r0, r0, r1
#else
	ldr  r0, =AUXREG_UP
#endif
	mcr	 p15, 0, r0, c1, c0, 1
	
#ifdef CONFIG_NEON_SUPPORT 
	/* Enabling the NEON Coprocessor registers */
	
	/* Enable access to CP10 and CP11 */
	mov	 r1,#NEON_CP_ACCESS
	mcr	 p15, 0, r1, c1, c0, 2

	/* Set FPEXC register EN bit */
	mov	 r1, #FPEXC_EN_SET
	msr	 fpexc, r1

	/* Clear ASEDIS bit in CPACR register */
	mrc	 p15, 0, r1, c1, c0, 2
	bic	 r1, r1, #ASEDIS_CLR
	mcr	 p15, 0, r1, c1, c0, 2
#endif
	pop {r1, lr}
	bx lr
.endfunc

.global enable_mmu
.func enable_mmu
enable_mmu:
	push	{ lr }
/* Enable mmu */
	dmb
	mcr	 p15, 0, r0, c7, c5, 0
	mov	 r0, #1
	bl	 data_cache_clean_invalidate_all
	bl	 flush_all_translation_table_entries
	dmb
	isb
	mov	 r2, #0
	mcr	 p15, 0, r2, c2, c0, 0 @set TTBR0 to Zero
	mcr	 p15, 0, r2, c2, c0, 1 @set TTBR1 to Zero
	mcr	 p15, 0, r2, c2, c0, 2 @set TTBCR to Zero

	GET_CPU_ID  r3
	cmp r3, #0

	ldreq  r2, =tmp_page_table
	ldrne  r2, =secure_page_table
	add  r2, r2, r10
	orr	 r3, r2, #TTBR_FLAGS
	mcr	 p15, 0, r3, c2, c0, 0 @set TTBR0 to kern_page_table physical addr
	mrc	 p15, 0, r3, c3, c0, 0 @read domain
	mov	 r4, #0 @domain value to set
	lsl	 r4, r4, #2
	mov	 r5, #3
	lsl	 r5, r5, r4
	neg	 r5, r5
	and	 r5, r5, r3
	mov	 r6, #1
	orr	 r5, r5, r6, lsl r4
	mcr	 p15, 0, r5, c3, c0, 0 @write domain
	dsb
	isb
	mrc	 p15, 0, r3, c1, c0, 0 @read sctlr
	mov	 r4, #1
	orr	 r3, r3, r4 @enable mmu
	orr	 r3, r3, r4, lsl #2 @enable dcache
	orr	 r3, r3, r4, lsl #12 @enable icache
	pop	 { lr }
	mcr	 p15, 0, r3, c1, c0, 0 @write sctlr
	isb
	mov	 pc, lr
.endfunc

#map section entries in page table.
#r2  = secure page table base
#r3  = virtual address
#r4  = physical address
#r5  = size of the map to create
#r6  = cachable
#r7  = page table index
#r8  = value at r7
#r9  = tex
#r10 = va_to_pa
#r11 = ap
#r12 = xn
.global init_page_table
.func init_page_table
init_page_table:
#calc l1 pte value
	push { r0 - r12 }
	movw r8, #4095 @SECTION_BASE_MASK
	and r8, r4, r8, lsl #20
	orr r8, r8, #2 @L1_TYPE_SECTION
	mov r0, #0
	orr r8, r8, r0, lsl #5 @DOMAIN
	mov r0, #1
	orr r8, r8, r0, lsl #2 @B
	orr r8, r8, r6, lsl #3 @C
	orr r8, r8, r9, lsl #12 @TEX
	orr r8, r8, r12, lsl #4 @XN
	and r0, r11, #3 @L1_SECTION_AP_MASK
	orr r8, r8, r0, lsl #10 @L1_SECTION_AP_SHIFT
	lsr r0, r11, #2
	and r0, r0, #1
	orr r8, r8, r0, lsl #15 @L1_SECTION_AP2_SHIFT
#ifdef CONFIG_SW_MULTICORE
	mov r0, #1
	orr r8, r8, r0, lsl #16 @Sharable
#else
	mov r0, #0
	orr r8, r8, r0, lsl #16
#endif
	mov r0, #0
	orr r8, r8, r0, lsl #17 @NG
	mov r0, #0
	orr r8, r8, r0, lsl #19 @NS
#calculated l1 pte value. now find entry pos
	mov r1, #0
2:  cmp r1, r5
	beq 1f
#find l1 pte index
	mov r7, r3
	add r7, r7, r1
	lsr r7, #20
	mov r0, #4
	mul r7, r7, r0
	add r7, r2, r7
	str r8, [r7]
#increment mapped size
	mov r0, #1
	add r1, r1, r0, lsl #20
	add r8, r8, r0, lsl #20
	b   2b
1:  pop { r0 - r12 }
	mov pc, lr
.endfunc

#ifdef CONFIG_CORTEX_A9
.global __scu_base_reg
__scu_base_reg:
	.word SCU_BASE
#endif	

.section normal_image, "a"
.global kernel_start
.global kernel_end	
kernel_start:
#ifndef CONFIG_BOOT_SVISOR
 .incbin "normal.bin";
#else
.incbin "svisor.bin";
#endif
kernel_end:

#ifdef CONFIG_MULTI_GUESTS_SUPPORT	
.section normal_image_2, "a"
.global kernel_2_start
.global kernel_2_end	
kernel_2_start:
.incbin "normal_2.bin";
kernel_2_end:

#ifndef CONFIG_ZYNQ7_BOARD
.section initrd_image, "a"
.global initrd_image_start
.global initrd_image_end	
initrd_image_start:
.incbin "linux2_initrd.bin.gz";
initrd_image_end:
#endif

#endif	

.section fs_image, "a"
.global fs_image_start
.global fs_image_end
fs_image_start:
fs_image_end:

/* Memory pool data section. 
 */
.section mem_info, "a"
.global mem_info_start
.global mem_info_end	
mem_info_start:
	.space 8192
mem_info_end:

