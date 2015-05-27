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
 * Test application configuration versatile board.The following 
 * details and all configured in service service id,name,heap size,
 * running Mode,group id, check allow multiple instance,asid value,
 * entry point,process name,elf flag,map device name,permission,
 * elf flag and private data
 * 
 */

#ifndef _BOARD_TEST_CONFIG_H__
#define _BOARD_TEST_CONFIG_H__

#include <task.h>

#ifdef CONFIG_TEST_SUITE
/**
 * @brief initializes the sa_config_t function for the test_suite_task_init
 *
 * @param psa_config: Configuration parameter for the task
 *
 * @return SW_OK on success, -1 on failure
 */
int test_suite_task_init(sa_config_t *psa_config);

/*
 * @brief
 * test_suite_task_exit - This function gets called before the task deletion and
 * frees if memory is allocated for data
 *
 * @return
 * SW_OK
 */
int test_suite_task_exit(void *data);

/**
 * @brief: Integer Context Switch task init
 *
 * This function initializes testing task parameters
 *
 * @param psa_config: Configuration parameter for the task and its get called 
 * before the task creation
 *
 * @return
 * SW_OK
 * SW_* - An implementation-defined error code for any other error
 */
int int_contxt_switch_task_init(sa_config_t *psa_config);

/**
 * @brief Integer Context Switch task exit
 *
 * This function gets called before the task deletion
 *
 * @param data : Private data which need to be freed
 *
 * @return
 * SW_OK
 */
int int_contxt_switch_task_exit(void *data);
#endif

#ifdef CONFIG_TEST_TASKS
/**
 * @brief: heap_test_task_init
 *
 * This function initializes testing task parameters
 *
 * @param psa_config: Configuration parameter for the task and its get called 
 * before the task creation
 *
 * @return
 * SW_OK
 * SW_* - An implementation-defined error code for any other error
 */
int heap_test_task_init(sa_config_t *psa_config);

/**
 * @brief heap_test_task_exit
 *
 * This function gets called before the task deletion
 *
 * @param data : Private data which need to be freed
 *
 * @return
 * SW_OK
 */
int heap_test_task_exit(void* data);

/**
 * @brief initializes the sa_config_t function for the test_suite_user_init
 *
 * @param psa_config: Configuration parameter for the task
 *
 * @return SW_OK on success, -1 on failure
 */
int test_suite_user_init(sa_config_t *psa_config);

/*
 * @brief
 * test_suite_user_exit - This function gets called before the task deletion and
 * frees if memory is allocated for data
 *
 * @return
 * SW_OK
 */
int test_suite_user_exit(void *data);

/**
 * @brief initializes the sa_config_t function for the test_shm_init
 *
 * @param psa_config: Configuration parameter for the task
 *
 * @return SW_OK on success, -1 on failure
 */
int test_shm_init(sa_config_t *psa_config);

/*
 * @brief
 * test_shm_exit - This function gets called before the task deletion and
 * frees if memory is allocated for data
 *
 * @return
 * SW_OK
 */
int test_shm_exit(void *data);


#endif

#endif
