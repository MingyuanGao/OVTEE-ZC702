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
 *  Helper API declarations for secure kernel
 */

#ifndef __OTZ_SECURE_API_H_
#define __OTZ_SECURE_API_H_

#include <sw_types.h>
#include <otz_common.h>
#include <task.h>
#include <tls.h>
#include <task_control.h>

#define SECURE_IPC_PARAM1	0x00001000
#define SECURE_IPC_PARAM2	0x00002000
#define SECURE_IPC_PARAM3	0x00003000
#define SECURE_IPC_PARAM4	0x00004000
#define SECURE_IPC_PARAM_SIZE	0x00001000


/**
 * @brief Dispatcher command function to handle the Open session request from
 * non-secure world
 *
 * This function invokes the open session API and its get called from dispatcher
 * task. 
 *
 * @param param: pointer to struct otz_smc_cmd
 *
 * @return otz_return_t:
 * OTZ_OK - Session established successfully\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int open_session_from_ns(void *param);


/**
 * @brief Dispatcher command function to handle the Close session request from
 * non-secure world
 *
 * This function invokes the close session API and its get called from dispatcher
 * task. 
 * @param param: pointer to struct otz_smc_cmd
 *
 * @return otz_return_t:
 * OTZ_OK - Session established successfully\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int close_session_from_ns(void *param);

/**
 * @brief Invokes the init function of the service task
 *
 * This function invokes the init function of the corresponding service task
 *
 * @param svc_id: Service identifier of the task
 * @param psa_config: Configuration details for the task
 *
 * @return otz_return_t:
 * OTZ_OK - Session established successfully\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int sa_create_entry_point(int svc_id, sa_config_t *psa_config);


/**
 * @brief Invokes the exit function of the service task
 *
 *
 * This function invokes the exit function of the corresponding service task
 *
 * @param svc_id: Service identifier of the task
 * @param data: Private data which need to be freed
 * @param elf_flag : to free the mapped secure memory
 * @return otz_return_t:
 * OTZ_OK - Session closed successfully\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int sa_destroy_entry_point(int svc_id, void *data, int flag);

/**
 * @brief Open Session
 *
 * This function establish the session between secure and 
 * non-secure application. This function creates the task correspond to the 
 * service ID.
 * 
 * @param psa_config: Pointer to task init configuration structure. 
 * @param session_context: Session ID as output parameter. 
 *
 * @return otz_return_t:
 * OTZ_OK - Session established successfully\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int sa_open_session(sa_config_t *psa_config, void *session_context);

/**
 * @brief Close Session
 *
 * This function close the session which got established between secure and 
 * non-secure application. This function destroys the corresponding service 
 * task.
 *
 * @param session_context: Session ID
 *
 * @return otz_return_t:
 * OTZ_OK - Session closed successfully\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int sa_close_session(void *session_context);

/**
 * @brief Helper function to return the service id, session id and command id 
 * from the smc command parameter
 *
 * @param svc_id: service identifier as output parameter
 * @param task_id: session context as output parameter
 * @param cmd_id: command identifier as output parameter
 * @param cmd_type: command type as output parameter
 */
void get_api_context(int *svc_id, int *task_id, int *cmd_id, int *cmd_type);



/**
 * @brief Open session request from secure application task
 *
 * This function invokes the open session API and its get called from secure
 * application task. This is to initiate the request from secure application,
 * either for IPI or secure to non-secure world communication.
 *
 * @param svc_id: Service ID
 * @param session_id: Pointer to session ID as an output parameter
 *
 * @return otz_return_t:
 * OTZ_OK - Session established successfully\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int open_session_from_secure(int svc_id, int *session_id);

/**
 * @brief Close session request which is got initiated from secure task
 *
 * This function invokes the close session API and its get called from 
 * secure application task. This is to close the request from secure application,
 * either for IPI or secure to non-secure world communication.
 *
 * @param svc_id: Service ID
 * @param session_id: Session ID
 *
 * @return otz_return_t:
 * OTZ_OK - Session established successfully\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int close_session_from_secure(int session_id);

/**
 * @brief Send the data between two tasks
 *
 * This function used to send the data between two tasks. We need to call this
 * function after the session establishment.
 *
 * @param src_svc_id: Source service ID
 * @param src_context: Source session ID
 * @param svc_id: Target service ID
 * @param session_id: Target session ID
 * @param cmd_id: Target command ID
 * @param enc_ctx: Encode context
 * @param meta_data: Meta data 
 *
 * @return otz_return_t:
 * OTZ_OK - Data sent successfully\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 *
 */
int ipc_send(int src_svc_id, int src_context, int svc_id, 
		int session_id, int cmd_id, 
		struct otz_secure_encode_ctx *enc_ctx, 
		struct otzc_encode_meta *meta_data);

/**
 * @brief Helper function to decode the response data for IPI 
 *
 * This helper function decodes the data which got returned from target secure 
 * application.
 *
 * @param data: Encoded data
 * @param meta_data: Meta data helps to identify the encoded data
 * @param type: Data type of the decoded data as output parameter
 * @param offset: Current offset as input parameter and 
 * Next offset as output parameter
 * @param pos: Current position as input parameter and 
 * Next position as output parameter
 * @param out_data: Decoded data as output parameter
 * @param out_len: Decoded data length as output parameter
 *
 * @return otz_return_t:
 * OTZ_OK - Decoded data successfully\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int decode_ipi_out_data(void *data, 
		struct otzc_encode_meta *meta_data,
		int *type, int *offset, int *pos, 
		void **out_data, int *out_len);


/**
 * @brief Helper function to send the data between two tasks for single request
 * and single response buffer 
 *
 * This function helps to send the data between two task of single request and
 * single response. This takes care of session establishment, encoding, sending
 * and decoding of data.
 *
 * @param src_svc_id: Source service ID
 * @param src_session_id: Source session ID
 * @param target_svc_id: Target service ID
 * @param target_cmd_id: Target command ID
 * @param req_buf: Request buffer
 * @param req_buf_len: Request buffer length
 * @param res_buf: Response buffer
 * @param res_buf_len: Response buffer length
 * @param ret_res_buf_len: Return response buffer length
 *
 * @return otz_return_t:
 * OTZ_OK -  API success\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int ipc_api(int src_svc_id, int src_session_id, int target_svc_id, 
		int target_cmd_id, void *req_buf, int req_buf_len, 
		void* res_buf, int res_buf_len, int *ret_res_buf_len);


/**
 * @brief: Test IPC command for crypto operation 
 *
 * This function helps to test the IPC functionality by invoking crypto
 * operation.
 *
 * @param src_svc_id: Source service ID
 * @param src_session_id: Source session ID
 */
void ipc_test_crypto(int src_svc_id, int src_session_id);

/**
 * @brief Test IPC command for echo operation
 *
 * This function helps to test IPC command of echo service.
 *
 * @param src_svc_id: Source service ID
 * @param src_session_id: Source session ID
 */
void ipc_test_echo(int src_svc_id, int src_session_id);

/**
 * @brief Helper function to handle the task return
 *
 * This function helps to handle the return functionality of task. This function
 * puts the task in to wait or suspend state based on the return value and also
 * helps to set the return value of secure call or IPC call.
 *
 * @param task_id: Task ID
 * @param tls: Task local storage 
 */
void handle_task_return(int task_id, sw_tls* tls);

/**
 * @brief Function to handle the task exit
 *
 * This function helps to handle the exit functionality of task.
 *
 * @param task_id: Task ID
 * @param tls: Task local storage 
 */
void exit_task(int task_id, sw_tls* tls);

/* cpu_ipi.c */

/**
 * @brief Invoke IPI  
 *
 * This function formulate the parameters for IPI and suspend the current task
 * and invoke the SWI command for IPI notification
 *
 * @param smc_cmd: Pointer to command structure
 *
 */
void otz_ipi(struct otz_smc_cmd *smc_cmd);

/**
 * @brief Return for IPI
 *
 * This function sets the IPI return value in the originated task. Reschedules
 * the originated task.
 *
 * @param smc_cmd: Command parameter which got passed in IPI
 * @param ret_val: Return value of the target task result.
 */
void otz_ipi_return(struct otz_smc_cmd *smc_cmd, int ret_val);

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
/**
 * @brief Send notification to non-secure world
 *
 * This function sends the notification message to Non-secure world.
 *
 * @param task_id: Task ID
 */
void notify_ns(int task_id);

/**
 * @brief Dispatcher command function to register the shared memory
 * for notification
 *
 * This function registers the shared memory which will be used for 
 * notification from secure world to normal world.
 *
 * @param param: pointer to struct otz_smc_cmd
 *
 * @return otz_return_t:
 * OTZ_OK - Shared memory registration success\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int register_notify_data_api(void *param);

/**
 * @brief Dispatcher command function to un-register the shared memory
 * of notification
 *
 * This function un-registers the shared memory which was used for 
 * notification from secure world to normal world.
 *
 *
 * @return otz_return_t:
 * OTZ_OK - Shared memory un-registration success\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int unregister_notify_data_api(void);
#endif

/**
* @brief Returns the current running guest identifier
*
* @return 
*/
sw_int get_current_guest_no(void);

#endif /* __OTZ_SECURE_API_H_ */


