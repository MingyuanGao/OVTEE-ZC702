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
 * Helper API's for Secure kernel
 */

#include <sw_types.h>
#include <sw_debug.h>
#include <sw_mem_functions.h>
#include <sw_string_functions.h>
#include <cpu_data.h>
#include <page_table.h>
#include <secure_api.h>
#include <otz_common.h>
#include <otz_id.h>
#include <task.h>
#include <task_control.h>
#include <dispatcher_task.h>
#include <echo_task.h>
#include <test_suite_user.h>
#include <drm_task.h>
#include <crypto_task.h>
#include <sw_mmu_helper.h>
#include <cpu.h>
#include <tzhyp_global.h>
#ifdef CONFIG_FFMPEG
#include <ffmpeg_test_task.h>
#endif
#include <linux_task.h>
#include <sw_modinit.h>
#ifdef CONFIG_SHELL
#include <shell_process_task.h>
#endif
#ifdef CONFIG_TEST_SUITE
#include <test_suite_task.h>
#endif
#include <debug_config.h>
#include <board_config.h>
#include <board_test.h>

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
int open_session_from_ns(void *param)
{

	int ret_val=0;
	int *svc_id = NULL;
	sa_config_t sa_config;
	sw_phy_addr cmd_phy;
	struct otz_smc_cmd *cmd = NULL;
	void *session_context = NULL;

	cmd_phy = (sw_phy_addr) param;


	if(map_to_ns(cmd_phy, (sw_vir_addr*) &cmd)) {
		goto ret_func;
	}


	if(cmd->resp_buf_len == 0) {
		sw_seterrno(SW_EINVAL);
		goto ret_func;
	}

	if(map_to_ns(cmd->resp_buf_phys, (sw_vir_addr*)&session_context) != 0) {
		goto ret_func;
	}

	if(map_to_ns(cmd->req_buf_phys, (sw_vir_addr*)&svc_id) != 0) {
		goto ret_func;
	}

	if(*svc_id == OTZ_SVC_INVALID) {
		sw_seterrno(SW_EINVAL);
		goto ret_func;
	}

	ret_val = sa_create_entry_point(*svc_id, &sa_config);
	if(ret_val == SW_OK) {
		sa_config.guest_no = get_current_guest_no();
		ret_val = sa_open_session(&sa_config, session_context);
	}

	if(ret_val != SW_OK) {
		sa_destroy_entry_point(*svc_id,
				sa_config.data,sa_config.elf_flag);
		cmd->cmd_status = OTZ_STATUS_INCOMPLETE;
	}
	else {
		sw_printk("SW: session context %x\n", *((int*)session_context));
		cmd->ret_resp_buf_len = sizeof(int);
		cmd->cmd_status = OTZ_STATUS_COMPLETE;
	}

ret_func:
	if(svc_id)
		unmap_from_ns((sw_vir_addr)svc_id);

	if(session_context)
		unmap_from_ns((sw_vir_addr)session_context);

	if(cmd)    
		unmap_from_ns((sw_vir_addr)cmd);    

	return ret_val;
}

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
int close_session_from_ns(void *param)
{
	int ret_val = SW_OK;
	int *svc_id = NULL;
	sw_phy_addr cmd_phy;
	struct otz_smc_cmd *cmd = NULL;
	void *session_context = NULL;
	struct sw_task *task;

	cmd_phy = (sw_phy_addr) param;

	if(map_to_ns(cmd_phy, (sw_vir_addr*) &cmd)) {
		ret_val = SW_ERROR;
		goto ret_func;
	}

	if(cmd->req_buf_len == 0) {
		sw_seterrno(SW_EINVAL);
		ret_val = SW_ERROR;
		goto ret_func;
	}

	if(map_to_ns(cmd->req_buf_phys, (sw_vir_addr*)&svc_id) != 0) {
		ret_val = SW_ERROR;
		goto ret_func;
	}

	if(map_to_ns(cmd->resp_buf_phys, (sw_vir_addr*)&session_context) != 0) {
		ret_val = SW_ERROR;
		goto ret_func;
	}

	task = get_task(*((sw_uint*)session_context));

    sw_printk("SW: Session closed %x, Sending Bye from Secure World\n",\
                                                *((int*)session_context));
	sa_destroy_entry_point(*svc_id, task->tls->private_data, task->elf_flag);
	sa_close_session(session_context);


	cmd->cmd_status = OTZ_STATUS_COMPLETE;

ret_func:
	if(svc_id)
		unmap_from_ns((sw_vir_addr)svc_id);

	if(session_context)
		unmap_from_ns((sw_vir_addr)session_context);

	if(cmd)
		unmap_from_ns((sw_vir_addr)cmd);    

	return ret_val;
}

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
int open_session_from_secure(int svc_id, int *session_id)
{
	int ret_val;
	sa_config_t sa_config;

	if(svc_id == OTZ_SVC_INVALID) {
		sw_seterrno(SW_EINVAL);
		ret_val = SW_ERROR;
		goto ret_func;
	}
	sw_phy_addr ses_id = secure_vir_to_phy((sw_vir_addr)session_id);
	ses_id = (ses_id & ~(PAGE_SIZE - 1));

	ret_val = sa_create_entry_point(svc_id, &sa_config);
	if(ret_val == SW_OK) {
	}

	if(ret_val == SW_OK) {
		sw_printk("SW: session context %x\n", *((int*)session_id));
	}
	else {
		sa_destroy_entry_point(svc_id, sa_config.data,
				sa_config.elf_flag);   
	}
ret_func:
	return ret_val;
}

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
int close_session_from_secure(int session_id)
{
	struct sw_task *task;

	task = get_task(session_id);
	if(!task)
		return SW_ERROR;
		
	sa_destroy_entry_point(task->service_id, task->tls->private_data, task->elf_flag);
	sa_close_session((void*)&session_id);
	return SW_OK;
}

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
int sa_create_entry_point(int svc_id, sa_config_t *psa_config)
{
	int ret=SW_OK;

	if(svc_id == OTZ_SVC_ECHO) {
		if(echo_task_init(psa_config) != SW_OK)
			return SW_ERROR;
	} 
#ifdef FIX_TASK_IMPLEMENTATION
	else if(svc_id == OTZ_SVC_DRM) {
		if( drm_task_init(psa_config) != SW_OK)
			return SW_ERROR;
	}
#endif	
#ifdef CONFIG_CRYPTO	
	else if(svc_id == OTZ_SVC_CRYPT) {
		if( crypto_task_init(psa_config) != SW_OK)
			return SW_ERROR;
	}
#endif
	else if(svc_id == OTZ_SVC_GLOBAL) {
		ret = dispatch_task_init(psa_config);
		psa_config->guest_no = -1;
		return ret;
	}
#ifdef CONFIG_KIM
	else if(svc_id == OTZ_SVC_KERNEL_INTEGRITY_CHECK) {
		ret = kernel_integrity_check_task_init(psa_config);
		psa_config->guest_no = -1;
		return ret;
	}
#endif
	else if(svc_id == OTZ_SVC_LINUX) {
		ret = linux_task_init(psa_config);
		psa_config->guest_no = -1;
		return ret;
       	}

#ifdef CONFIG_SHELL
	else if(svc_id == OTZ_SVC_SHELL) {
		ret = shell_cmd_process_init(psa_config);
		psa_config->guest_no = -1;
		return ret;
	}
#endif
#ifdef CONFIG_TEST_TASKS
	else if(svc_id == OTZ_SVC_TEST_HEAP) {
        if(heap_test_task_init(psa_config) != SW_OK)
        	return ret;
    }
	else if(svc_id == OTZ_SVC_TEST_SHM) {
		if(test_shm_init(psa_config) != SW_OK)
			return SW_ERROR;
	}
	else if(svc_id == OTZ_SVC_TEST_SUITE_USER) {
		if(test_suite_user_init(psa_config) != SW_OK)
			return SW_ERROR;
	}
#endif
#ifdef CONFIG_FFMPEG
	else if(svc_id == OTZ_SVC_FFMPEG_TEST) {
		if(ffmpeg_test_task_init(psa_config) != SW_OK)
			return SW_ERROR;
	}
#endif
#ifdef CONFIG_TEST_SUITE
	else if(svc_id == OTZ_SVC_INT_CONTXT_SWITCH) {
        if( int_contxt_switch_task_init(psa_config) != SW_OK)
            return SW_ERROR;
    }
	else if(svc_id == OTZ_SVC_TEST_SUITE_KERNEL) {
		if(test_suite_task_init(psa_config) != SW_OK)
			return SW_ERROR;
	}
#endif
	else
		return SW_ERROR;
	psa_config->guest_no = -1;
	return ret;
}

/**
 * @brief Invokes the exit function of the service task
 *
 *
 * This function invokes the exit function of the corresponding service task
 *
 * @param svc_id: Service identifier of the task
 * @param data: Private data which need to be freed
 *
 * @return otz_return_t:
 * OTZ_OK - Session closed successfully\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int sa_destroy_entry_point(int svc_id, void *data, int elf_flag)
{

	if(svc_id == OTZ_SVC_ECHO) {
		return echo_task_exit(data);
	}
#ifdef FIX_TASK_IMPLEMENTATION
	if(svc_id == OTZ_SVC_DRM) {
		return drm_task_exit(data);
	}
#endif	
#ifdef CONFIG_CRYPTO
	if(svc_id == OTZ_SVC_CRYPT) {
		return crypto_task_exit(data);
	}
#endif
	if(svc_id == OTZ_SVC_GLOBAL) {
		return dispatch_task_exit(data);
	}
#ifdef CONFIG_KIM
	if(svc_id == OTZ_SVC_KERNEL_INTEGRITY_CHECK){
		return kernel_integrity_check_task_exit(data);
	}
#endif
 	if(svc_id == OTZ_SVC_LINUX) {
           return linux_task_exit(data);
   	}
#ifdef CONFIG_SHELL
	if(svc_id == OTZ_SVC_SHELL) {
		return shell_cmd_process_exit(data);
	}
#endif
#ifdef CONFIG_TEST_TASKS
	if(svc_id == OTZ_SVC_TEST_HEAP) {
        return heap_test_task_exit(data);
    }
	if(svc_id == OTZ_SVC_TEST_SHM) {
		return test_shm_exit(data);
	}
	if(svc_id == OTZ_SVC_TEST_SUITE_USER) {
		return test_suite_user_exit(data);
	}
#endif
#ifdef CONFIG_FFMPEG
	if(svc_id == OTZ_SVC_FFMPEG_TEST) {
		return ffmpeg_test_task_exit(data);
	}
#endif
#ifdef CONFIG_TEST_SUITE
	if(svc_id == OTZ_SVC_TEST_SUITE_KERNEL) {
		return test_suite_task_exit(data);
	}
	if(svc_id ==  OTZ_SVC_INT_CONTXT_SWITCH) {
        return int_contxt_switch_task_exit(data);
    }
#endif
	sw_seterrno(SW_EINVAL);
	return SW_ERROR;
}


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
int sa_open_session(sa_config_t *psa_config, void *session_context)
{
	int ret_val;
	/* session context = task id */
	ret_val = create_task(psa_config, (int*)session_context);

	return ret_val;
}

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
int sa_close_session(void *session_context)
{
	int ret_val;
	ret_val = destroy_task(*((int*)session_context));
	return ret_val;
}


/**
 * @brief Helper function to return the service id, session id and command id 
 * from the smc command parameter
 *
 * @param svc_id: service identifier as output parameter
 * @param task_id: session context as output parameter
 * @param cmd_id: command identifier as output parameter
 * @param cmd_type: command type as output parameter
 */
void get_api_context(int *svc_id, int *task_id, int *cmd_id, int *cmd_type)
{
	sw_phy_addr cmd_phy;
	struct otz_smc_cmd *cmd = NULL;

	*svc_id = OTZ_SVC_INVALID;
	*task_id = 0;
	*cmd_id = OTZ_GLOBAL_CMD_ID_INVALID;
	*cmd_type = params_stack[2];

	if(*cmd_type == OTZ_CMD_TYPE_NS_TO_SECURE) {
		cmd_phy = (sw_phy_addr) params_stack[1];
		if(!cmd) {
			if(map_to_ns(cmd_phy, (sw_vir_addr*) &cmd))
				return;
		}
	}
	else {
		cmd = (struct otz_smc_cmd *)params_stack[1];
	}

	*svc_id = ((cmd->id >> 10) & (0x3ff));
	*cmd_id = (cmd->id & 0x3ff);
	*task_id = cmd->context;

	if(*cmd_type == OTZ_CMD_TYPE_NS_TO_SECURE) {
		unmap_from_ns((sw_vir_addr)cmd);
	}

	return;
}

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
		void **out_data, int *out_len)
{
	int ret = SW_OK;
	int temp_offset = *offset, temp_pos = *pos;

	switch(meta_data[temp_pos].type) {
		case OTZ_ENC_UINT32: {
								 *offset = temp_offset + sizeof(sw_uint);
								 *pos = temp_pos + 1;
								 *type = OTZ_ENC_UINT32;
								 *out_data =  data + temp_offset;
								 *out_len =  meta_data[temp_pos].len;
								 break;
							 }
		case OTZ_SECURE_MEM_REF: {
									 *out_data = (void*)(*((sw_uint*)data + temp_offset));
									 *offset = temp_offset + sizeof(sw_uint);
									 *type = OTZ_MEM_REF;
									 *pos = temp_pos + 1;
									 *out_len =  meta_data[temp_pos].ret_len;
									 break;
								 }
		default:
								 sw_seterrno(SW_EINVAL);
								 ret = SW_ERROR;
								 break;
	}
	return ret;
}

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
int ipc_send(int src_svc_id, int src_context, int svc_id, int session_id, int cmd_id, 
		struct otz_secure_encode_ctx *enc_ctx, 
		struct otzc_encode_meta *meta_data)
{
	struct otz_smc_cmd smc_cmd;

	smc_cmd.src_id = (src_svc_id << 10);
	smc_cmd.src_context = src_context;
	smc_cmd.id = (svc_id << 10) | cmd_id;
	smc_cmd.context = session_id;
	smc_cmd.req_buf_len = enc_ctx->enc_req_offset;
	smc_cmd.resp_buf_len = enc_ctx->enc_res_offset;
	smc_cmd.ret_resp_buf_len = 0;

	if(enc_ctx->req_addr)
		smc_cmd.req_buf_phys = (sw_phy_addr)enc_ctx->req_addr;
	else
		smc_cmd.req_buf_phys = 0;

	if(enc_ctx->res_addr)
		smc_cmd.resp_buf_phys = (sw_phy_addr)enc_ctx->res_addr;
	else
		smc_cmd.resp_buf_phys = 0;

	if(meta_data)
		smc_cmd.meta_data_phys = (sw_phy_addr)meta_data;
	else
		smc_cmd.meta_data_phys = 0;       

	otz_ipi(&smc_cmd);

	return SW_OK;
}

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
		void* res_buf, int res_buf_len, int *ret_res_buf_len)
{
	int ret_val= SW_OK;
#ifdef USER_SPACE_IPC_IMPLEMENTED
	int session_id;
	struct otz_secure_encode_cmd enc;
	struct otzc_encode_meta *meta_data = NULL;
	struct otz_secure_encode_ctx *enc_ctx = NULL;
	void *decode_out;
	int offset = 0, pos = OTZ_MAX_REQ_PARAMS, type;

	ret_val = __sw_open("IPC", target_svc_id, (sw_int)&session_id);

	if(ret_val != SW_OK) {
		sw_printk("SW: ipc api: connect failed\n");
		goto ret_func;
	}

	enc.len = req_buf_len;
	enc.data = req_buf;
	enc.param_type = OTZC_PARAM_IN;

	ret_val = otz_encode_data(&enc, &meta_data, &enc_ctx, OTZ_SECURE_MEM_REF);
	if(ret_val != SW_OK) {
		sw_printk("SW: ipc api: encode data failed\n");
		goto handler_err1;
	}

	enc.len = res_buf_len;
	enc.data = res_buf;
	enc.param_type = OTZC_PARAM_OUT;

	ret_val = otz_encode_data(&enc, &meta_data, &enc_ctx, OTZ_SECURE_MEM_REF);
	if(ret_val != SW_OK) {
		sw_printk("SW: ipc api: encode data failed\n");
		goto handler_err1;
	}

	ret_val = ipc_send(src_svc_id, src_session_id, target_svc_id, session_id, 
			target_cmd_id, enc_ctx, meta_data);

	if(ret_val != SW_OK) {
		sw_printk("SW: ipc api: send failed\n");
		goto handler_err1;
	}

	if(get_task_tls(src_session_id)->ipi_ret_val == SMC_SUCCESS) {

		if(decode_ipi_out_data(enc_ctx->res_addr, meta_data, 
					&type, &offset, &pos,(void**)&decode_out, ret_res_buf_len)) {

			otz_release_data(enc_ctx, meta_data);
			sw_printk("SW: ipc api: decode data failed\n");
			goto handler_err1;
		}

		if(decode_out != res_buf) {
			otz_release_data(enc_ctx, meta_data);
			sw_printk("SW: ipc api: wrong response buffer\n");
			goto handler_err1;
		}
	}
	else {
		otz_release_data(enc_ctx, meta_data);
		sw_printk("SW: ipc api: target service returns error 0x%x\n", 
				get_task_tls(src_session_id)->ipi_ret_val);
		goto handler_err1;
	}    
	otz_release_data(enc_ctx, meta_data);

handler_err1:
	__sw_close(session_id);
ret_func:
#endif
	return ret_val;

}

/**
 * @brief: Test IPC command for crypto operation 
 *
 * This function helps to test the IPC functionality by invoking crypto
 * operation.
 *
 * @param src_svc_id: Source service ID
 * @param src_session_id: Source session ID
 */
void ipc_test_crypto(int src_svc_id, int src_session_id)
{
#ifdef USER_SPACE_IPC_IMPLEMENTED
	int ret_val, session_id, loop_cntr;
	struct otz_secure_encode_cmd enc;
	struct otzc_encode_meta *meta_data = NULL;
	struct otz_secure_encode_ctx *enc_ctx = NULL;
	char *input_string = "This is a test for IPC encryption/decryption";

	char output_buf[256], *decode_out;
	int output_buf_len = 256;

	char decrypt_output_buf[256];
	int decrypt_output_buf_len = 256;

	int offset = 0, pos = OTZ_MAX_REQ_PARAMS, type;

	sw_uint input_buf_len = sw_strnlen(input_string,44)+1;

	ret_val = sw_open("IPC", OTZ_SVC_CRYPT, &session_id);

	if(ret_val != SW_OK) {
		sw_printk("SW: ipc test cmd: connect failed\n");
		goto ret_func;
	}

	enc.len = sizeof(sw_uint);
	enc.data = (void*)input_buf_len;
	enc.param_type = OTZC_PARAM_IN;

	ret_val = otz_encode_data(&enc, &meta_data, &enc_ctx, OTZ_ENC_UINT32);
	if(ret_val != SW_OK) {
		sw_printk("SW: ipc test cmd: encode data failed\n");
		goto handler_err1;
	}

	enc.len = input_buf_len;
	enc.data = input_string;
	enc.param_type = OTZC_PARAM_IN;

	ret_val = otz_encode_data(&enc, &meta_data, &enc_ctx, OTZ_SECURE_MEM_REF);
	if(ret_val != SW_OK) {
		sw_printk("SW: ipc test cmd: encode data failed\n");
		goto handler_err1;
	}

	enc.len = output_buf_len;
	enc.data = output_buf;
	enc.param_type = OTZC_PARAM_OUT;

	ret_val = otz_encode_data(&enc, &meta_data, &enc_ctx, OTZ_SECURE_MEM_REF);
	if(ret_val != SW_OK) {
		sw_printk("SW: ipc test cmd: encode data failed\n");
		goto handler_err1;
	}

	ret_val = ipc_send(src_svc_id, src_session_id, OTZ_SVC_CRYPT, 
			session_id, OTZ_CRYPT_CMD_ID_ENCRYPT, 
			enc_ctx, meta_data);

	if(ret_val != SW_OK) {
		sw_printk("SW: ipc test cmd: send failed\n");
		goto handler_err1;
	}

	if(get_task_tls(src_session_id)->ipi_ret_val == SMC_SUCCESS) {

		if(decode_ipi_out_data(enc_ctx->res_addr, meta_data, 
					&type, &offset, &pos,(void**)&decode_out, &output_buf_len)) {

			otz_release_data(enc_ctx, meta_data);
			sw_printk("SW: ipc test cmd: decode data failed\n");
			goto handler_err1;
		}

		if(decode_out != output_buf) {
			sw_printk("SW: ipc test cmd: wrong out buffer\n");
		}
		sw_printk("SW: IPC Encrypted string is ");
		for(loop_cntr=0;loop_cntr<output_buf_len;loop_cntr++) {
			sw_printk("SW: 0x%x ",output_buf[loop_cntr]);
		}

	}
	else {
		otz_release_data(enc_ctx, meta_data);
		sw_printk("SW: ipc test cmd: service returns error 0x%x failed\n", 
				get_task_tls(src_session_id)->ipi_ret_val);
		goto handler_err1;
	}    
	otz_release_data(enc_ctx, meta_data);


	meta_data = NULL;
	enc_ctx = NULL;

	enc.len = sizeof(sw_uint);
	enc.data = (void*)output_buf_len;
	enc.param_type = OTZC_PARAM_IN;

	ret_val = otz_encode_data(&enc, &meta_data, &enc_ctx, OTZ_ENC_UINT32);
	if(ret_val != SW_OK) {
		sw_printk("SW: ipc test cmd: encode data failed\n");
		goto handler_err1;
	}

	enc.len = output_buf_len;
	enc.data = output_buf;
	enc.param_type = OTZC_PARAM_IN;

	ret_val = otz_encode_data(&enc, &meta_data, &enc_ctx, OTZ_SECURE_MEM_REF);
	if(ret_val != SW_OK) {
		sw_printk("SW: ipc test cmd: encode data failed\n");
		goto handler_err1;
	}

	enc.len = decrypt_output_buf_len;
	enc.data = decrypt_output_buf;
	enc.param_type = OTZC_PARAM_OUT;

	ret_val = otz_encode_data(&enc, &meta_data, &enc_ctx, OTZ_SECURE_MEM_REF);
	if(ret_val != SW_OK) {
		sw_printk("SW: ipc test cmd: encode data failed\n");
		goto handler_err1;
	}

	ret_val = ipc_send(src_svc_id, src_session_id, OTZ_SVC_CRYPT, session_id, 
			OTZ_CRYPT_CMD_ID_DECRYPT, 
			enc_ctx, meta_data);

	if(ret_val != SW_OK) {
		sw_printk("SW: ipc test cmd: send failed\n");
		goto handler_err1;
	}

	if(get_task_tls(src_session_id)->ipi_ret_val == SMC_SUCCESS) {
		if(decode_ipi_out_data(enc_ctx->res_addr, meta_data, 
					&type, &offset, &pos, (void**)&decode_out, 
					&decrypt_output_buf_len)) {

			otz_release_data(enc_ctx, meta_data);
			sw_printk("SW: ipc test cmd: decode data failed\n");
			goto handler_err1;
		}

		if(decode_out != decrypt_output_buf) {
			sw_printk("SW: ipc test cmd: wrong out buffer\n");
		}
		sw_printk("SW: IPC: decrypted output %s\n", decrypt_output_buf);
	}
	else {
		otz_release_data(enc_ctx, meta_data);
		sw_printk("SW: ipc test cmd: service returns error 0x%x failed\n", 
				get_task_tls(src_session_id)->ipi_ret_val);
		goto handler_err1;
	}

	otz_release_data(enc_ctx, meta_data);

handler_err1:
/*
	sw_close(session_id, OTZ_SVC_CRYPT);
*/
	sw_close(session_id);

ret_func:
	return;
#endif
}

/**
 * @brief Test IPC command for echo operation
 *
 * This function helps to test IPC command of echo service.
 *
 * @param src_svc_id: Source service ID
 * @param src_session_id: Source session ID
 */
void ipc_test_echo(int src_svc_id, int src_session_id)
{
#if defined (USER_SPACE_IPC_IMPLEMENTED)
	char* req_buf = "Test IPI echo cmd";
	int   req_buf_len = sw_strnlen(req_buf,17) + 1;

	char res_buf[256];
	int  res_buf_len = 256;
	int  ret_val;

	ret_val = ipc_api(src_svc_id, src_session_id, OTZ_SVC_ECHO,  
			OTZ_ECHO_CMD_ID_IPI_SEND_CMD, req_buf, req_buf_len, 
			res_buf, res_buf_len, &res_buf_len);

	if(ret_val == SW_OK) {
		sw_printk("SW: ipc echo test: %s\n", res_buf);
	}
#endif
}



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
void handle_task_return(int task_id, sw_tls* tls)
{
	exit_task(task_id, tls);
	schedule();
}

/**
 * @brief Helper function to handle the task exit
 *
 * This function helps to handle the exit functionality of task. This function
 * puts the task in to wait or suspend state based on the return value and also
 * helps to set the return value of secure call or IPC call.
 *
 * @param task_id: Task ID
 * @param tls: Task local storage 
 */
void exit_task(int task_id, sw_tls* tls)
{
	suspend_task(task_id, TASK_STATE_SUSPEND);

	if(tls->params[2] == OTZ_CMD_TYPE_NS_TO_SECURE){
		set_secure_api_ret(tls->ret_val);
	}
	else if(tls->params[2] == OTZ_CMD_TYPE_SECURE_TO_SECURE)
		otz_ipi_return((struct otz_smc_cmd *)tls->params[1], tls->ret_val);
}



#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
/**
 * @brief Send notification to non-secure world
 *
 * This function sends the notification message to Non-secure world.
 *
 * @param task_id: Task ID
 */
void notify_ns(int task_id)
{
	struct sw_task *task;
	sw_phy_addr cmd_phy;
	sw_uint *params;
	struct otz_smc_cmd *cmd = NULL;


	task = get_task(task_id);
	if(!task->tls)
		return;

	params = task->tls->params;
	if(!params){
		return;
	}
	if(params[2] == OTZ_CMD_TYPE_NS_TO_SECURE) {
		cmd_phy = (sw_phy_addr) params[1];
		if(!cmd) {
			if(map_to_ns(cmd_phy, (sw_vir_addr*) &cmd)) {
				return;
			}
		}
	}
	else {
		cmd = (struct otz_smc_cmd *)params[1];
	}


	invoke_ns_callback(task->guest_no, ((cmd->id >> 10) & (0x3ff)), 
			cmd->context, cmd->enc_id, cmd->src_context,
			cmd->dev_file_id);

	return;   
}

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
int register_notify_data_api(void *param)
{
	int ret_val = SW_OK;
	sw_phy_addr cmd_phy;
	struct otz_smc_cmd *cmd = NULL;

	cmd_phy = (sw_phy_addr) param;


	if(map_to_ns(cmd_phy, (sw_vir_addr*) &cmd)) {
		ret_val = SW_ERROR;
		goto ret_func;
	}

	ret_val = register_notify_data(cmd->req_buf_phys);

ret_func:

	if(cmd)    
		unmap_from_ns((sw_vir_addr)cmd);    

	return ret_val;
}

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
int unregister_notify_data_api(void)
{
	unregister_notify_data();
	return SW_OK;
}
#endif
