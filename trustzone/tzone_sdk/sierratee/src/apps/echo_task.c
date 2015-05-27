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
 * Echo task implementation
 */
#include <sw_types.h>
#include <sw_debug.h>
#include <sw_string_functions.h>
#include <sw_mem_functions.h>
#include <otz_common.h>
#include <otz_id.h>
#include <task_control.h>
#include <echo_task.h>

#include <otz_app_eg.h>

#include <sw_user_heap.h>
#include <tls.h>
#include <sw_user_app_api.h>
#include <sw_syscall.h>

/**
 * @brief Echo the data for the user supplied buffer
 *
 * This function copies the request buffer to response buffer to show the 
 * non-zero copy functionality
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
int process_otz_echo_send_cmd(void *req_buf, sw_uint req_buf_len, 
		void *res_buf, sw_uint res_buf_len, 
		struct otzc_encode_meta *meta_data,
		sw_uint *ret_res_buf_len)
{
	echo_data_t *echo_data = NULL;
	char *out_buf;
	int offset = 0, pos = 0, mapped = 0, type, out_len, ret_val = SW_OK; 

	echo_data = (echo_data_t*)sw_malloc(sizeof(echo_data_t));
	if(!echo_data) {
		sw_seterrno(SW_ENOMEM);
		ret_val = SMC_ENOMEM;
		goto send_cmd_ret;
	}

	if(req_buf_len > 0) {
		while (offset <= req_buf_len) {
			if(decode_data(req_buf, meta_data, 
						&type, &offset, &pos, &mapped, (void**)&out_buf, &out_len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto send_cmd_ret;
			}
			else {
				if(type != OTZ_ENC_UINT32) {
					sw_seterrno(SW_EINVAL);
					ret_val = SMC_EINVAL_ARG;
					goto send_cmd_ret;
				}

				echo_data->len = *((sw_uint*)out_buf);
			}

			if(decode_data(req_buf, meta_data, 
						&type, &offset, &pos, &mapped, (void**)&out_buf, &out_len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto send_cmd_ret;
			}
			else {
				if(type != OTZ_ENC_ARRAY) {
					sw_seterrno(SW_EINVAL);
					ret_val = SMC_EINVAL_ARG;
					goto send_cmd_ret;
				}

				sw_memcpy(echo_data->data, out_buf, echo_data->len);
			}

			break;
		}
	}
	
	sw_printf("echo task data:%s\n", echo_data->data);
	char newdata[10] = "New value";	

	offset = 0, pos = OTZ_MAX_REQ_PARAMS;
	if(res_buf_len > 0) {
		while (offset <= res_buf_len) {
			if(decode_data(res_buf, meta_data, 
						&type, &offset, &pos, &mapped, (void**)&out_buf, &out_len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto send_cmd_ret;
			}
			else {
				if(type != OTZ_ENC_ARRAY) {
					sw_seterrno(SW_EINVAL);
					ret_val = SMC_EINVAL_ARG;
					goto send_cmd_ret;
				}
			}
			//sw_memcpy(out_buf, echo_data->data, echo_data->len);
			sw_memcpy(out_buf, newdata, 10);
			//if(update_response_len(meta_data, pos, echo_data->len)) {
			if(update_response_len(meta_data, pos, 10)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto send_cmd_ret;
			}

			break;
		}
		//*ret_res_buf_len = echo_data->len;
		*ret_res_buf_len = 10;
	}
	/*sw_printf("SW: echo task data: %s\n", echo_data->data);*/
	/*sw_printf("SW: echo send cmd %s and len 0x%x strlen 0x%x\n", echo_data->data,*/
	/*echo_data->len, sw_strlen(echo_data->data));*/
send_cmd_ret:
	if(echo_data)
		sw_free(echo_data);
	return ret_val;
}

/**
 * @brief Echo the data for the shared buffer
 *
 * This function copies the request buffer to response buffer to show the 
 * zero copy functionality
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
int process_otz_echo_send_cmd_shared_buf(void *req_buf, sw_uint req_buf_len, 
		void *res_buf, sw_uint res_buf_len, 
		struct otzc_encode_meta *meta_data,
		sw_uint *ret_res_buf_len)
{
	echo_data_t *echo_data = NULL;
	char *out_buf;
	int offset = 0, pos = 0, mapped = 0, type, out_len, ret_val = SW_OK; 

	echo_data = (echo_data_t*)sw_malloc(sizeof(echo_data_t));
	if(!echo_data) {
		sw_seterrno(SW_ENOMEM);
		ret_val = SMC_ENOMEM;
		goto shared_cmd_ret;
	}

	if(req_buf_len > 0) {
		while (offset <= req_buf_len) {
			if(decode_data(req_buf, meta_data, 
						&type, &offset, &pos, &mapped, (void**)&out_buf, &out_len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto shared_cmd_ret;
			}
			else {
				if(type != OTZ_ENC_UINT32) {
					sw_seterrno(SW_EINVAL);
					ret_val = SMC_EINVAL_ARG;
					goto shared_cmd_ret;
				}

				echo_data->len = *((sw_uint*)out_buf);
			}

			if(decode_data(req_buf, meta_data, 
						&type, &offset, &pos, &mapped, (void**)&out_buf, &out_len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto shared_cmd_ret;
			}
			else {
				if(type != OTZ_MEM_REF) {
					sw_seterrno(SW_EINVAL);
					ret_val = SMC_EINVAL_ARG;
					goto shared_cmd_ret;
				}

				sw_memcpy(echo_data->data, out_buf, echo_data->len);
				if(type == OTZ_MEM_REF)
					free_decoded_data((sw_uint *)out_buf);
			}

			break;
		}
	}

	offset = 0, pos = OTZ_MAX_REQ_PARAMS;
	if (res_buf_len > 0) {
		while (offset <= res_buf_len) {
			if(decode_data(res_buf, meta_data, 
						&type, &offset, &pos, &mapped, (void**)&out_buf, &out_len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto shared_cmd_ret;
			}
			else {
				if(type != OTZ_MEM_REF) {
					sw_seterrno(SW_EINVAL);
					ret_val = SMC_EINVAL_ARG;
					goto shared_cmd_ret;
				}
			}

			sw_memcpy(out_buf, echo_data->data, echo_data->len);
			if(type == OTZ_MEM_REF)
				free_decoded_data((sw_uint *)out_buf);
			if(update_response_len(meta_data, pos, echo_data->len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto shared_cmd_ret;
			}
			break;
		}
		*ret_res_buf_len = echo_data->len;
	}

	/*sw_printf("SW: echo task data: %s\n", echo_data->data);*/
	/*sw_printf("SW: echo send cmd %s and len 0x%x strlen 0x%x\n",
			echo_data->data, echo_data->len, sw_strlen(echo_data->data)); */
shared_cmd_ret:
	if(echo_data)
		sw_free(echo_data);
	return ret_val;

}


/**
 * @brief Echo the data for IPI testing
 *
 * This function copies the request buffer to response buffer to show the 
 * IPI functionality
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
int process_otz_echo_ipi_send_cmd(void *req_buf, sw_uint req_buf_len, 
		void *res_buf, sw_uint res_buf_len, 
		struct otzc_encode_meta *meta_data,
		sw_uint *ret_res_buf_len)
{
	echo_data_t *echo_data = NULL;
	char *out_buf;
	int offset = 0, pos = 0, mapped = 0, type, out_len, ret_val = SW_OK; 

	echo_data = (echo_data_t*)sw_malloc(sizeof(echo_data_t));
	if(!echo_data) {
		sw_seterrno(SW_ENOMEM);
		ret_val = SMC_ENOMEM;
		goto ipi_cmd_ret;
	}

	if(req_buf_len > 0) {
		while (offset <= req_buf_len) {
			if(decode_data(req_buf, meta_data, 
						&type, &offset, &pos, &mapped, (void**)&out_buf, &out_len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto ipi_cmd_ret;
			}
			else {
				if(type != OTZ_MEM_REF) {
					sw_seterrno(SW_EINVAL);
					ret_val = SMC_EINVAL_ARG;
					goto ipi_cmd_ret;
				}

				sw_memcpy(echo_data->data, out_buf, out_len);
				echo_data->len = out_len;
			}
			break;
		}
	}

	offset = 0, pos = OTZ_MAX_REQ_PARAMS;
	if (res_buf_len > 0) {
		while (offset <= res_buf_len) {
			if(decode_data(res_buf, meta_data, 
						&type, &offset, &pos, &mapped, (void**)&out_buf, &out_len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto ipi_cmd_ret;
			}
			else {
				if(type != OTZ_MEM_REF) {
					sw_seterrno(SW_EINVAL);
					ret_val = SMC_EINVAL_ARG;
					goto ipi_cmd_ret;
				}
			}
			sw_memcpy(out_buf, echo_data->data, echo_data->len);
			if(update_response_len(meta_data, pos, echo_data->len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto ipi_cmd_ret;
			}

			break;
		}
		*ret_res_buf_len = echo_data->len;
	}


	/*sw_printf("SW: echo send cmd %s and len 0x%x strlen 0x%x\n",*/
	/*echo_data->data, echo_data->len, sw_strlen(echo_data->data));*/
ipi_cmd_ret:
	if(echo_data)
		sw_free(echo_data);
	return ret_val;
}


/**
 * @brief Process echo service 
 *
 * This function process the echo service commands
 *
 * @param svc_cmd_id: Command identifer to process the echo service command
 * @param req_buf: Virtual address of the request buffer
 * @param req_buf_len: Request buffer length
 * @param resp_buf: Virtual address of the response buffer
 * @param res_buf_len: Response buffer length
 * @param meta_data: Virtual address of the meta data of the encoded data
 * @param ret_res_buf_len: Return length of the response buffer
 *
 * @return SMC return codes:
 * SMC_SUCCESS: Echo service command processed successfully. \n
 * SMC_*: An implementation-defined error code for any other error.
 */
int process_otz_echo_svc(sw_uint svc_cmd_id, 
		void *req_buf, sw_uint req_buf_len, 
		void *resp_buf, sw_uint res_buf_len,
		struct otzc_encode_meta *meta_data, sw_uint *ret_res_buf_len) 
{
	int ret_val = SMC_ERROR;

	switch (svc_cmd_id) {
		case OTZ_ECHO_CMD_ID_SEND_CMD:
			ret_val = process_otz_echo_send_cmd(
					req_buf,
					req_buf_len,
					resp_buf,
					res_buf_len,
					meta_data,
					ret_res_buf_len);
			break;
		case OTZ_ECHO_CMD_ID_SEND_CMD_SHARED_BUF:
		case OTZ_ECHO_CMD_ID_SEND_CMD_ARRAY_SPACE:
			ret_val = process_otz_echo_send_cmd_shared_buf(
					req_buf,
					req_buf_len,
					resp_buf,
					res_buf_len,
					meta_data,
					ret_res_buf_len);
			break;
		case OTZ_ECHO_CMD_ID_IPI_SEND_CMD:
			ret_val = process_otz_echo_ipi_send_cmd(
					req_buf,
					req_buf_len,
					resp_buf,
					res_buf_len,
					meta_data,
					ret_res_buf_len);
			break;
		default:
			ret_val = SMC_EOPNOTSUPP;
			break;
	}

	return ret_val;
}


/**
 * @brief Echo task entry point
 *
 * This function implements the commands to echo the input buffer using copying 
 * the buffer and using shared memory
 *
 * @param task_id: task identifier
 * @param tls: Pointer to task local storage
 */
void echo_task(int task_id, sw_tls* tls)
{
	task_init(task_id, tls);
	tls->ret_val = 0;
	tls->ret_val = process_otzapi(task_id, tls);
	/*__sw_usleep(100);*/
	task_exit(task_id, tls);
	tee_panic("echo task - hangs\n");
}

