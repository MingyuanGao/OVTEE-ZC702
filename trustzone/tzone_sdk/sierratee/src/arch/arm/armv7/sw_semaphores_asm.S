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
 * Assembly routines for Semaphore operations
 */

.global cpu_acquire_mutex
.func cpu_acquire_mutex
@	void cpu_acuqire_mutex(void *lock_val);
cpu_acquire_mutex:
	/* Set Lock = 1 */
	mov	r2, #1
	/* get lock state */
	ldrex	r1, [r0]
	/* Is it locked? */
	teq	r1, #0
	bne	wait_for_int
	/* Lock now */
	strexeq	r1, r2, [r0]
	
	/* Check locked state */
	teqeq	r1, #0
	/* start from first in case of failure */
	bne	cpu_acquire_mutex
	mov r0, #1
	bx	lr
wait_for_int:
	mov r0, #0
	bx	lr
.endfunc

.global cpu_release_mutex
.func cpu_release_mutex
@	void cpu_release_mutex(void *lock_val);
cpu_release_mutex:
	/* Set Lock = 0 */
	mov	r2, #0
	/* get lock state */
	ldrex	r1, [r0]
	/* Is it locked? */
	teq	r1, #1
	bne	not_locked
	/* Unlock now */
	strexeq	r1, r2, [r0]
	
	/* Check locked state */
	teqeq	r1, #1
	/* start from first in case of failure */
	bne	cpu_release_mutex
	mov r0, #0
	bx	lr
not_locked:
	mov r0, #1
	bx  lr
.endfunc

.global cpu_acquire_mutex_nowait
.func cpu_acquire_mutex_nowait
@	void cpu_acquire_mutex_nowait(void *lock_val);
cpu_acquire_mutex_nowait:
	/* Set Lock = 1 */
	mov	r2, #1
	/* get lock state */
	ldrex	r1, [r0]
	/* Is it locked? */
	teq	r1, #0
	/* Lock now */
	strexeq	r1, r2, [r0]
	mov r0, r1
	bx	lr
.endfunc
		

.global cpu_acquire_semaphore
.func cpu_acquire_semaphore
@	void cpu_acquire_semaphore(void *lock_val);
cpu_acquire_semaphore:
	/* Get semaphore counter */
	ldrex	r1, [r0]
	cmp	r1, #0
	beq	wait_for_event
	/* Decrement the counter */
	sub	r1, #1
	strex	r2, r1, [r0]
	cmp	r2, #0
	bne	cpu_acquire_semaphore
	dmb
	mov r0, #1
	bx	lr
wait_for_event:
	mov r0, #0
	bx  lr
.endfunc

.global cpu_release_semaphore
.func cpu_release_semaphore
@	void cpu_release_semaphore(void *lock_val);
cpu_release_semaphore:
	/* Get semaphore counter */
	ldrex	r2, [r0]
	ldr 	r3, [r1]
	cmp     r2, r3
	beq     invalid_release
	add     r2, #1
	strex   r3, r2, [r0]
	cmp     r3, #0
	bne     cpu_release_semaphore
	dmb
	mov     r0, #0
	bx      lr
invalid_release:
	mov r0, #1
	bx	lr
.endfunc
