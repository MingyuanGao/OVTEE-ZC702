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
 * Header for Async test task implementation
 */

#ifndef _TEST_SUITE_TASK_H_
#define _TEST_SUITE_TASK_H_

#include <sw_types.h>

/**
 * @brief test_suite_task entry point
 *
 * This function implements the commands to echo the input buffer using copying 
 * the buffer and using shared memory
 *
 * @param task_id: task identifier
 * @param tls: Pointer to task local storage
 */
void test_suite_task(int task_id, sw_tls* tls);

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
		struct otzc_encode_meta *meta_data, sw_uint *ret_res_buf_len);

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
int process_otz_test_suite_async(void *req_buf, sw_uint req_buf_len, 
		void *res_buf, sw_uint res_buf_len, 
		struct otzc_encode_meta *meta_data,
		sw_uint *ret_res_buf_len);

/**
 * @brief Interrupt handler for async test task
 *
 * This function set the variable to implement the functionality of notification
 * meachanism.
 *
 * @param interrupt: Interrupt ID
 * @param data: TLS data
 */
void async_test_task_handler(struct timer_event* tevent);

#if 0
/**
 * @brief 
 *      Dummy wake up function. It is called from wakeup/wakeup function
 * @param queue_item
 * @param wakeup_flag
 */
void test_wq_dummy_wakeup_function(struct task_queue_item *queue_item, 
		sw_uint wakeup_flag);

/**
 * @brief 
 *      This function wakes up the task but does not set the condition
 *      So the task will go back to sleep again.
 *      This function is called from Timer Interrupt handler
 * @param tevent
 */
void test_wq_noset_condition(struct timer_event* tevent);

/**
 * @brief 
 * This function sets the condition and wakes up the task
 * This function is called from Timer Interrupt handler
 * @param tevent
 * @param tevent
 */
void test_wq_set_condition(struct timer_event* tevent);

/**
 * @brief 
 * This function initializes the waitqueue and put itself to a waitqueue.
 * It also registers timer handlers whill will set the condition and wakes
 * up the task again
 * @param task_id
 */
void test_wait_queue(int task_id);
#endif

#endif