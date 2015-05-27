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
 * System calls  implementation
 */
#include <sw_types.h> 
#include <sw_debug.h> 
#include <sw_modinit.h>
#include <sw_device_id.h>

#include <sw_syscalls_id.h>

#include <sw_timer_functions.h>

#include <task.h>
#include <sw_string_functions.h>

#include <sw_buddy.h>
#include <secure_api.h>
#include <global.h>
#include <sw_user_mgmt.h>
#include <monitor.h>
#include <tzhyp_global.h>
#include <sw_mem_functions.h>
#include <sw_heap.h>
#include <otz_id.h>
#include <sw_sem.h>
#include <sw_shm.h>

#include <board_config.h>

/** @defgroup syscalls Kernel System calls
 *  POSIX Syscall implementation.
 *  @{
 */

/**
* @brief Close device/file using descriptor data 
*
* @param fd_data: Device/File descriptor data
*
* @return 
*/
sw_int  close_device_file(struct fd_link *fd_data) 
{
	sw_int ret = -1;
	if(fd_data->dev_info) {
		if(fd_data->dev_info->close) {
			ret = fd_data->dev_info->close(fd_data->fd);
		}
		else {
			ret = -1;
			sw_seterrno(SW_EPERM);
		} 
	}
	return ret;		
}

/**
* @brief 
*
* @param fd
*
* @return 
*/
static struct fd_link * find_fd(sw_int fd)
{
	struct sw_task* task;
	struct link *l, *head;
	struct fd_link *fd_data;
	sw_uint found = FALSE;
	
	task = get_current_task();
	if(!task) 
		head= &global_val.file_dev_list;
	else
		head= &task->file_dev_list;
	
	l = head->next;
	while (l != head) {
		fd_data = l->data;
		if(fd_data) {
			if(fd_data->fd == fd) {
				found = TRUE;
				break;
			}
		}
		l = l->next;
	}
	
	if(found) 
		return fd_data;
	
	return NULL;
}

/**
* @brief 
*
* @param head
* @param current
* @param next
*
* @return 
*/
static struct sw_file_operations * get_next_file_dev(
				struct link *head, 
				struct link *current, 
				struct link **next)
{
	if(current != head) {
		*next = current->next;
		return current->data;
	}
	*next = NULL;
	return NULL;
}

/**
* @brief 
*
* @param name
*
* @return 
*/
struct sw_file_operations * find_device(char* name) 
{
	struct sw_file_operations *dev_info;
	struct link *current, *next;
	current = global_val.sw_dev_head.dev_list.next;
	do {
		dev_info = get_next_file_dev(&global_val.sw_dev_head.dev_list, 
						current, &next);
		if(!next)
			return NULL;
		if(!dev_info)	
			return NULL;

		if(sw_strncmp(dev_info->sw_dev_name,name,20) == 0)
			return dev_info;
		current = next;
	} while(current);
	return NULL;
}

/**
* @brief System call implementation handler
*
* @param swi_id: System call identifier
* @param regs: Context registers
*/
void invoke_syscall_handler(sw_int swi_id, struct swi_temp_regs *regs)
{
	sw_int ret=0, fd = -1;
	struct sw_file_operations *dev_info;
	struct fd_link *fd_data;
	struct sw_task *task, *tmp_task;
	sw_timeval *tmp_tv = NULL;
	switch (swi_id) {
		case SW_SYSCALL_OPEN:
			dev_info = find_device((char*)regs->regs[0]);
			if(dev_info != NULL) {
				if(get_permission(dev_info->dev_id) != 1){
					sw_printk("\n\rSW: No Permission to access %s device\n", 
							dev_info->sw_dev_name);
					fd = -1;
					sw_seterrno(SW_EPERM);
				}
				else if(dev_info->open != NULL)
					if(dev_info->dev_id == IPC_DEVICE_ID) {
						fd = dev_info->open(regs->regs[1], regs->regs[2]);
					}
					else {
						fd = dev_info->open(regs->regs[0], regs->regs[1], regs->regs[2]);
					}
				else {
					fd = -1;
					sw_seterrno(SW_EPERM);
				} 
				
			}
			if(dev_info && dev_info->dev_id == IPC_DEVICE_ID)
				regs->regs[0] = SW_OK;	
			else 
				regs->regs[0] = fd;
			if(fd != -1) {
				task = get_current_task();
				fd_data = (struct fd_link * )sw_malloc_private(
												COMMON_HEAP_ID, 
												sizeof(struct fd_link));
				if(fd_data) {
					fd_data->fd = fd;
					fd_data->dev_info = dev_info;
					set_link_data(&fd_data->head, fd_data);
					if(task)				
						add_link(&task->file_dev_list, &fd_data->head, TAIL);
					else 
						add_link(&global_val.file_dev_list, &fd_data->head, TAIL);
				}
				else { 
					regs->regs[0] = -1;
					sw_seterrno(SW_ENOMEM);

				}
			}
			else {
				regs->regs[0] = -1;
				sw_seterrno(SW_ENOENT);
			}
			break;
		case SW_SYSCALL_CLOSE:
			fd_data = find_fd(regs->regs[0]);
			if(fd_data) {			
				if(fd_data->dev_info) {
					if(get_permission(fd_data->dev_info->dev_id) != 1){
						sw_printk("\n\rSW: No Permission to access %s device\n", 
							fd_data->dev_info->sw_dev_name);
						regs->regs[0] = -1;
						sw_seterrno(SW_EPERM);
					}
					else if(fd_data->dev_info->close) {
						regs->regs[0] = fd_data->dev_info->close(fd_data->fd);
					}
					else {
						regs->regs[0] = -1;
						sw_seterrno(SW_EPERM);
					} 
				}
			}
			else {
				regs->regs[0] = -1;
				sw_seterrno(SW_ENOENT);
			}
			if(regs->regs[0] != -1) {
				remove_link(&fd_data->head);
				sw_free_private(COMMON_HEAP_ID, fd_data);
			}
			break;
		case SW_SYSCALL_WRITE:
			fd_data = find_fd(regs->regs[0]);
			if(fd_data) {
				if(fd_data->dev_info) {
					if(get_permission(fd_data->dev_info->dev_id) != 1){
						sw_printk("\n\rSW: No Permission to access %s device\n", 
							fd_data->dev_info->sw_dev_name);
						regs->regs[0] = -1;
						sw_seterrno(SW_EPERM);
					}
					else if(fd_data->dev_info->write)
						regs->regs[0] = fd_data->dev_info->write(regs->regs[0], 
									regs->regs[1], regs->regs[2]);
					else {
						regs->regs[0] = -1;
						sw_seterrno(SW_EPERM);
					} 
				}
			}
			else {
				regs->regs[0] = -1;
				sw_seterrno(SW_ENOENT);
			}

			break;

		case SW_SYSCALL_READ:
			fd_data = find_fd(regs->regs[0]);
			if(fd_data) {
				if(fd_data->dev_info) {
					if(get_permission(fd_data->dev_info->dev_id) != 1){
						sw_printk("\n\rSW: No Permission to access %s device\n", 
								fd_data->dev_info->sw_dev_name);
						regs->regs[0] = -1;
						sw_seterrno(SW_EPERM);
					}
					else if(fd_data->dev_info->read)
						regs->regs[0] = fd_data->dev_info->read(regs->regs[0], 
											regs->regs[1], 
											regs->regs[2]);
					else {
						regs->regs[0] = -1;
						sw_seterrno(SW_EPERM);
					} 
				}
			}
			else {
				regs->regs[0] = -1;
				sw_seterrno(SW_ENOENT);
			}

			break;

		case SW_SYSCALL_IOCTL:
			fd_data = find_fd(regs->regs[0]);
			if(fd_data) {
				if(fd_data->dev_info) {
					if(get_permission(fd_data->dev_info->dev_id) != 1){
						sw_printk("\n\rSW: No Permission to access %s device\n", 
							fd_data->dev_info->sw_dev_name);
						regs->regs[0] = -1;
						sw_seterrno(SW_EPERM);
					}
					else if(fd_data->dev_info->ioctl)
						fd_data->dev_info->ioctl(regs->regs[0], 
									regs->regs[1], regs->regs[2], regs->regs[3]);
				}
				else {
					regs->regs[0] = -1;
					sw_seterrno(SW_ENOENT);
				}
			}
			else {
					regs->regs[0] = -1;
					sw_seterrno(SW_ENOENT);
			}
			break;
		case SW_SYSCALL_EXE_SMC:
			__execute_smc(regs->regs[0]);
			break;
		case SW_SYSCALL_UNMAP_NS:
			ret = __unmap_from_ns(regs->regs[0]);
			regs->regs[0] = ret;
			break;
		case SW_SYSCALL_MAP_NS:
			ret = __map_to_ns(regs->regs[0],(sw_vir_addr*)regs->regs[1]);
			regs->regs[0] = ret;
			break;
		case SW_SYSCALL_USLEEP:
			sw_usleep((sw_int)regs->regs[0]);
			break;
#ifdef CONFIG_SHELL
		case SW_SYSCALL_START_GUEST:
			launch_current_guest();
			break;
#endif
		case SW_SYSCALL_ALLOC_USR_HEAP: 
			task = get_task(regs->regs[0]);
			if(task == NULL) {
				regs->regs[0] = SW_EINVAL;
				sw_seterrno(SW_EINVAL);
				break;
			}
			ret = alloc_user_heap(task->task_id, regs->regs[1], regs->regs[2],
													&global_val.sw_mem_info);
			regs->regs[0] = ret;
			break;
		case SW_SYSCALL_GET_TLS: 
			task = get_current_task();
			if(task == NULL) {
				regs->regs[0] = 0;
				sw_seterrno(SW_EINVAL);
				break;
			}
			regs->regs[0] = (sw_uint)task->tls;
			break;
		case SW_SYSCALL_SET_TLS:

			task = get_current_task();
			if(task == NULL) {
				regs->regs[0] = SW_EINVAL;
				sw_seterrno(SW_EINVAL);
				break;
			}
			sw_memcpy((void*)task->tls, (void*)regs->regs[0], sizeof(sw_tls));
			regs->regs[0] = SW_OK;
			break;
		case SW_SYSCALL_EXIT:
			task = get_current_task();
			task->tls->ret_val = (sw_uint)regs;
			handle_task_return(task->task_id,task->tls);
			break;
		case SW_SYSCALL_ABORT:
			task = get_current_task();
			task->tls->ret_val = SMC_ERROR;
			handle_task_return(task->task_id,task->tls);
			break;
		case SW_SYSCALL_KILL:
			tmp_task = get_current_task();
			task = get_task(regs->regs[0]);
			task->tls->ret_val = SMC_ERROR;
			if(task->acl.uid == tmp_task->acl.uid || task->acl.gid == tmp_task->acl.gid || task->acl.uid == SUPER_USER_ID) {
				handle_task_return(task->task_id,task->tls);
				break;
			}
			sw_printf("Cannot kill process %d\n",task->task_id);
			break;
		case SW_SYSCALL_WAIT:
			tmp_task = get_current_task();
			task = get_task(regs->regs[0]);
			add_to_wait_list(task, tmp_task);
			suspend_task(tmp_task->task_id, TASK_STATE_SUSPEND);
			break;
		case SW_SYSCALL_GETTIMEOFDAY:
			if(regs->regs[0] == NULL) {
				regs->regs[0] = -1;
				break;
				}
			tmp_tv = (sw_timeval *)regs->regs[0];
			tmp_tv->tval64 = read_timestamp() + global_val.epoch;
			regs->regs[0] = 0;
			break;
		case SW_SYSCALL_SEM_INIT:
			regs->regs[0] = (sw_uint)_sw_semaphore_init((char *)regs->regs[0], 
					(sw_uint)regs->regs[1]);
			break;
		case SW_SYSCALL_SEM_ACQUIRE:
			regs->regs[0] = _sw_acquire_semaphore((sw_semaphore *)regs->regs[0]);
			break;
		case SW_SYSCALL_SEM_RELEASE:
			regs->regs[0] = _sw_release_semaphore((sw_semaphore *)regs->regs[0]);
			break;
		case SW_SYSCALL_SEM_DELETE:
			regs->regs[0] = _sw_delete_semaphore((sw_semaphore *)regs->regs[0]);
			break;
		case SW_SYSCALL_MUTEX_INIT:
			regs->regs[0] = (sw_uint)_sw_mutex_init((char *)regs->regs[0]);
			break;
		case SW_SYSCALL_MUTEX_ACQUIRE:
			regs->regs[0] = _sw_acquire_mutex((sw_mutex *)regs->regs[0]);
			break;
		case SW_SYSCALL_MUTEX_ACQUIRE_NOWAIT:
			regs->regs[0] = _sw_acquire_mutex_nowait((sw_mutex *)regs->regs[0]);
			break;
		case SW_SYSCALL_MUTEX_RELEASE:
			regs->regs[0] = _sw_release_mutex((sw_mutex *)regs->regs[0]);
			break;
		case SW_SYSCALL_MUTEX_DELETE:
			regs->regs[0] = _sw_mutex_destroy((sw_mutex *)regs->regs[0]);
			break;
		case SW_SYSCALL_SHM_CREATE:
			regs->regs[0] = _shm_create((char *)regs->regs[0], regs->regs[1],
					regs->regs[2]);
			break;
		case SW_SYSCALL_SHM_CONTROL:
			regs->regs[0] = _shm_control(regs->regs[0], regs->regs[1],
					(sw_uint *)regs->regs[2]);
			break;
		case SW_SYSCALL_SHM_ATTACH:
			regs->regs[0] = (sw_uint)_shm_attach(regs->regs[0], 
					(sw_uint *)regs->regs[1], regs->regs[2]);
			break;
		case SW_SYSCALL_SHM_DETACH:
			regs->regs[0] = _shm_detach(regs->regs[0], (sw_uint *)regs->regs[1]);
			break;
		case SW_SYSCALL_CLOCK_GETTIME:
			if(regs->regs[0] == NULL || regs->regs[1] == NULL) {
				regs->regs[0] = -1;
				break;
				}
			if(regs->regs[0] == CLOCK_REALTIME) {
				tmp_tv = (sw_timeval *)regs->regs[1];
				tmp_tv->tval64 = read_timestamp() + global_val.epoch;
				regs->regs[0] = 0;
				}
			else
				{
				sw_printf("Clock not supported\n");
				regs->regs[0] = -1;
				}
			break;

#ifdef NEWLIB_SUPPORT
		case SW_SYSCALL_SBRK:
			sw_libc_sbrk((sw_uint *)regs->regs[0], (sw_uint *)regs->regs[1], 
						(sw_uint *)regs->regs[2]);
			break;

		case SW_SYSCALL_SBRK_UPDATE:
			sw_libc_sbrk_update(regs->regs[0]);
			break;
#endif
	}
}


