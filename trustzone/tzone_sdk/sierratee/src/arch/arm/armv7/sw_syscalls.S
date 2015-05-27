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
 * sw_syscalls implementation
 */
#include <sw_syscalls_id.h>

.global __sw_open
.func __sw_open
__sw_open:
	push {lr}
	swi SW_SYSCALL_OPEN
	pop {lr}
	bx  lr
.endfunc

.global __sw_close
.func __sw_close
__sw_close:
	push {lr}
	swi SW_SYSCALL_CLOSE
	pop {lr}
	bx lr
.endfunc

.global __sw_read
.func __sw_read
__sw_read:
	push {lr}
	swi SW_SYSCALL_READ
	pop {lr}
	bx lr
.endfunc

.global __sw_write
.func __sw_write
__sw_write:
	push {lr}
	swi SW_SYSCALL_WRITE
	pop {lr}
	bx lr
.endfunc

.global __sw_ioctl
.func __sw_ioctl
__sw_ioctl:
	push {lr}
	swi SW_SYSCALL_IOCTL
	pop {lr}
	bx lr
.endfunc

.global __sw_file_getposition
.func __sw_file_getposition
__sw_file_getposition:
	push {lr}
	swi SW_SYSCALL_GET_POSITION
	pop {lr}
	bx lr
.endfunc

.global __sw_truncate
.func __sw_truncate
__sw_truncate:
	push {lr}
	swi SW_SYSCALL_TRUNCATE
	pop {lr}
	bx lr
.endfunc

.global __sw_rename
.func __sw_rename
__sw_rename:
	push {lr}
	swi SW_SYSCALL_RENAME
	pop {lr}
	bx lr
.endfunc

.global __sw_lseek
.func __sw_lseek
__sw_lseek:
	push {lr}
	swi SW_SYSCALL_LSEEK
	pop {lr}
	bx lr
.endfunc
.global __sw_remove
.func __sw_remove
__sw_remove:
	push {lr}
	swi SW_SYSCALL_REMOVE
	pop {lr}
	bx lr
.endfunc
.global __sw_fstat
.func __sw_fstat
__sw_fstat:
	push {lr}
	swi SW_SYSCALL_FSTAT
	pop {lr}
	bx lr
.endfunc

.global __asm_map_to_ns
.func __asm_map_to_ns
__asm_map_to_ns:
	push {lr}
	swi SW_SYSCALL_MAP_NS
	pop {lr}
	bx lr
.endfunc

.global __asm_unmap_from_ns
.func __asm_unmap_from_ns
__asm_unmap_from_ns:
	push {lr}
	swi SW_SYSCALL_UNMAP_NS
	pop {lr}
	bx lr
.endfunc

.global __asm_execute_smc
.func __asm_execute_smc
__asm_execute_smc :
	push {lr}
	swi SW_SYSCALL_EXE_SMC
	pop {lr}
	bx lr
.endfunc

.global __sw_usleep
.func __sw_usleep
__sw_usleep :
	push {lr}
	swi SW_SYSCALL_USLEEP
	pop {lr}
	bx lr
.endfunc

#ifdef CONFIG_SHELL
.global __sw_start_guest
.func __sw_start_guest
__sw_start_guest:
	push {lr}
	swi SW_SYSCALL_START_GUEST
	pop {lr}
	bx lr
.endfunc
#endif

.global __sw_get_tls
.func __sw_get_tls
__sw_get_tls:
	push {lr}
	swi SW_SYSCALL_GET_TLS
	pop {lr}
	bx lr
.endfunc

.global __sw_set_tls
.func __sw_set_tls
__sw_set_tls:
	push {lr}
	swi SW_SYSCALL_SET_TLS
	pop {lr}
	bx lr
.endfunc


.global __alloc_user_heap
.func __alloc_user_heap
__alloc_user_heap:
	push {lr}
	swi SW_SYSCALL_ALLOC_USR_HEAP
	pop {lr}
	bx lr
.endfunc

.global __sw_exit
.func __sw_exit
__sw_exit:
	push {lr}
	swi SW_SYSCALL_EXIT
	pop {lr}
	bx lr
.endfunc

.global __sw_abort
.func __sw_abort
__sw_abort:
	push {lr}
	swi SW_SYSCALL_ABORT
	pop {lr}
	bx lr
.endfunc

.global __sw_kill
.func __sw_kill
__sw_kill:
	push {lr}
	swi SW_SYSCALL_KILL
	pop {lr}
	bx lr
.endfunc

.global __sw_wait
.func __sw_wait
__sw_wait:
	push {lr}
	swi SW_SYSCALL_WAIT
	bl schedule
	pop {lr}
	bx lr
.endfunc

.global __sw_gettimeofday
.func __sw_gettimeofday
__sw_gettimeofday:
	push {lr}
	swi SW_SYSCALL_GETTIMEOFDAY
	pop {lr}
	bx lr
.endfunc

.global sw_semaphore_init
.func sw_semaphore_init
sw_semaphore_init:
	push {lr}
	swi  SW_SYSCALL_SEM_INIT
	pop  {lr}
	bx   lr
.endfunc
.global sw_acquire_semaphore
.func sw_acquire_semaphore
sw_acquire_semaphore:
	push {lr}
	swi  SW_SYSCALL_SEM_ACQUIRE
	pop  {lr}
	bx   lr
.endfunc
.global sw_release_semaphore
.func sw_release_semaphore
sw_release_semaphore:
	push {lr}
	swi  SW_SYSCALL_SEM_RELEASE
	pop  {lr}
	bx   lr
.endfunc
.global sw_delete_semaphore
.func sw_delete_semaphore
sw_delete_semaphore:
	push {lr}
	swi  SW_SYSCALL_SEM_DELETE
	pop  {lr}
	bx   lr
.endfunc
.global sw_mutex_init
.func sw_mutex_init
sw_mutex_init:
	push {lr}
	swi  SW_SYSCALL_MUTEX_INIT
	pop  {lr}
	bx   lr
.endfunc
.global sw_acquire_mutex
.func sw_acquire_mutex
sw_acquire_mutex:
	push {lr}
	swi  SW_SYSCALL_MUTEX_ACQUIRE
	pop  {lr}
	bx   lr
.endfunc
.global sw_acquire_mutex_nowait
.func sw_acquire_mutex_nowait
sw_acquire_mutex_nowait:
	push {lr}
	swi  SW_SYSCALL_MUTEX_ACQUIRE_NOWAIT
	pop  {lr}
	bx   lr
.endfunc
.global sw_release_mutex
.func sw_release_mutex
sw_release_mutex:
	push {lr}
	swi  SW_SYSCALL_MUTEX_RELEASE
	pop  {lr}
	bx   lr
.endfunc
.global sw_mutex_destroy
.func sw_mutex_destroy
sw_mutex_destroy:
	push {lr}
	swi  SW_SYSCALL_MUTEX_DELETE
	pop  {lr}
	bx   lr
.endfunc
.global shm_create
.func shm_create
shm_create:
	push {lr}
	swi  SW_SYSCALL_SHM_CREATE
	pop  {lr}
	bx   lr
.endfunc
.global shm_control
.func shm_control
shm_control:
	push {lr}
	swi  SW_SYSCALL_SHM_CONTROL
	pop  {lr}
	bx   lr
.endfunc
.global shm_attach
.func shm_attach
shm_attach:
	push {lr}
	swi  SW_SYSCALL_SHM_ATTACH
	pop  {lr}
	bx   lr
.endfunc
.global shm_detach
.func shm_detach
shm_detach:
	push {lr}
	swi  SW_SYSCALL_SHM_DETACH
	pop  {lr}
	bx   lr
.endfunc

.global __sw_clock_gettime
.func __sw_clock_gettime
__sw_clock_gettime:
	push {lr}
	swi SW_SYSCALL_CLOCK_GETTIME
	pop {lr}
	bx lr
.endfunc

#ifdef NEWLIB_SUPPORT
.global __sw_sbrk
.func __sw_sbrk
__sw_sbrk:
	push {lr}
	swi SW_SYSCALL_SBRK
	pop {lr}
	bx lr
.endfunc

.global __sw_sbrk_update
.func __sw_sbrk_update
__sw_sbrk_update:
	push {lr}
	swi SW_SYSCALL_SBRK_UPDATE
	pop {lr}
	bx lr
.endfunc
#endif
