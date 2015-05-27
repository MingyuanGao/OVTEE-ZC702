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

#include <task.h>
#include <otz_id.h>
#include <secure_api.h>
#include <global.h>
#include <board_test.h>
#include <board_config.h>
#include <int_contxt_switch_task.h>
#include <heap_test_task.h>
#include <test_suite_task.h>
#include <test_suite_user.h>
#include <test_shm.h>
#include <echo_task.h>
#include <sw_heap.h>
#include <sw_debug.h>
#include <otz_id.h>
#include <page_table.h>
#include <sw_mmu_helper.h>
#include <sw_heap.h>
#include <sw_string_functions.h>
#include <sw_mem_functions.h>
#include <mem_mng.h>
#include <sw_user_mgmt.h>
#include <sw_device_id.h>

/**
 * @brief Create entry_point,open_session and start the task
 *          of test application by their corresponding
 *          sa_config_t instance and service_id
 */
void  run_init_test_tasks(void) {

	int ret_val = SW_ERROR;
/*Integer context switch task1*/
#ifdef CONFIG_TEST_SUITE
	sa_config_t sa_config_ctxt1;
	int ctxt_task_id1;
    ret_val = sa_create_entry_point(OTZ_SVC_INT_CONTXT_SWITCH, &sa_config_ctxt1);
    if(ret_val == SW_OK) {
        if(sa_open_session(&sa_config_ctxt1, (void*)&ctxt_task_id1)
                == SW_OK) {
            start_task(ctxt_task_id1, NULL);
        }
    }
#endif

#ifdef CONFIG_TEST_TASKS
/*Heap_buddy_allocator task */
	sa_config_t test_config;
	int test_task_id;
	ret_val = sa_create_entry_point(OTZ_SVC_TEST_HEAP, &test_config);
	if(ret_val == SW_OK) {
        if(sa_open_session(&test_config, (void*)&test_task_id)
                == SW_OK) {
            start_task(test_task_id, NULL);
        }
    }
#endif

#ifdef CONFIG_TEST_SUITE
/*Integer context switch task2*/
	sa_config_t sa_config_ctxt2;
	int ctxt_task_id2;
    ret_val = sa_create_entry_point(OTZ_SVC_INT_CONTXT_SWITCH, &sa_config_ctxt2);
    if(ret_val == SW_OK) {
        if(sa_open_session(&sa_config_ctxt2, (void*)&ctxt_task_id2)
                == SW_OK) {
            start_task(ctxt_task_id2, NULL);
        }
    }
#endif
#ifdef CONFIG_TEST_TASKS
	sa_config_t sa_config_ctxt3;
	int ctxt_task_id3;
    ret_val = sa_create_entry_point(OTZ_SVC_TEST_SHM, &sa_config_ctxt3);
    if(ret_val == SW_OK) {
        if(sa_open_session(&sa_config_ctxt3, (void*)&ctxt_task_id3)
                == SW_OK) {
            start_task(ctxt_task_id3, NULL);
        }
    }
#endif
}

#ifdef CONFIG_TEST_TASKS
/**
 * @brief initializes the sa_config_t function for the test_shm_init
 *
 * @param psa_config: Configuration parameter for the task
 *
 * @return SW_OK on success, -1 on failure
 */
int test_shm_init(sa_config_t *psa_config)
{
	sw_memset(psa_config,0x0,sizeof(sa_config_t));

	/* Service id */
	psa_config->service_uuid = OTZ_SVC_TEST_SHM;
	/* Service name */
	sw_strncpy(psa_config->service_name, "test_shm",
			SERVICE_NAME_LEN-1);
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
	update_global_ACL(UART_DEVICE_ID,COMMON_GROUP_ID,OTZ_SVC_TEST_SHM);


	/* Service entry point */
	psa_config->entry_point = (sw_uint)&test_shm;
	
	/* Private data for this service */
	psa_config->data = (void*)sw_malloc_private(COMMON_HEAP_ID,
			sizeof(struct echo_global));
	if(!psa_config->data) {
		sw_printk("SW: test suite task init: allocation of local storage data failed\n");
		sw_seterrno(SW_ENOMEM);
		return SW_ERROR;
	}
	sw_memset(psa_config->data, 0, sizeof(struct echo_global));


	return SW_OK;
}

/*
 * @brief
 * test_shm_exit - This function gets called before the task deletion and
 * frees if memory is allocated for data
 *
 * @return
 * SW_OK
 */
int test_shm_exit(void *data)
{
	if(data)
		sw_free_private(COMMON_HEAP_ID, data);
	return SW_OK;
}

/**
 * @brief initializes the sa_config_t function for the test_suite_user_init
 *
 * @param psa_config: Configuration parameter for the task
 *
 * @return SW_OK on success, -1 on failure
 */
int test_suite_user_init(sa_config_t *psa_config)
{
	sw_memset(psa_config,0x0,sizeof(sa_config_t));

	/* Service id */
	psa_config->service_uuid = OTZ_SVC_TEST_SUITE_USER;
	/* Service name */
	sw_strncpy(psa_config->service_name, "test_suite_user",
			SERVICE_NAME_LEN-1);
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
	update_global_ACL(UART_DEVICE_ID,COMMON_GROUP_ID,OTZ_SVC_TEST_SUITE_USER);

	/* Service file pathname */
	sw_strncpy(psa_config->file_path, "/apps/test_suite_user.o",
			FILE_PATH_LEN-1);
	psa_config->file_path[FILE_PATH_LEN-1]='\0';
	/* service entry function name */
	sw_strncpy(psa_config->entry_func,"test_suite_user",
			ENTRY_FUNC_LEN-1);
	psa_config->entry_func[ENTRY_FUNC_LEN-1]='\0';
	/* Service process name */
	sw_strncpy(psa_config->process_name,"process_otz_test_user_svc",
			PROCESS_NAME_LEN-1);
	psa_config->process_name[PROCESS_NAME_LEN-1]='\0';
	
	/* Service entry point */
	psa_config->entry_point = (sw_uint)&test_suite_user;
	/* Service process */
	psa_config->process = &process_otz_test_user_svc;

	/* Private data for this service */
	psa_config->data = (void*)sw_malloc_private(COMMON_HEAP_ID,
			sizeof(struct echo_global));
	if(!psa_config->data) {
		sw_printk("SW: test suite user init: allocation of local storage data failed\n");
		sw_seterrno(SW_ENOMEM);
		return SW_ERROR;
	}
	sw_memset(psa_config->data, 0, sizeof(struct echo_global));


	return SW_OK;
}

/*
 * @brief
 * test_suite_task_exit - This function gets called before the task deletion and
 * frees if memory is allocated for data
 *
 * @return
 * SW_OK
 */
int test_suite_user_exit(void *data)
{
	if(data)
		sw_free_private(COMMON_HEAP_ID, data);
	return SW_OK;
}
#endif

#ifdef CONFIG_TEST_SUITE
/**
 * @brief initializes the sa_config_t function for the test_suite_task_init
 *
 * @param psa_config: Configuration parameter for the task
 *
 * @return SW_OK on success, -1 on failure
 */
int test_suite_task_init(sa_config_t *psa_config)
{
	sw_memset(psa_config,0x0,sizeof(sa_config_t));

	/* Service id */
	psa_config->service_uuid = OTZ_SVC_TEST_SUITE_KERNEL;
	/* Service name */
	sw_strncpy(psa_config->service_name, "test_suite",
			SERVICE_NAME_LEN-1);
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


	/* Mapping device for this service */
	update_global_ACL(UART_DEVICE_ID,COMMON_GROUP_ID,OTZ_SVC_TEST_SUITE_KERNEL);

	/* Service file pathname */
	sw_strncpy(psa_config->file_path, "/apps/test_suite_task.o",
			FILE_PATH_LEN-1);
	psa_config->file_path[FILE_PATH_LEN-1]='\0';
	/* service entry function name */
	sw_strncpy(psa_config->entry_func,"test_suite_task",
			ENTRY_FUNC_LEN-1);
	psa_config->entry_func[ENTRY_FUNC_LEN-1]='\0';
	/* Service process name */
	sw_strncpy(psa_config->process_name,"process_otz_test_suite_svc",
			PROCESS_NAME_LEN-1);
	psa_config->process_name[PROCESS_NAME_LEN-1]='\0';
	/* Service entry point */
	psa_config->entry_point = (sw_uint)&test_suite_task;
	/* Service process */
	psa_config->process = &process_otz_test_suite_svc;


	/* Private data for this service */
	psa_config->data = (void*)sw_malloc_private(COMMON_HEAP_ID,
			sizeof(struct echo_global));
	if(!psa_config->data) {
		sw_printk("SW: test suite task init: allocation of local storage data failed\n");
		sw_seterrno(SW_ENOMEM);
		return SW_ERROR;
	}
	sw_memset(psa_config->data, 0, sizeof(struct echo_global));


	return SW_OK;
}

/*
 * @brief
 * test_suite_task_exit - This function gets called before the task deletion and
 * frees if memory is allocated for data
 *
 * @return
 * SW_OK
 */
int test_suite_task_exit(void *data)
{
	if(data)
		sw_free_private(COMMON_HEAP_ID, data);
	return SW_OK;
}

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
int int_contxt_switch_task_init(sa_config_t *psa_config)
{
	sw_memset(psa_config,0x0,sizeof(sa_config_t));

	/* Service id */
	psa_config->service_uuid = OTZ_SVC_INT_CONTXT_SWITCH;
	sw_strncpy(psa_config->service_name, "int_contxt_switch",SERVICE_NAME_LEN-1);
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


	/* Mapping device for this service */
    update_global_ACL(UART_DEVICE_ID,COMMON_GROUP_ID,OTZ_SVC_INT_CONTXT_SWITCH);

	psa_config->entry_point = (sw_uint)&int_contxt_switch_task;

	psa_config->data = NULL;


	return SW_OK;
}

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
int int_contxt_switch_task_exit(void *data)
{
	if(data)
		sw_free(data);
	return SW_OK;
}
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
int heap_test_task_init(sa_config_t *psa_config) {

	sw_memset(psa_config, 0x0, sizeof(sa_config_t));

	/* Service id */
	psa_config->service_uuid = OTZ_SVC_TEST_HEAP;
	/* Service name */
	sw_strncpy(psa_config->service_name, "test_heap",SERVICE_NAME_LEN-1);
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
	update_global_ACL(UART_DEVICE_ID,COMMON_GROUP_ID,OTZ_SVC_TEST_HEAP);

    /* Service entry point */
    psa_config->entry_point = (sw_uint)&heap_test_task;

	/* Private data for this service */
	psa_config->data = NULL;

	return SW_OK;
}

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
int heap_test_task_exit(void* data) {
	if(data)
		sw_free_private(COMMON_HEAP_ID, data);
	return SW_OK;
}
#endif
