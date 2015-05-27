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
 * Header for semaphore implementation functions
 */

#ifndef SW_USER_SEMAPHORE_H
#define SW_USER_SEMAPHORE_H

#include <sw_types.h>

#define OTZ_ACQUIRED 1
#define OTZ_RELEASED 0

#define TRUE   1
#define FALSE  0

#define OTZ_OK 0
#define OTZ_INVALID 1
#define OTZ_BUSY 2
#define OTZ_AGAIN 3

/**
 * @brief 
 */
typedef struct {
} sw_mutex;

/**
 * @brief dymanic initialization of the mutex. The attribute parameter is kept
 * for consistency and is not used. Attempting to re-initialize a mutex will
 * reset it.
 *
 * This function must be invoked before a mutex can be used.
 *
 * @param mutex - A pointer to the mutex that needs to be initialized
 * @param attribute - A pointer to the attributes of the mutex 
 *
 *@return OTZ_OK if the initialization was successful
 *@return OTZ_INVALID if the value specified by mutex was invalid
 **/
sw_mutex* sw_mutex_init(char *mutex_name);

/**
 * @brief this function destroys the mutex object. Attempting to destroy a mutex
 * that has already been destroyed has no effect. Attempting to destroy a locked
 * mutex fails.
 * 
 * @param mutex - A pointer to the mutex that needs to be destroyed.
 * @return OTZ_OK if the deletion was successful.
 * @return OTZ_INVALID if the value specified by the mutex was invalid.
 * @return OTZ_BUSY if the mutex is locked and cannot be destroyed.
 **/
sw_int sw_mutex_destroy(sw_mutex *mutex);

/**
 * @brief this function locks the mutex object referenced. If the mutex is
 * already locked, it blocks till it becomes available. Once it is available, it
 * is locked. No deadlock detection is provided.
 *
 * @param mutex - A pointer to the mutex that has to be locked.
 * @return OTZ_OK If the attempt to lock was successful.
 * @return OTZ_INVALID If the mutex was un-initialized, of the value is invalid.
 **/
sw_int sw_acquire_mutex(sw_mutex *mutex);

/**
 * @brief this function unlocks the mutex object referenced. Attempts to unlock
 * a mutex that is already locked has no effect.
 *
 * @param mutex - A pointer to the mutex that has to be unlocked.
 * @return OTZ_OK If the attempt to unlock was successful.
 * @return OTZ_EINVAL If the value specified in the mutex is invalid or
 * un-initialized.
 **/
sw_int sw_release_mutex(sw_mutex *mutex);

/**
 * @brief Try to lock the mutex. This function is same as the lock function,
 * except that it returns if the mutex cannot be locked.
 *
 * @param mutex - A pointer to the mutex that needs to be locked.
 * @return OTZ_OK If the lock was successful.
 * @return OTZ_INVALID If the value specified by mutex is invalid or mutex was
 * not initalized.
 * @return OTZ_BUSY If the mutex is already locked.
 **/
sw_int sw_acquire_mutex_nowait(sw_mutex *mutex);

/**
 * @brief 
 */
typedef struct {
} sw_semaphore;

/**
 * @brief This function initializes the semaphore, and sets is counters to the
 * value passed to it. 
 *
 * @param sem - A pointer to the semaphore that needs to be initialized
 * @param shared - Not supported and any value that is passed is ignored.
 * @param value - The value that needs to be assigned to the semaphore.
 *
 * @return OTZ_OK If the initialization suceeded.
 * @return OTZ_INVALID If the semaphore that was passed is invalid.
 **/
sw_semaphore* sw_semaphore_init(char *sem_name, sw_uint value);

/**
 * @brief This function locks the semaphore referenced by the sem value. An
 * atomic decrement is performed. If the counters become negative, This is is
 * blocked, till another task sends an event
 *
 * @param sem - A pointer to the semaphore to be waited upon on.
 *
 * @return OTZ_OK If the operation was successful.
 * @return OTZ_INVALID If the semaphore that was passed to it is invalid.
 **/
sw_int sw_acquire_semaphore(sw_semaphore *sem);

/**
 * @brief This function unlocks the semaphore referenced by sem. The value is
 * incremented atomically. If there are any tasks waiting on this semaphore,
 * only one of them would be released from sw_acquire_semaphore.
 *
 * @param sem - A pointer to the semaphore that needs to be incremented.
 *
 * @return OTZ_OK If the operation is successful.
 * @return OTZ_INVALID If the semaphore is invalid.
 *
 **/
sw_int sw_release_semaphore(sw_semaphore *sem);

/**
 * @brief Delete the semaphore pointed to by sem variable
 * The semaphore cannot be used after deletion.
 *
 * @param sem - The semaphore to be destroyed.
 *
 * @retval OTZ_OK If the semaphore has been deleted successfully.
 * @retval OTZ_INVALID If the semaphore parameter is invalid.
 * @retval OTZ_BUSY If the semaphore is currently busy and hence cannot be
 * destroyed.
 **/
sw_int sw_delete_semaphore(sw_semaphore *sem);

/**
 * @brief Get the current value of the semaphore. It is copied into the argument
 * supplied by the caller.
 *
 * @param sem - The semaphore whose value has to be obtained.
 * @param value - The location where the value of the semaphore needs to be
 * copied.
 *
 * @return OTZ_OK The value has been succesfully copied.
 * @return OTZ_INVALID Invalid value for semaphore or the location where the
 * value needs to be copied into.
 **/
sw_int sw_get_semaphore_count(sw_semaphore *sem, sw_int *value);

#endif
