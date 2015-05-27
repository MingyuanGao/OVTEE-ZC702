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
 * Async test task implementation
 */

#include <sw_types.h>
#include <sw_debug.h>
#include <sw_string_functions.h>
#include <sw_mem_functions.h>
#include <sw_semaphore.h>
#include <sw_shared_memory.h>
#include <otz_common.h>
#include <otz_id.h>
#include <task_control.h>
#include <otz_app_eg.h>
#include <sw_user_app_api.h>
#include <test_suite_user.h>

/**
 * @brief test_suite_task entry point
 *
 * This function implements the commands to echo the input buffer using copying 
 * the buffer and using shared memory
 *
 * @param task_id: task identifier
 * @param tls: Pointer to task local storage
 */
void test_suite_user(int task_id, sw_tls* tls)
{
	task_init(task_id, tls);
	tls->ret_val = 0;
	tls->ret_val = process_otzapi(task_id, tls);
	task_exit(task_id, tls);
	tee_panic("test suite task - hangs\n");
}

/**
 * @brief Process test suite service 
 *
 * This function process the test suite service commands
 *
 * @param svc_cmd_id: Command identifer to process the test suite service command
 * @param req_buf: Virtual address of the request buffer
 * @param req_buf_len: Request buffer length
 * @param resp_buf: Virtual address of the response buffer
 * @param res_buf_len: Response buffer length
 * @param meta_data: Virtual address of the meta data of the encoded data
 * @param ret_res_buf_len: Return length of the response buffer
 *
 * @return SMC return codes:
 * SMC_SUCCESS: test service command processed successfully. \n
 * SMC_*: An implementation-defined error code for any other error.
 */
int process_otz_test_user_svc(sw_uint svc_cmd_id, 
		void *req_buf, sw_uint req_buf_len, 
		void *resp_buf, sw_uint res_buf_len,
		struct otzc_encode_meta *meta_data, sw_uint *ret_res_buf_len) 
{
	int ret_val = SMC_ERROR;
	switch (svc_cmd_id) {
		case OTZ_TEST_SUITE_CMD_ID_SHM:
			ret_val = process_otz_test_suite_shm(req_buf, req_buf_len,
					resp_buf, res_buf_len, meta_data, ret_res_buf_len);
			break;
		case OTZ_TEST_SUITE_CMD_ID_MUTEX:
			ret_val = process_otz_test_suite_mutex(req_buf, req_buf_len,
					resp_buf, res_buf_len, meta_data, ret_res_buf_len);
			break;
		default:
			ret_val = SMC_EOPNOTSUPP;
			break;
	}
	return ret_val;
}

/**
 * @brief Test the mutex operations
 *
 * This function tests the functionality of mutex and semaphores
 *
 * @param req_buf: Virtual address of the request buffer
 * @param req_buf_len: Request buffer length
 * @param res_buf: Virtual address of the response buffer
 * @param res_buf_len: Response buffer length
 * @param meta_data: Virtual address of the meta data of the encoded data
 * @param ret_res_buf_len: Return length of the response buffer
 *
 * @return SMC return codes:
 * SMC_SUCCESS: API processed successfully. \n
 * SMC_*: An implementation-defined error code for any other error.
 */
int process_otz_test_suite_mutex(void *req_buf, sw_uint req_buf_len,
		void *res_buf, sw_uint res_buf_len,
		struct otzc_encode_meta *meta_data,
		sw_uint *ret_res_buf_len)
{
	sw_mutex *g_otz_mutex;
	otz_mutex_test_data_t *otz_mutex_test_data = NULL;
	unsigned char *out_buf;
	int ret_val = SMC_SUCCESS;
	int offset = 0, pos = 0, mapped = 0, type, out_len=0;

	otz_mutex_test_data = (otz_mutex_test_data_t*)sw_malloc
								(sizeof(otz_mutex_test_data_t));
	if(!otz_mutex_test_data) {
		sw_seterrno(SW_ENOMEM);
		ret_val = SMC_ENOMEM;
		goto mutex_cmd_ret;
	}

	while (offset <= req_buf_len) {
		if(decode_data(req_buf, meta_data, &type, &offset, &pos, &mapped,
					(void**)&out_buf, &out_len)) {
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto mutex_cmd_ret;
		}
		if(type != OTZ_ENC_UINT32) {
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto mutex_cmd_ret;
		}
		otz_mutex_test_data->len = *((sw_uint*)out_buf);
		if(decode_data(req_buf, meta_data, &type, &offset, &pos, &mapped,
					(void**)&out_buf, &out_len)) {
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto mutex_cmd_ret;
		}
		if(type != OTZ_MEM_REF) {
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto mutex_cmd_ret;
		}
		if(out_len < DATA_BUF_LEN) {
			sw_memcpy(otz_mutex_test_data->data, out_buf, out_len);
		} else {
			sw_seterrno(SW_ENOMEM);
			ret_val = SMC_ENOMEM;
			goto mutex_cmd_ret;
		}
		if(type == OTZ_MEM_REF)
			free_decoded_data(out_buf);
		break;
	}
	g_otz_mutex = sw_mutex_init("MUTEX_TEST");
	sw_printf("SW: Attempting to lock the variable \n");
	if(sw_acquire_mutex(g_otz_mutex) == OTZ_INVALID) {
		sw_printf("SW: Unable to lock mutex. It is invalid \n");
		goto handle_error;
	}
	sw_printf("SW: Lock successful. Trying to lock it one more time \n");
	if(sw_acquire_mutex_nowait(g_otz_mutex) == OTZ_BUSY) {
		sw_printf("SW: Mutex already locked. We cannot lock it anymore !! \n");
	}
	sw_release_mutex(g_otz_mutex);
	sw_printf("SW: Unlock successful. Trying to lock it one more time \n");
	if(sw_acquire_mutex_nowait(g_otz_mutex) == OTZ_BUSY) {
		sw_printf("SW: Error while unlocking the mutex !! \n");
		goto handle_error;
	}

	sw_printf("SW: Going to sleep \n");
	__sw_usleep(10);

	sw_printf("SW: Second time locking successful \n");
	sw_release_mutex(g_otz_mutex);
	sw_mutex_destroy(g_otz_mutex);
handle_error:
	offset = 0, pos = OTZ_MAX_REQ_PARAMS;
	while (offset <= res_buf_len) {
		if(decode_data(res_buf, meta_data, &type, &offset, &pos, &mapped,
					(void**)&out_buf, &out_len)) {
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto mutex_cmd_ret;
		}
		if(type != OTZ_MEM_REF) {
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto mutex_cmd_ret;
		}
		if(out_len >= otz_mutex_test_data->len) {
			sw_memcpy(out_buf,otz_mutex_test_data->response, otz_mutex_test_data->len);
			if(update_response_len(meta_data, pos, otz_mutex_test_data->len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto mutex_cmd_ret;
			}
		} else {
			sw_seterrno(SW_ENOMEM);
			ret_val = SMC_ENOMEM;
			goto mutex_cmd_ret;
		}
		if(type == OTZ_MEM_REF)
			free_decoded_data(out_buf);
		break;
	}
	*ret_res_buf_len = otz_mutex_test_data->len;

mutex_cmd_ret:
	if(otz_mutex_test_data)	
		sw_free(otz_mutex_test_data);
	return ret_val;
	
	return 0;
}

/*
 * Test shared memory and semaphore operations
 */

int process_otz_test_suite_shm(void *req_buf, sw_uint req_buf_len,
		void *res_buf, sw_uint res_buf_len,
		struct otzc_encode_meta *meta_data,
		sw_uint *ret_res_buf_len)
{
	sw_semaphore *sem = sw_semaphore_init("SHARED_MEM_TEST", 1);
	sw_uint *addr = NULL, status = 0;
	char *buffer;
	buffer = "Shared memory is memory that may be simultaneously\r\n \
		accessed by multiple programs with an intent to provide communication \r\n\
		among them or avoid redundant copies. Shared memory is an efficient\r\n\
		means of passing data between programs. Depending on context,programs\r\n\
		may run on a single  processor or on multiple separate processors. \r\n\
		Using memory for communication inside a single program, for example \r\n\
		among its multiple threads, is also referred to as shared memory. In \r\n\
		computing, shared memory is memory that may be simultaneously accessed\r\n\
		by multiple programs with an intent to provide communication among them\r\n\
		or avoid redundant copies. Shared memory is an efficient means of \r\n\
		passing data between programs. Depending on context, programs may run\r\n\
		on a single processor or on multiple separate processors. Using memory\r\n\
		for communication inside a single program, for example among its \r\n\
			multiple threads, is also referred to as shared memory.\r\n";
	sw_uint shm_id = shm_create("SHARED_MEM_TEST", 1024, shm_flag_read 
			| shm_flag_write);
	if(shm_id == SW_ERROR)
		goto error;
	if((sw_uint)(addr = shm_attach(shm_id, NULL, shm_flag_write)) == SW_ERROR)
		goto error;
	sw_acquire_semaphore(sem);
	shm_control(shm_id, shm_cmd_stat, &status);
	if((status & shm_flag_read) || (status == 0)){
		sw_memcpy(addr, buffer, sw_strlen(buffer));
		status = shm_flag_write;
		shm_control(shm_id, shm_cmd_set_stat, &status);
	}
	shm_detach(shm_id, addr);
error:
	return SW_OK;
}

