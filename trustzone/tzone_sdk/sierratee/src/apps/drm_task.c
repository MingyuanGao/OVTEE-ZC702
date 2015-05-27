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
 * DRM task implementation
 */

#include <sw_types.h>
#include <monitor.h>
#include <sw_debug.h>
#include <cpu_data.h>
#include <global.h>
#include <task.h>
#include <dispatcher_task.h>
#include <page_table.h>
#include <sw_string_functions.h>
#include <sw_mem_functions.h>
#include <sw_wait.h>
#include <sw_semaphores.h>
#include <otz_common.h>
#include <otz_id.h>
#include <cpu.h>
#include <task_control.h>
#include <drm_task.h>

#include <otz_app_eg.h>

#include <sw_io.h>
#include <sw_board.h>
#include <sw_heap.h>
#include <sw_user_api.h>

/**
 * @brief Dummy DRM data echo for the user supplied buffer
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
int process_otz_drm_send_cmd(void *req_buf, sw_uint req_buf_len, 
		void *res_buf, sw_uint res_buf_len, 
		struct otzc_encode_meta *meta_data,
		sw_uint *ret_res_buf_len)
{
	drm_data_t *drm_data = NULL;
	char *out_buf;
	int offset = 0, pos = 0, mapped = 0, type, out_len; 
	int ret_val = SMC_SUCCESS;

	drm_data = (drm_data_t*)sw_malloc(sizeof(drm_data_t));
	if(!drm_data) {
		sw_seterrno(SW_ENOMEM);
		ret_val = SMC_ENOMEM;
		goto drm_cmd_ret;
	}


	if(req_buf_len > 0) {
		while (offset <= req_buf_len) {
			if(decode_data(req_buf, meta_data, 
						&type, &offset, &pos, &mapped, (void**)&out_buf, &out_len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto drm_cmd_ret;
			}
			else {
				if(type != OTZ_ENC_UINT32) {
					sw_seterrno(SW_EINVAL);
					ret_val = SMC_EINVAL_ARG;
					goto drm_cmd_ret;
				}

				drm_data->len = *((sw_uint*)out_buf);
			}

			if(decode_data(req_buf, meta_data, 
						&type, &offset, &pos, &mapped, (void**)&out_buf, &out_len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto drm_cmd_ret;
			}
			else {
				if(type != OTZ_ENC_ARRAY) {
					sw_seterrno(SW_EINVAL);
					ret_val = SMC_EINVAL_ARG;
					goto drm_cmd_ret;
				}

				sw_memcpy(drm_data->data, out_buf, drm_data->len);
			}

			break;
		}
	}

	offset = 0, pos = OTZ_MAX_REQ_PARAMS;
	if(res_buf_len > 0) {
		while (offset <= res_buf_len) {
			if(decode_data(res_buf, meta_data, 
						&type, &offset, &pos, &mapped, (void**)&out_buf, &out_len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto drm_cmd_ret;
			}
			else {
				if(type != OTZ_ENC_ARRAY) {
					sw_seterrno(SW_EINVAL);
					ret_val = SMC_EINVAL_ARG;
					goto drm_cmd_ret;
				}
			}
			sw_memcpy(out_buf, drm_data->data, drm_data->len);
			if(update_response_len(meta_data, pos, drm_data->len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto drm_cmd_ret;
			}

			break;
		}
		*ret_res_buf_len = drm_data->len;
	}
drm_cmd_ret:
	if(drm_data)	
		sw_free(drm_data);
	return ret_val;
}

/**
 * @brief Dummy DRM data echo for the shared buffer
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
int process_otz_drm_send_cmd_shared_buf(void *req_buf, sw_uint req_buf_len, 
		void *res_buf, sw_uint res_buf_len, 
		struct otzc_encode_meta *meta_data,
		sw_uint *ret_res_buf_len)
{
	drm_data_t *drm_data;
	char *out_buf;
	int offset = 0, pos = 0, mapped = 0, type, out_len; 
	int ret_val = SW_OK;

	drm_data = (drm_data_t*)sw_malloc(sizeof(drm_data_t));
	if(!drm_data) {
		sw_seterrno(SW_ENOMEM);
		ret_val = SMC_ENOMEM;
		goto drm_cmd_ret;
	}

	if(req_buf_len > 0) {
		while (offset <= req_buf_len) {
			if(decode_data(req_buf, meta_data, 
						&type, &offset, &pos, &mapped, (void**)&out_buf, &out_len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto drm_cmd_ret;
			}
			else {
				if(type != OTZ_ENC_UINT32) {
					sw_seterrno(SW_EINVAL);
					ret_val = SMC_EINVAL_ARG;
					goto drm_cmd_ret;
				}

				drm_data->len = *((sw_uint*)out_buf);
			}

			if(decode_data(req_buf, meta_data, 
						&type, &offset, &pos, &mapped, (void**)&out_buf, &out_len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto drm_cmd_ret;
			}
			else {
				if(type != OTZ_MEM_REF) {
					sw_seterrno(SW_EINVAL);
					ret_val = SMC_EINVAL_ARG;
					goto drm_cmd_ret;
				}
				sw_memcpy(drm_data->data, out_buf, drm_data->len);
				if(type == OTZ_MEM_REF)
					free_decoded_data(out_buf);
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
				goto drm_cmd_ret;
			}
			else {
				if(type != OTZ_MEM_REF) {
					sw_seterrno(SW_EINVAL);
					ret_val = SMC_EINVAL_ARG;
					goto drm_cmd_ret;
				}
			}
			sw_memcpy(out_buf, drm_data->data, drm_data->len);
			if(update_response_len(meta_data, pos, drm_data->len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto drm_cmd_ret;
			}
			if(type == OTZ_MEM_REF)
				free_decoded_data(out_buf);
			break;
		}
		*ret_res_buf_len = drm_data->len;
	}
drm_cmd_ret:
	if(drm_data)	
		sw_free(drm_data);
	return ret_val;
}

/**
 * @brief Process DRM service 
 *
 * This function process the DRM service commands
 *
 * @param svc_cmd_id: Command identifer to process the drm service command
 * @param req_buf: Virtual address of the request buffer
 * @param req_buf_len: Request buffer length
 * @param resp_buf: Virtual address of the response buffer
 * @param res_buf_len: Response buffer length
 * @param meta_data: Virtual address of the meta data of the encoded data
 * @param ret_res_buf_len: Return length of the response buffer
 *
 * @return SMC return codes:
 * SMC_SUCCESS: Drm service command processed successfully. \n
 * SMC_*: An implementation-defined error code for any other error.
 */
int process_otz_drm_svc(sw_uint svc_cmd_id, 
		void *req_buf, sw_uint req_buf_len, 
		void *resp_buf, sw_uint res_buf_len,
		struct otzc_encode_meta *meta_data, sw_uint *ret_res_buf_len) 
{
	int ret_val = SMC_ERROR;

	switch (svc_cmd_id) {
		case OTZ_DRM_CMD_ID_SEND_CMD:
			ret_val = process_otz_drm_send_cmd(
					req_buf,
					req_buf_len,
					resp_buf,
					res_buf_len,
					meta_data,
					ret_res_buf_len);
			break;
		case OTZ_DRM_CMD_ID_SEND_CMD_SHARED_BUF:
			ret_val = process_otz_drm_send_cmd_shared_buf(
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
 * @brief DRM task entry point
 *
 * This function implements the commands to drm the input buffer using copying 
 * the buffer and using shared memory
 *
 * @param task_id: task identifier
 * @param tls: Pointer to task local storage
 */
void drm_task(int task_id, sw_tls* tls)
{
	task_init(task_id, tls);
	tls->ret_val = 0;
#ifdef CONFIG_NS_JIFFIES_WORKAROUND
	disable_ns_jiffies_update();
#endif
	tls->ret_val = process_otzapi(task_id, tls);

#ifdef CONFIG_NS_JIFFIES_WORKAROUND
	enable_ns_jiffies_update();
#endif
	task_exit(task_id, tls);

	tee_panic("drm task - hangs\n");
}

