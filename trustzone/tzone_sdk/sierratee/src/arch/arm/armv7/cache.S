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
 *	ARM Cache operations implementation
 */
#include <sw_arm_defines.h>
#include <sw_platform_asm.h>

.global enable_l1_cache
.func enable_l1_cache
enable_l1_cache:
	@ void enable_l1_cache(void)
	push 	{r0-r12, lr}
	mrc	p15, 0, r0, c1, c0, 0
	orr 	r1, r1, #SCTLR_ICACHE_BIT
	orr 	r1, r1, #SCTLR_DCACHE_BIT
	mcr 	p15, 0, r1, c1, c0, 0
	isb
	pop	{r0-r12, lr}
	bx 	lr
.endfunc

.global icache_inv_all
.func icache_inv_all
	@ void icache_inv_all(void);
icache_inv_all:
	@ void icache_inv_all(void)
	push 	{r0-r12, lr}
	mov	r0, #0
	mcr 	p15, 0, r0, c7, c5, 0
	isb
	pop	{r0-r12, lr}
	bx	lr
.endfunc

/* 	r0 - Cache level
	r1 - Log2 Line Length
	r2 - Way
	r3 - LEADING ZEROS of WAY
	r4 - Set
	r5 - clean or invalidate
*/
data_cache_clean_invalidate_set_way:
	mov	r9, r2
2:
	/* 
		val = (cache level << 1) | (set << log2 LINE_LEN) |  (way << LEADING_ZEROS_OF WAY)
	*/
	orr	r11, r0, r9, lsl r3
	orr	r11, r11, r4, lsl r1
	cmp	r5, #1
	mcreq 	p15, 0, r11, c7, c6, 2
	mcrne 	p15, 0, r11, c7, c14, 2
	subs	r9, r9, #1
	bge 	2b
	subs	r4, r4, #1
	bge 	data_cache_clean_invalidate_set_way
	bx	lr	

.global data_cache_clean_invalidate_all
.func data_cache_clean_invalidate_all
	/* Need to configure the Cache Level, Sets and Way parameters for 
		this function to work */
	@ void data_cache_clean_invalidate_all(int invalid_only);
data_cache_clean_invalidate_all:
	push	{r0-r12, lr}
	dmb
	mov	r5, r0
	ldr	r0, =CACHE_LEVEL1_SET
	cmp	r0, #1
	bne	1f
	/* Level 1 */
	mov	r0, #0
	mcr	p15, 2, r0, c0, c0, 0
	isb
	ldr	r1, =MAX_L1_LINE_LEN
	ldr	r2, =MAX_L1_CACHE_WAYS
	clz	r3, r2	
	ldr	r4, =MAX_L1_SETS
	bl	data_cache_clean_invalidate_set_way
	/* Level 2 */
	ldr	r0, =CACHE_LEVEL2_SET
	cmp	r0, #1
	bne	1f
	mov	r0, #2 /* Shift Cache level Id by 1 bit */
	mcr	p15, 2, r0, c0, c0, 0
	isb
	ldr	r1, =MAX_L2_LINE_LEN
	ldr	r2, =MAX_L2_CACHE_WAYS
	clz	r3, r2	
	ldr	r4, =MAX_L2_SETS
	bl	data_cache_clean_invalidate_set_way
1:	
	mov 	r0, #0
	mcr 	p15, 2, r10, c0, c0, 0
	dsb
	isb
	pop	{r0-r12, lr}
	bx	lr
.endfunc


.global flush_all_translation_table_entries
.func flush_all_translation_table_entries
	@ void flush_all_translation_table_entries(void);
flush_all_translation_table_entries:
	mcr	 p15, 0, r0, c8, c3,  0
	mcr	 p15, 0, r0, c7, c5,  6  @ Inval. branch predict. array
	dsb
	isb
	mov pc, lr	
.endfunc

.global inv_translation_table_by_va_asid
.func inv_translation_table_by_va_asid
	@ void inv_translation_table_by_va_asid(sw_uint va);
inv_translation_table_by_va_asid:
	mcr	 p15, 0, r0, c8, c3,  1
	mcr	 p15, 0, r0, c7, c5,  6  @ Inval. branch predict. array
	dsb
	isb
	mov pc, lr	
.endfunc

.global inv_translation_table_by_asid
.func inv_translation_table_by_asid
	@ void inv_translation_table_by_asid(sw_uint asid);
inv_translation_table_by_asid:
	mcr	 p15, 0, r0, c8, c3,  2
	mcr	 p15, 0, r0, c7, c5,  6  @ Inval. branch predict. array
	dsb
	isb
	mov pc, lr	
.endfunc


.global flush_icache_and_dcache
.func flush_icache_and_dcache
	@ void flush_icache_and_dcache(void);
flush_icache_and_dcache:
	push	{r0-r12, lr}
	mov 	r0, #0
	bl  	data_cache_clean_invalidate_all
	mov 	r0, #0
	/* Invalidate instruction cache */
	mcr 	p15, 0, r0, c7, c5, 0	   
	dsb
	isb
	pop	{r0-r12, lr}
	mov pc, lr
.endfunc
