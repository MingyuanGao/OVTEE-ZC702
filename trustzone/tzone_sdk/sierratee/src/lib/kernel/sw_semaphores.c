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
 * semaphore implementation functions
 */

#include <sw_sem.h>
#include <global.h>
#include <sw_string_functions.h>

/** @ingroup OS_KernelApi Kernel API for Secure OS
 *  API for Sierra OS Kernel tasks
 *  @{
 */
/**
 * @brief 
 *
 * @param mutex
 * @param attribute
 *
 * @return 
 */
sw_mutex* _sw_mutex_init(char* mutex_name)
{
	struct link *temp;
	sw_mutex *mutex;
	sw_uint found = FALSE, cpsr;
	lock_irq_shared_resource(&global_val.mutex_list_lock, &cpsr);
	temp = global_val.mutex_list.next;
	while(temp != &global_val.mutex_list){
		mutex = temp->data;
		if(sw_strncmp(mutex->mutex_name, mutex_name,
							sw_strnlen(mutex_name,MAX_NAME_LEN)) == SW_OK){
			found = TRUE;
			break;
		}
		temp = temp->next;
	}
	if(!found){
		mutex = sw_malloc_private(COMMON_HEAP_ID, sizeof(sw_mutex));
		if(!mutex){
			sw_seterrno(SW_ENOMEM);
			return (sw_mutex *)SW_ERROR;
		}
		link_init(&mutex->head);
		set_link_data(&mutex->head, mutex);
		add_link(&global_val.mutex_list, &mutex->head, TAIL);
		sw_strncpy(mutex->mutex_name, mutex_name, MAX_NAME_LEN);
		mutex->lock_param = OTZ_FREE;
		mutex->init_param = TRUE;
		mutex->owner_id = 0;
		mutex->wait_queue.elements_count = 0;
		link_init(&mutex->wait_queue.elements_list);
		mutex->wait_queue.lock_shared.lock = 0;
	}
	unlock_irq_shared_resource(&global_val.mutex_list_lock, cpsr);
	return mutex;
}

/**
 * @brief 
 *
 * @param mutex
 *
 * @return 
 */
sw_int _sw_mutex_destroy(sw_mutex *mutex)
{
	sw_uint cpsr;
	if(mutex == NULL) {
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}
	if(mutex->lock_param != OTZ_FREE) {
		sw_seterrno(SW_EBUSY);
		return SW_ERROR;
	}
	mutex->init_param = FALSE;
	mutex->owner_id = 0;
	lock_irq_shared_resource(&global_val.mutex_list_lock, &cpsr);
	remove_link(&mutex->head);
	unlock_irq_shared_resource(&global_val.mutex_list_lock, cpsr);
	sw_free_private(COMMON_HEAP_ID, mutex);
	return SW_OK;
}

/**
 * @brief 
 *
 * @param mutex
 *
 * @return 
 */
sw_int _sw_acquire_mutex(sw_mutex *mutex)
{
	if((mutex == NULL) || (mutex->init_param == FALSE)) {
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}
	if(mutex->owner_id == get_current_task_id()){
		sw_seterrno(SW_EPERM);
		return SW_ERROR;
	}
	while(cpu_acquire_mutex((void*)(&mutex->lock_param)) != OTZ_LOCKED){
		sw_wait_event(&mutex->wait_queue, &mutex->lock_param);
	}
	mutex->owner_id = get_current_task_id();
	return SW_OK;
}

/**
 * @brief 
 *
 * @param mutex
 *
 * @return 
 */
sw_int _sw_release_mutex(sw_mutex *mutex)
{
	if((mutex == NULL) || (mutex->init_param == FALSE) || (mutex->owner_id 
				!= get_current_task_id())) {
		sw_seterrno(SW_EINVAL);
		return SW_OK;
	}
	cpu_release_mutex((void*)(&mutex->lock_param));	
	sw_wakeup(&mutex->wait_queue, WAKE_UP_IMMEDIATE);
	mutex->owner_id = 0;
	return SW_OK;
}

/**
 * @brief 
 *
 * @param mutex
 *
 * @return 
 */
sw_int _sw_acquire_mutex_nowait(sw_mutex *mutex)
{
	if((mutex == NULL) || (mutex->init_param == FALSE)) {
		sw_seterrno(SW_EINVAL);
		return SW_OK;
	}
	if(cpu_acquire_mutex_nowait((void*)&(mutex->lock_param)) != SW_OK) {
		sw_seterrno(SW_EBUSY);
		return SW_ERROR;
	}
	mutex->owner_id = get_current_task_id();
	return SW_OK;
}

/**
 * @brief 
 *
 * @param sem
 * @param shared
 * @param value
 *
 * @return 
 */
sw_semaphore* _sw_semaphore_init(char *sem_name, sw_uint value)
{
	struct link *temp;
	sw_semaphore *sem;
	sw_uint found = FALSE, cpsr;
	lock_irq_shared_resource(&global_val.semaphore_list_lock, &cpsr);
	temp = global_val.semaphore_list.next;
	while(temp != &global_val.semaphore_list){
		sem = temp->data;
		if(sw_strncmp(sem->sem_name, sem_name, sw_strnlen(sem_name,MAX_NAME_LEN)) == SW_OK){
			found = TRUE;
			break;
		}
		temp = temp->next;
	}
	if(!found){
		sem = sw_malloc_private(COMMON_HEAP_ID, sizeof(sw_semaphore));
		if(!sem){
			sw_seterrno(SW_ENOMEM);
			return (sw_semaphore *)SW_ERROR;
		}
		link_init(&sem->head);
		set_link_data(&sem->head, sem);
		add_link(&global_val.semaphore_list, &sem->head, TAIL);
		sw_strncpy(sem->sem_name, sem_name, MAX_NAME_LEN);
		sem->init_param = TRUE;
		sem->count = value;
		sem->counter_param = value;
		sem->condition = (sem->counter_param == 0);
		sem->wait_queue.elements_count = 0;
		link_init(&sem->wait_queue.elements_list);
		sem->wait_queue.lock_shared.lock = 0;
	}
	unlock_irq_shared_resource(&global_val.semaphore_list_lock, cpsr);
	return sem;
}

/**
 * @brief 
 *
 * @param sem
 *
 * @return 
 */
sw_int _sw_acquire_semaphore(sw_semaphore *sem)
{
	if((sem == NULL) || (sem->init_param == FALSE)) {
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}
	while(cpu_acquire_semaphore((sw_int*)&(sem->counter_param)) != OTZ_ACQUIRED){
		sw_wait_event(&sem->wait_queue, &sem->counter_param);
	}
	sem->condition = (sem->counter_param == 0);
	sw_wakeup(&sem->wait_queue, WAKE_UP);
	return SW_OK;
}

/**
 * @brief 
 *
 * @param sem
 *
 * @return 
 */
sw_int _sw_release_semaphore(sw_semaphore *sem)
{
	if((sem == NULL) || (sem->init_param == FALSE)) {
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}
	while(cpu_release_semaphore((void*)&sem->counter_param, (void*)&sem->count)
			!= OTZ_RELEASED){
		sem->condition = (sem->counter_param == 0);
		sw_wait_event(&sem->wait_queue, &sem->condition);
	}
	sw_wakeup(&sem->wait_queue, WAKE_UP);
	return SW_OK;
}

/**
 * @brief 
 *
 * @param sem
 *
 * @return 
 */
sw_int _sw_delete_semaphore(sw_semaphore *sem)
{
	sw_uint cpsr;
	if(sem == NULL) {
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}
	if(sem->counter_param < 0) {
		sw_seterrno(SW_EBUSY);
		return SW_ERROR;
	}
	sem->counter_param = 0;
	sem->init_param = FALSE;
	lock_irq_shared_resource(&global_val.semaphore_list_lock, &cpsr);
	remove_link(&sem->head);
	unlock_irq_shared_resource(&global_val.semaphore_list_lock, cpsr);
	sw_free_private(COMMON_HEAP_ID, sem);
	return SW_OK;
}

/**
 * @brief 
 *
 * @param sem
 * @param value
 *
 * @return 
 */
sw_int _sw_get_semaphore_count(sw_semaphore *sem, sw_int *value)
{
	if((sem == NULL) || (sem->init_param == NULL) || (value == NULL)) {
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
	}
	*value = sem->counter_param;
	return SW_OK;
}
/** @} */ // end of OS_KernelApi
