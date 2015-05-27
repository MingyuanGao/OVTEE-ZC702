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
#include <otz_common.h>
#include <otz_id.h>
#include <cpu.h>
#include <task_control.h>
#include <echo_task.h>

#include <otz_app_eg.h>

#include <sw_io.h>
#include <sw_board.h>
#include <sw_heap.h>
#include <sw_app_api.h>
#include <test_suite_task.h>
#include <sw_timer_functions.h>

/**
 * @brief test_suite_task entry point
 *
 * This function implements the commands to echo the input buffer using copying 
 * the buffer and using shared memory
 *
 * @param task_id: task identifier
 * @param tls: Pointer to task local storage
 */
void test_suite_task(int task_id, sw_tls* tls)
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
int process_otz_test_suite_svc(sw_uint svc_cmd_id, 
		void *req_buf, sw_uint req_buf_len, 
		void *resp_buf, sw_uint res_buf_len,
		struct otzc_encode_meta *meta_data, sw_uint *ret_res_buf_len) 
{
	int ret_val = SMC_ERROR;
	switch (svc_cmd_id) {
#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT	
		case OTZ_TEST_SUITE_CMD_ID_ASYNC:
			ret_val = process_otz_test_suite_async(req_buf, req_buf_len,
					resp_buf, res_buf_len, meta_data, ret_res_buf_len);
			break;
#endif
		default:
			ret_val = SMC_EOPNOTSUPP;
			break;
	}
	return ret_val;
}

/**
 * @brief test the data for the user supplied buffer with async support
 *
 * This function copies the request buffer to response buffer to show the 
 * non-zero copy functionality and to show the async support by wait for the 
 * flag and it got set in interrupt handler
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

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT	
int process_otz_test_suite_async(void *req_buf, sw_uint req_buf_len, 
		void *res_buf, sw_uint res_buf_len, 
		struct otzc_encode_meta *meta_data,
		sw_uint *ret_res_buf_len)
{
	echo_data_t *echo_data = NULL;
	char *out_buf;
	int offset = 0, pos = 0, mapped = 0, type, out_len;
	int task_id; 
	sw_tls *tls;
	int ret_val = SW_OK;
	struct echo_global *echo_global;

	task_id = get_current_task_id();
	tls = get_task_tls(task_id);
	echo_global = (struct echo_global *)tls->private_data;

	echo_data = (echo_data_t*)sw_malloc(sizeof(echo_data_t));
	if(!echo_data) {
		sw_seterrno(SW_ENOMEM);
		ret_val = SMC_ENOMEM;
		goto async_cmd_ret;
	}
	if(!echo_global->data_available)
	{
		struct timer_event* tevent;
		sw_timeval time;

		tevent = timer_event_create(&async_test_task_handler,(void*)task_id);
		if(!tevent){
			sw_printf("SW: Out of Memory : Cannot register Handler\n");
			sw_seterrno(SW_ENOMEM);
			ret_val = SMC_ENOMEM;
			goto async_cmd_ret;
		}

		/* Time duration = 100ms */
		time.tval.nsec = 0;
		time.tval.sec = 2;

		struct sw_task* task = get_task(task_id);

		task->notify_queue.elements_count = 0;
		link_init(&task->notify_queue.elements_list);
		task->notify_queue.lock_shared.lock = 0;
		timer_event_start(tevent,&time);

#ifdef ASYNC_DBG
		sw_printf("SW: Before calling wait event \n"); 
#endif
		sw_wait_event_async(&task->notify_queue, 
			echo_global->data_available, SMC_PENDING);
#ifdef ASYNC_DBG
		sw_printf("SW: Coming out from wait event \n");
#endif
	}

	if(req_buf_len > 0) {
		while (offset <= req_buf_len) {
			if(decode_data(req_buf, meta_data, 
						&type, &offset, &pos, &mapped, (void**)&out_buf, &out_len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto async_cmd_ret;
			}
			else {
				if(type != OTZ_ENC_UINT32) {
					sw_seterrno(SW_EINVAL);
					ret_val = SMC_EINVAL_ARG;
					goto async_cmd_ret;
				}
				echo_data->len = *((sw_uint*)out_buf);
			}

			if(decode_data(req_buf, meta_data, 
						&type, &offset, &pos, &mapped, (void**)&out_buf, &out_len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto async_cmd_ret;
			}
			else {
				if(type != OTZ_ENC_ARRAY) {
					sw_seterrno(SW_EINVAL);
					ret_val = SMC_EINVAL_ARG;
					goto async_cmd_ret;
				}
				sw_memcpy(echo_data->data, out_buf, echo_data->len);
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
				goto async_cmd_ret;
			}
			else {
				if(type != OTZ_ENC_ARRAY) {
					sw_seterrno(SW_EINVAL);
					ret_val = SMC_EINVAL_ARG;
					goto async_cmd_ret;
				}
			}
			sw_memcpy(out_buf, echo_data->data, echo_data->len);
			if(update_response_len(meta_data, pos, echo_data->len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto async_cmd_ret;
			}
			break;
		}
		*ret_res_buf_len = echo_data->len;
	}
	/*sw_printf("SW: echo task data: %s\n", echo_data->data);*/
	/*sw_printf("SW: echo send cmd %s and len 0x%x strlen 0x%x\n", echo_data->data,*/
	/*echo_data->len, sw_strlen(echo_data->data));*/
	
async_cmd_ret:
	if(echo_data)	
		sw_free(echo_data);
	return ret_val;
}
#endif

/**
 * @brief Interrupt handler for async test task
 *
 * This function set the variable to implement the functionality of notification
 * meachanism.
 *
 * @param interrupt: Interrupt ID
 * @param data: TLS data
 */
#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
void async_test_task_handler(struct timer_event* tevent)
{
	void* data = tevent->data;
	sw_tls *tls;
	struct echo_global *echo_data;

	sw_uint task_id = (sw_uint) data;
	sw_printf("SW: async test task handler 0x%x\n", task_id);
	tls = get_task_tls(task_id);
	if(tls) {
		echo_data = (struct echo_global *)tls->private_data;
		echo_data->data_available = 1;
	}
	if(tls) {    
		notify_ns(task_id);
	}
	else {
		sw_printf("SW: where is the task???\n");
	}

	tevent->state &= ~TIMER_STATE_EXECUTING;
	timer_event_destroy(tevent);
}
#endif 
