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
 * Header for sw_syscalls implementation
 */

#ifndef __SW_SYSCALL_H_
#define __SW_SYSCALL_H_

#include <tls.h>

/**
* @brief Architecture specific open system call entry point
*
* @param name: Name of the device/file
* @param flags: File open/creation flags 
* @param mode: File access modes
*
* @return 
*/
int __sw_open(char* name, sw_int flags, sw_int mode);

/**
* @brief 
*  Architecture specific close system call entry point
*
* @param fd: Descriptor for close system call
*
* @return 
*/
int __sw_close(sw_int fd);

/**
* @brief 
*  Architecture specific read system call entry point
*
* @param fd:  Previously opend valid file desciptor
* @param buf: Buffer for read operation
* @param count: Buffer length for read operation
*
* @return 
 */
int __sw_read(sw_int fd, void* buf, sw_int count);

/**
 * @brief 
 *  Architecture specific write system call entry point
 *
 * @param fd:  Previously opend valid file desciptor
 * @param buf: Buffer for write operation
 * @param count: Buffer length for write operation
 *
 * @return 
 */
int __sw_write(sw_int fd, void* buf, sw_int count);

/**
 * @brief 
 *  Architecture specific ioctl system call entry point
 *
 * @param fd:  Previously opend valid file desciptor
 * @param ioctl_id: Identifier for ioctl command
 * @param req: Request buffer for ioctl command
 * @param res: Response buffer for ioctl command
 *
 * @return 
 */
int __sw_ioctl(sw_int fd, sw_uint ioctl_id, void* req, void* res);

/**
 * @brief
 * lseek system call sets the position indicator associated with the file 
 * to a new position 
 *
 * @param file -   The file descriptor of the file
 * @param offset - Number of bytes to offset from origin
 * @param flags - Position used as reference for the offset value
 *
 * @return - If successful, the function returns zero.
 *           Otherwise, it returns non-zero value
 */

long __sw_lseek(int file, long offset, int flags);

/**
 * @brief 
 * System call to remove the file whose name is specified in filename
 *
 * @param rm_p - string containing the name of the file to be deleted
 *
 * @return - if the file is successfully deleted, a zero value is returned.
 *           On failure, a non-zero value is returned
 */
int __sw_remove(const char *rm_p);

/**
 * @brief 
 *      Execute smc system call
 */
void __asm_execute_smc(void);

/**
 * @brief 
 *     system call to  map a address as non-secure entry
 * @param phy_addr
 * @param va_addr
 *
 * @return 
 */
int __asm_map_to_ns(sw_phy_addr phy_addr, sw_vir_addr *va_addr);

/**
 * @brief 
 *     system call to Unmap a non-secure address 
 *
 * @param va_addr
 *
 * @return 
 */
int __asm_unmap_from_ns(sw_vir_addr va_addr);

/**
 * @brief 
 *     system call to usleep
 *
 * @param va_addr
 *
 * @return 
 */
int __sw_usleep(int seconds);

#ifdef CONFIG_SHELL
/**
 * @brief 
 *     system call to start non secure guest
 *
 * @return 
 */

void  __sw_start_guest();
#endif

/**
 * @brief System call to create the user heap
 *
 * @param task_id: Task ID
 * @param size: Size of the heap
 * @param min_alloc_size: Minimum Size of the heap
 *
 * @return :
 * SW_OK - User heap allocated successfully.\n
 * SW_* - Implementation defined error.\n
 */
int __alloc_user_heap(sw_uint task_id, sw_uint size,sw_uint min_alloc_size);

/**
 * @brief System call to return the TLS data
 *
 * @return Valid TLS pointer or NULL
 */
void *__sw_get_tls(void);

/**
 * @brief System call to set the TLS data
 *
 * @param tls: Pointer to TLS
 *
 * @return 
 * SW_OK - TLS data updated successfully.\n
 * SW_* - Implementation defined error code.\n
 */
int __sw_set_tls(void *tls);


/**
* @brief System call to exit the user task
*
* @param task_id: Task Exit status
*/
void exit_usr_task(int exit_status);

#ifdef NEWLIB_SUPPORT
/**
 * @brief 
 *     sbrk system call
 */
void __sw_sbrk(sw_uint *heap_start, sw_uint* heap_size, 
				sw_uint* prev_heap_end);

/**
 * @brief 
 *     sbrk_update system call to update the current heap pos
 */
void __sw_sbrk_update(sw_uint current_libc_heap);
#endif

/**
* @brief System call to unmap non-secure memory from secure page table.
*
* @param va_addr: Virtual address need to be unmapped
*
* @return 
* SW_OK - Unmapping is done successfully. \n
* SW_* - Implementation defined error.\n
*/
int unmap_from_ns(sw_vir_addr va_addr);

/**
* @brief System call to map non-secure memory in secure page table.
*
* @param va_addr: Physical address need to be mapped
* @param va_addr: Virtual address need to be mapped
*
* @return 
* SW_OK - Mapping is done successfully. \n
* SW_* - Implementation defined error.\n
*/
int map_to_ns(sw_phy_addr phy_addr, sw_vir_addr *va_addr);

/**
 * @brief Invoke scheduler
 *
 * This function invokes the scheduler to schedule the next ready task
 */
void schedule(void);
#endif
