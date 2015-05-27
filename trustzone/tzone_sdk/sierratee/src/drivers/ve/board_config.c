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


#include <board_config.h>
#include <task.h>
#include <sw_debug.h>
#include <otz_id.h>
#include <page_table.h>
#include <dispatcher_task.h>
#include <sw_string_functions.h>
#include <sw_mem_functions.h>
#include <mem_mng.h>
#include <sw_board.h>
#include <echo_task.h>
#include <shell_process_task.h>
#include <linux_task.h>
#include <drm_task.h>
#include <crypto_task.h>
#include <sw_mmu_helper.h>
#ifdef CONFIG_FFMPEG
#include <ffmpeg_test_task.h>
#endif
#include <sw_heap.h>
#include <sw_user_mgmt.h>
#include <global.h>
#include <sw_device_id.h>

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
int dispatch_task_init(sa_config_t *psa_config) {

	sw_memset(psa_config, 0x0, sizeof(sa_config_t));

	/* Service id */
	psa_config->service_uuid = OTZ_SVC_GLOBAL;


	/* Service name */
	sw_strncpy(psa_config->service_name, "dispatcher",SERVICE_NAME_LEN-1);
	psa_config->service_name[SERVICE_NAME_LEN-1]='\0';


	/* Stack size for service */
	psa_config->stack_size = TASK_STACK_SIZE;
	/* Heap size for service */
	psa_config->heap_size = TASK_HEAP_SIZE;
	/* Service minimum allocation size for the heap size */
	psa_config->min_alloc_size =  MIN_ALLOC_SIZE;


	/* Service running Mode */
	psa_config->mode = TASK_KERN_MODE;
	/* Service group id */
	psa_config->gid = COMMON_GROUP_ID;
	/* Restricts service multiple instance */
	psa_config->allow_multiple_instance = 1;
	/* Service asid value */
	psa_config->task_id = (sw_uint)get_next_asid_val();


	/* Application permission for this service */
	update_global_ACL(OTZ_SVC_GLOBAL,COMMON_GROUP_ID,psa_config->service_uuid);
	update_global_ACL(OTZ_SVC_ECHO,COMMON_GROUP_ID,psa_config->service_uuid);
	update_global_ACL(OTZ_SVC_DRM,COMMON_GROUP_ID,psa_config->service_uuid );
	update_global_ACL(OTZ_SVC_CRYPT,COMMON_GROUP_ID,psa_config->service_uuid);
	update_global_ACL(OTZ_SVC_MUTEX_TEST,COMMON_GROUP_ID,
			psa_config->service_uuid);
#ifdef CONFIG_GUI_SUPPORT
	update_global_ACL(OTZ_SVC_VIRTUAL_KEYBOARD,COMMON_GROUP_ID,
			psa_config->service_uuid);
#endif
#ifdef CONFIG_KIM
	update_global_ACL(OTZ_SVC_KERNEL_INTEGRITY_CHECK,COMMON_GROUP_ID,
			psa_config->service_uuid);
#endif
	update_global_ACL(OTZ_SVC_LINUX,COMMON_GROUP_ID,psa_config->service_uuid);

#ifdef CONFIG_SHELL
	update_global_ACL(OTZ_SVC_SHELL,COMMON_GROUP_ID,psa_config->service_uuid);
#endif
	update_global_ACL(OTZ_SVC_TEST_SUITE_USER, 
			COMMON_GROUP_ID,psa_config->service_uuid);
#ifdef CONFIG_FFMPEG
	update_global_ACL(OTZ_SVC_FFMPEG_TEST,COMMON_GROUP_ID,
			psa_config->service_uuid);
#endif
	update_global_ACL(OTZ_SVC_GP_INTERNAL,COMMON_GROUP_ID,
			psa_config->service_uuid);
	update_global_ACL(OTZ_SVC_TEST_SUITE_KERNEL,COMMON_GROUP_ID,
			psa_config->service_uuid);


	/* Mapping device for this service */
	update_global_ACL(UART_DEVICE_ID,COMMON_GROUP_ID,OTZ_SVC_GLOBAL);


	/* Service entry point */
	psa_config->entry_point = (sw_uint) &dispatch_task;


	/* Private data for this service */
	psa_config->data = (void *)sw_malloc_private(COMMON_HEAP_ID,
			sizeof(struct dispatch_global));
	if(!psa_config->data) {
		sw_printk("SW: dispatch task init: allocation of local storage data failed\n");
		sw_seterrno(SW_ENOMEM);
		return SW_ERROR;
	}
	sw_memset(psa_config->data, 0 , sizeof(struct dispatch_global));
	if(!global_val.g_dispatch_data) {
		global_val.g_dispatch_data = psa_config->data;
	}


	return SW_OK;
}

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
int dispatch_task_exit(void *data) {
	if(data)
		sw_free_private(COMMON_HEAP_ID, data);
	return SW_OK;
}

#ifdef CONFIG_SHELL
/**
 * @brief initializes the sa_config_t function for the shell_cmd_process
 *
 * @param psa_config: Configuration parameter for the task
 *
 * @return SW_OK on success, -1 on failure
 */
int shell_cmd_process_init(sa_config_t *psa_config) {

	sw_memset(psa_config, 0x0, sizeof(sa_config_t));

	/* Service id */
	psa_config->service_uuid = OTZ_SVC_SHELL;
	/* Service name */
	sw_strncpy(psa_config->service_name, "shell", SERVICE_NAME_LEN-1);
	psa_config->service_name[SERVICE_NAME_LEN-1]='\0';


	/* Stack size for service */
	psa_config->stack_size = TASK_STACK_SIZE;
	/* Heap size for service */
	psa_config->heap_size = TASK_HEAP_SIZE;
	/* Service Minimum allocation size for the heap size */
	psa_config->min_alloc_size =  MIN_ALLOC_SIZE;


	/* Service running mode */
	psa_config->mode = TASK_KERN_MODE;
	/* Service group id */	
	psa_config->gid = COMMON_GROUP_ID;
	/* Restricts service multiple instance */
	psa_config->allow_multiple_instance = 1;
	/* Service asid value */
	psa_config->task_id = (sw_uint)get_next_asid_val();


	/* Set Device ID for service */	
	update_global_ACL(UART_DEVICE_ID,COMMON_GROUP_ID,OTZ_SVC_SHELL);


	/* Service entry point */
	psa_config->entry_point = (sw_uint) &shell_process_task;


	/* Private data for this service */	
	psa_config->data = (void*)sw_malloc_private(COMMON_HEAP_ID,
			sizeof(struct shell_global));
	if(!psa_config->data) {
		sw_printf("SW: shell task init:allocation of local storage data failed\n");
		sw_seterrno(SW_ENOMEM);
		return SW_ERROR;
	}
	((struct shell_global *)psa_config->data)->data = NULL;
	((struct shell_global *)psa_config->data)->uart_id = SECURE_UART_ID;	


	return SW_OK;
}

/**
 * @brief
 * shell_cmd_process_exit - This function gets called before the task deletion
 * and frees the allocated memory for data
 *
 * @param data - Private data which need to be freed.
 *
 * @return - Allocated memory is freed on success
 */
int shell_cmd_process_exit(void *data) {
	if(data)
		sw_free_private(COMMON_HEAP_ID,data);
	return SW_OK;
}
#endif

#ifdef CONFIG_KIM
/**
 * @brief 
 *
 * @param psa_config
 *
 * @return 
 */
int kernel_integrity_check_task_init(sa_config_t *psa_config) {

	sw_memset(psa_config,0x0,sizeof(sa_config_t));

	/* Service id */    
	psa_config->service_uuid = OTZ_SVC_KERNEL_INTEGRITY_CHECK;
	/* Service name */
	sw_strncpy(psa_config->service_name,
			"otz_kernel_integrity_check",SERVICE_NAME_LEN-1);
	psa_config->service_name[SERVICE_NAME_LEN-1]='\0';


	/* Stack size for service */	
	psa_config->stack_size = TASK_STACK_SIZE ;
	/* Heap size for service */
	psa_config->heap_size = TASK_HEAP_SIZE;
	/* Service Minimum allocation size for the heap size */
	psa_config->min_alloc_size =  MIN_ALLOC_SIZE;


	/* Service running mode */
	psa_config->mode = TASK_USER_MODE;
	/* Service group id */
	psa_config->gid = COMMON_GROUP_ID;
	/* Restricts service multiple instance */
	psa_config->allow_multiple_instance = 1;


	/* Mapping device for this service */
	update_global_ACL(UART_DEVICE_ID,COMMON_GROUP_ID,
			OTZ_SVC_KERNEL_INTEGRITY_CHECK);


	/* Service entry point */
	psa_config->entry_point = &kernel_integrity_check_task;
	/* service entry function name */
	sw_strncpy(psa_config->entry_func,"kernel_integrity_check_task",
			ENTRY_FUNC_LEN-1);
	psa_config->entry_func[ENTRY_FUNC_LEN-1]='\0';
	/* Service process name */
	psa_config->process = process_otz_kern_check;
	sw_strncpy(psa_config->process_name,"process_otz_kern_check",
			PROCESS_NAME_LEN-1);
	psa_config->process_name[PROCESS_NAME_LEN-1]='\0';
	/* Service file pathname */
	sw_strncpy(psa_config->file_path, "", FILE_PATH_LEN-1);
	psa_config->file_path[FILE_PATH_LEN-1]='\0';


	/* Private data for services */
	psa_config->data = NULL;


	return SW_OK;
}

/**
 * @brief 
 *
 * @param data
 *
 * @return 
 */
int kernel_integrity_check_task_exit(void *data) {
	if(data)
		sw_free(data);
	return SW_OK;
}
#endif

/**
 * @brief 
 *
 * @param psa_config
 *
 * @return 
 */
int linux_task_init(sa_config_t *psa_config) {

	sw_memset(psa_config,0x0,sizeof(sa_config_t));

	/* Service id */    
	psa_config->service_uuid = OTZ_SVC_LINUX;
	/* Service name */
	sw_strncpy(psa_config->service_name, "Linux", SERVICE_NAME_LEN-1);
	psa_config->service_name[SERVICE_NAME_LEN-1]='\0';


	/* Stack size for service */
	psa_config->stack_size   = TASK_STACK_SIZE;
	/* Heap size for service */
	psa_config->heap_size    = TASK_HEAP_SIZE;
	/* Service Minimum allocation size for the heap size */
	psa_config->min_alloc_size =  MIN_ALLOC_SIZE;


	/* Service running mode */
	psa_config->mode         = TASK_KERN_MODE;
	/* Service group id */
	psa_config->gid = COMMON_GROUP_ID;
	/* Restricts service multiple instance */
	psa_config->allow_multiple_instance = 1;
	/* Service asid value */
	psa_config->task_id = (sw_uint)get_next_asid_val();


	/* Mapping device for this service */	
	update_global_ACL(UART_DEVICE_ID,COMMON_GROUP_ID,OTZ_SVC_LINUX);


	/* Service entry point */
	psa_config->entry_point  = (sw_uint) &linux_task;


	/* Private data for this service */	
	psa_config->data         = (void*)sw_malloc_private(COMMON_HEAP_ID,
			sizeof(struct linux_global));
	((struct linux_global *)psa_config->data)->invoke_flag = 0;


	return SW_OK;
}

/**
 * @brief 
 *
 * @param data
 *
 * @return 
 */
int linux_task_exit(void * data) {

	if(data)
		sw_free_private(COMMON_HEAP_ID, data);
	return SW_OK;
}

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
int ffmpeg_test_task_init(void *config) {

	sa_config_t *psa_config;
	psa_config = (sa_config_t *)config;
	sw_memset(psa_config,0x0,sizeof(sa_config_t));

	/* Service id */
	psa_config->service_uuid = OTZ_SVC_FFMPEG_TEST;
	/* Service name */
	sw_strncpy(psa_config->service_name,"ffmpeg_test",SERVICE_NAME_LEN-1);
	psa_config->service_name[SERVICE_NAME_LEN-1]='\0';


	/* Stack size for service */
	psa_config->stack_size = TASK_HEAP_SIZE;  /*  Giving 64k  */
	/* Heap size for service */
	psa_config->heap_size = TASK_HEAP_SIZE;
	/* Service Minimum allocation size for the heap size */
	psa_config->min_alloc_size =  MIN_ALLOC_SIZE;


	/* Service running mode */
	psa_config->mode = TASK_USER_MODE;
	/* Service group id */
	psa_config->gid = COMMON_GROUP_ID;
	/* Restricts service single instance */
	psa_config->allow_multiple_instance = 0;
	/* Service asid value */
	psa_config->task_id = (sw_uint)get_next_asid_val();


	/* Mapping device for this service */
	update_global_ACL(UART_DEVICE_ID,COMMON_GROUP_ID,
			OTZ_SVC_FFMPEG_TEST);
	/* Set Frame buffer for service */	
	update_global_ACL(FB_DEVICE_ID,COMMON_GROUP_ID,
			OTZ_SVC_FFMPEG_TEST);


	/* Service entry point */
	psa_config->entry_point = (sw_uint)&ffmpeg_test_task;
	/* Service process */
	psa_config->process = &process_otz_ffmpeg_test_svc;


	/* Private data for this service */	
	psa_config->data = NULL;


	return SW_OK;
}

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
int ffmpeg_test_task_exit(void* data) {
	if(data)
		sw_free(data);
	return SW_OK;
}

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

int echo_task_init(void *config)
{
	sa_config_t *psa_config;
	psa_config = (sa_config_t *)config;
	sw_memset(psa_config, 0x0, sizeof(sa_config_t));

	/* Service id */
	psa_config->service_uuid = OTZ_SVC_ECHO;
	/* Service name */
	sw_strncpy(psa_config->service_name, "echo",SERVICE_NAME_LEN-1);
	psa_config->service_name[SERVICE_NAME_LEN-1]='\0';


	/* Stack size for service */
	psa_config->stack_size = TASK_STACK_SIZE;
	/* Heap size for service */
	psa_config->heap_size = TASK_HEAP_SIZE;
	/* Service Minimum allocation size for the heap size */
	psa_config->min_alloc_size =  MIN_ALLOC_SIZE;


	/* Service running mode */
	psa_config->mode = TASK_USER_MODE;
	/* Service group id */
	psa_config->gid = COMMON_GROUP_ID;
	/* Restricts service multiple instance */
	psa_config->allow_multiple_instance = 1;
	/* Service asid value */
	psa_config->task_id = (sw_uint)get_next_asid_val();

	/* Mapping device for this service */
	update_global_ACL(UART_DEVICE_ID, COMMON_GROUP_ID, OTZ_SVC_ECHO);
	/* Mapping Inter process communication device for this service */
	update_global_ACL(IPC_DEVICE_ID, COMMON_GROUP_ID, OTZ_SVC_ECHO);


	/* Service entry point */
	psa_config->entry_point = (sw_uint)&echo_task;
	/* Service process */
	psa_config->process = &process_otz_echo_svc;


	/* Private data for this service */
	psa_config->data = (void*)sw_malloc_private(COMMON_HEAP_ID,
			sizeof(struct echo_global));
	if(!psa_config->data) {
		sw_printk("SW: echo task init: allocation of local storage data failed\n");
		sw_seterrno(SW_ENOMEM);
		return SW_ERROR;
	}
	sw_memset(psa_config->data, 0, sizeof(struct echo_global));


	return SW_OK;
}


/*
 * @brief Echo task exit
 *
 * This function gets called before the task deletion
 * @return otz_return_t:
 * SW_OK \n
 * OTZ_FAIL \n 
 */
int echo_task_exit(void *data)
{
	if(data)
		sw_free_private(COMMON_HEAP_ID,data);
	return SW_OK;
}

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

int drm_task_init(sa_config_t *psa_config)
{
	sw_memset(psa_config, 0x0, sizeof(sa_config_t));

	/* Service id */
	psa_config->service_uuid = OTZ_SVC_DRM;
	/* Service name */
	sw_strncpy(psa_config->service_name, "drm",SERVICE_NAME_LEN-1);
	psa_config->service_name[SERVICE_NAME_LEN-1]='\0';


	/* Stack size for service */
	psa_config->stack_size = TASK_STACK_SIZE;
	/* Heap size for service */
	psa_config->heap_size = TASK_HEAP_SIZE;
	/* Service Minimum allocation size for the heap size */
	psa_config->min_alloc_size =  MIN_ALLOC_SIZE;


	/* Service running mode */
	psa_config->mode = TASK_USER_MODE;
	/* Service group id */
	psa_config->gid = COMMON_GROUP_ID;
	/* Restricts service single instance */
	psa_config->allow_multiple_instance = 0;
	/* Service asid value */
	psa_config->task_id = (sw_uint)get_next_asid_val();


	/* Mapping device for this service */
	update_global_ACL(UART_DEVICE_ID,COMMON_GROUP_ID,OTZ_SVC_DRM);


	/* Service entry point */
	psa_config->entry_point = (sw_uint)&drm_task;
	/* Service process */
	psa_config->process = &process_otz_drm_svc;


	/* Private data for this service */
	psa_config->data = (void*)sw_malloc_private(COMMON_HEAP_ID, 
			sizeof(struct drm_global));
	if(!psa_config->data) {
		sw_printk("SW: drm task init: allocation of local storage data failed\n");
		sw_seterrno(SW_ENOMEM);
		return SW_ERROR;
	}
	sw_memset(psa_config->data, 0, sizeof(struct drm_global));


	return SW_OK;
}


/**
 * @brief DRM task exit
 *
 * This function gets called before the task deletion
 * @return otz_return_t:
 * OTZ_OK \n
 * OTZ_FAIL \n 
 */
int drm_task_exit(void *data)
{
	if(data)
		sw_free_private(COMMON_HEAP_ID, data);
	return SW_OK;
}
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
 * @return otz_return_t:
 * OTZ_OK \n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int crypto_task_init(void *config)
{
	sa_config_t *psa_config;
	psa_config = (sa_config_t *)config;
	sw_memset(psa_config, 0x0, sizeof(sa_config_t));

	/* Service id */
	psa_config->service_uuid = OTZ_SVC_CRYPT;
	/* Service name */
	sw_strncpy(psa_config->service_name, "crypto",SERVICE_NAME_LEN-1);
	psa_config->service_name[SERVICE_NAME_LEN-1]='\0';


	/* Stack size for service */
	psa_config->stack_size = TASK_STACK_SIZE;
	/* Heap size for service */
	psa_config->heap_size = TASK_HEAP_SIZE;
	/* Service Minimum allocation size for the heap size */
	psa_config->min_alloc_size =  MIN_ALLOC_SIZE;


	/* Service running mode */
	psa_config->mode = TASK_USER_MODE;
	/* Service group id */
	psa_config->gid = COMMON_GROUP_ID;
	/* Restricts service multiple instance */
	psa_config->allow_multiple_instance = 1;
	/* Service asid value */
	psa_config->task_id = (sw_uint)get_next_asid_val();


	/* Mapping device for this service */
	update_global_ACL(UART_DEVICE_ID,COMMON_GROUP_ID,OTZ_SVC_CRYPT);


	/* Service entry point */
	psa_config->entry_point = (sw_uint)&crypto_task;
	/* Service process */
	psa_config->process = &process_otz_crypto_svc;


	/* Private data for this service */
	psa_config->data = (void *)sw_malloc_private(COMMON_HEAP_ID, 
			sizeof(struct crypto_global));
	if(!psa_config->data) {
		sw_printk("SW: crypto task init: allocation of local storage data failed\n");
		sw_seterrno(SW_ENOMEM);
		return SW_ERROR;
	}


	return SW_OK;
}

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
int crypto_task_exit(void* data)
{
	if(data)
		sw_free_private(COMMON_HEAP_ID, data);
	return SW_OK;
}
#endif
