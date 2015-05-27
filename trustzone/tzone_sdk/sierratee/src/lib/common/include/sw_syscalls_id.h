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
 * Header for Syscall Numbers
 */

#ifndef __SW_SYSCALL_H_
#define __SW_SYSCALL_H_

/* 
 * Syscall Numbers 
 */
#define SW_SYSCALL_RENAME       	0xffe0
#define SW_SYSCALL_GET_POSITION 	0xffe1
#define SW_SYSCALL_REMOVE       	0xffe2
#define SW_SYSCALL_LSEEK 			0xffe3
#define SW_SYSCALL_FSTAT        	0xffe4
#define SW_SYSCALL_TRUNCATE     	0xffe5

#define SW_SYSCALL_EXIT				0xffe6
#define SW_SYSCALL_ABORT			0xffe7
#define SW_SYSCALL_KILL				0xffe8
#define SW_SYSCALL_WAIT				0xffe9
#define SW_SYSCALL_GETTIMEOFDAY		0xffea
#define SW_SYSCALL_CLOCK_GETTIME	0xffeb

#define SW_SYSCALL_OPEN         	0xfff0
#define SW_SYSCALL_CLOSE        	0xfff1
#define SW_SYSCALL_MAP_NS       	0xfff2
#define SW_SYSCALL_UNMAP_NS     	0xfff3
#define SW_SYSCALL_EXE_SMC      	0xfff4
#define SW_SYSCALL_READ         	0xfff5
#define SW_SYSCALL_WRITE        	0xfff6
#define SW_SYSCALL_IOCTL        	0xfff7
#define SW_SYSCALL_USLEEP       	0xfff8

#define SW_SYSCALL_SCHEDULE		0xbb

#define SW_SYSCALL_SEM_INIT		0xffc0
#define SW_SYSCALL_SEM_ACQUIRE	0xffc1
#define SW_SYSCALL_SEM_RELEASE	0xffc2
#define SW_SYSCALL_SEM_DELETE	0xffc3
#define SW_SYSCALL_MUTEX_INIT		0xffc4
#define SW_SYSCALL_MUTEX_ACQUIRE	0xffc5
#define SW_SYSCALL_MUTEX_RELEASE	0xffc6
#define SW_SYSCALL_MUTEX_DELETE	0xffc7
#define SW_SYSCALL_MUTEX_ACQUIRE_NOWAIT	0xffc8
#define SW_SYSCALL_SHM_CREATE	0xffc9
#define SW_SYSCALL_SHM_CONTROL	0xffca
#define SW_SYSCALL_SHM_ATTACH	0xffcb
#define SW_SYSCALL_SHM_DETACH	0xffcc

#ifdef NEWLIB_SUPPORT
#define SW_SYSCALL_SBRK			0xfffc
#define SW_SYSCALL_SBRK_UPDATE	0xfffd
#endif

/* Private Syscall numbers */
#define SW_SYSCALL_START_GUEST 		0Xfa00

#define SW_SYSCALL_ALLOC_USR_HEAP 	0Xfa01

#define SW_SYSCALL_GET_TLS 			0Xfa03
#define SW_SYSCALL_SET_TLS 			0Xfa04

#endif
