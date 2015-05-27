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
 * Application configuration versatile board.The following 
 * details and all configured in service service id,name,heap size,
 * running Mode,group id, check allow multiple instance,asid value,
 * entry point,process name,elf flag,map device name,permission,
 * elf flag and private data
 * 
 */

#ifndef _BOARD_CONFIG_H__
#define _BOARD_CONFIG_H__

#include <task.h>

#define COMMON_GROUP_ID 1
#define SUPER_USER_ID 122

/**
 * @brief Dispatcher task init
 *
 * This function initializes dispatcher task parameters and its get called 
 * before the task creation
 *
 * @param psa_config: Configuration parameter for the task
 *
 * @return otz_return_t:
 * OTZ_OK \n
 * OTZ_FAIL \n
 */
int dispatch_task_init(sa_config_t *psa_config);

/**
 * @brief Dispatcher task exit
 *
 * This function gets called before the task deletion
 *
 * @param data: Private data which need to be freed.
 * @return otz_return_t:
 * OTZ_OK \n
 * OTZ_FAIL \n 
 */
int dispatch_task_exit(void *data);

#ifdef CONFIG_SHELL
/**
 * @brief initializes the sa_config_t function for the shell_cmd_process
 *
 * @param psa_config: Configuration parameter for the task
 *
 * @return SW_OK on success, -1 on failure
 */
int shell_cmd_process_init(sa_config_t *psa_config);

/**
 * @brief
 * shell_cmd_process_exit - This function gets called before the task deletion
 * and frees the allocated memory for data
 *
 * @param data - Private data which need to be freed.
 *
 * @return - Allocated memory is freed on success
 */
int shell_cmd_process_exit(void *data);
#endif

#ifdef CONFIG_KIM
/**
 * @brief 
 *
 * @param psa_config
 *
 * @return 
 */
int kernel_integrity_check_task_init(sa_config_t *psa_config);

/**
 * @brief 
 *
 * @param data
 *
 * @return 
 */
int kernel_integrity_check_task_exit(void *data);
#endif

/**
 * @brief 
 *
 * @param psa_config
 *
 * @return 
 */
int linux_task_init(sa_config_t *psa_config);

/**
 * @brief 
 *
 * @param data
 *
 * @return 
 */
int linux_task_exit(void * data);

#ifdef CONFIG_FFMPEG
/**
 * @brief: FFmpeg test task init
 *
 * This function initializes FFmpeg test task parameters
 *
 * @param psa_config: Configuration parameter for the task and its get called 
 * before the task creation
 *
 * @return otz_return_t:
 * OTZ_OK \n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int ffmpeg_test_task_init(void *config);

/**
 * @brief FFmpeg test task exit
 *
 * This function gets called before the task deletion
 *
 * @param data: Private data which need to be freed.
 * @return otz_return_t:
 * SW_OK \n
 * OTZ_FAIL \n 
 */
int ffmpeg_test_task_exit(void* data);
#endif

/**
 * @brief: Echo task init
 *
 * This function initializes echo task parameters
 *
 * @param psa_config: Configuration parameter for the task and its get called 
 * before the task creation
 *
 * @return otz_return_t:
 * SW_OK \n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int echo_task_init(void *config);

/*
 * @brief Echo task exit
 *
 * This function gets called before the task deletion
 * @return otz_return_t:
 * SW_OK \n
 * OTZ_FAIL \n 
 */
int echo_task_exit(void *data);

/**
 * @brief: user task init
 *
 * This function initializes user task parameters
 *
 * @param psa_config: Configuration parameter for the task and its get called 
 * before the task creation
 *
 * @return otz_return_t:
 * SW_OK \n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int user_task_init(sa_config_t *psa_config);

/**
 * @brief User test task exit
 *
 * This function gets called before the task deletion
 *
 * @param data: Private data which need to be freed.
 * @return:
 * SW_OK \n
 * SW_* - An implementation-defined error code for any other error.\n
 */
int user_task_exit(void *data);

#if FIX_TASK_IMPLEMENTATION
/**
 * @brief: DRM task init
 *
 * This function initializes drm task parameters
 *
 * @param psa_config: Configuration parameter for the task and its get called 
 * before the task creation
 *
 * @return otz_return_t:
 * OTZ_OK \n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int drm_task_init(sa_config_t *psa_config);

/**
 * @brief DRM task exit
 *
 * This function gets called before the task deletion
 *
 * @param data: Private data which need to be freed.
 * @return otz_return_t:
 * OTZ_OK \n
 * OTZ_FAIL \n 
 */
int drm_task_exit(void *data);
#endif

#ifdef CONFIG_CRYPTO
/**
 * @brief: Crypto task init
 *
 * This function initializes crypto task parameters
 *
 * @param psa_config: Configuration parameter for the task and its get called 
 * before the task creation
 *
 * @return otz_return_t:already
 * OTZ_OK \n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int crypto_task_init(void *config);

/**
 * @brief Crypto task exit
 *
 * This function gets called before the task deletion
 *
 * @param data: Private data which need to be freed.
 * @return otz_return_t:
 * OTZ_OK \n
 * OTZ_FAIL \n 
 */
int crypto_task_exit(void* data);

#endif

#endif
