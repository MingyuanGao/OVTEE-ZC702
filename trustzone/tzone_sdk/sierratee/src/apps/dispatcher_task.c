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
 * Global task implementation 
 */

#include <sw_types.h>
#include <monitor.h>
#include <sw_debug.h>
#include <sw_string_functions.h>
#include <sw_mem_functions.h>
#include <cpu_data.h>
#include <global.h>
#include <task.h>
#include <tzhyp.h>

#include <global.h>
#include <otz_common.h>
#include <otz_id.h>
#include <smc_id.h>

#include <cpu.h>
#include <dispatcher_task.h>
#include <secure_api.h>
#include <sw_board.h>
#include <sw_heap.h>
#include <sw_addr_config.h>
#include <sw_app_api.h>

/**
 * @brief Dispatcher task entry point
 *
 * This task will process the smc commands and helps in creating 
 * the service tasks and calls the service task functions. \n
 * 1. On arrival of API command invocation "valid params flag" got set. 
 * Based on the flag, it process the command by invoking the appropriate task.\n
 * 2. At the completion of processing, the corresponding task sets the return
 * value of the command result in the dispatcher private data.
 * 3. Based on the result and the source, it returns to non-secure world or 
 * originated task in case of IPI.
 * 
 * @param cur_task_id - Dispatcher task identifier
 */
void dispatch_task(int cur_task_id, sw_tls *tls)
{
	int ret_val;
	int svc_id, task_id, cmd_id, cmd_type;

	task_init(cur_task_id, tls);
	
	sw_printk("SW: dispatch task id 0x%x\n", cur_task_id);
	sw_printk("# ");
	global_val.exec_mode = 0x1;

	while (1) {
		if(secure_interrupt_set) {
			secure_interrupt_set = 0;   
			return_secure_api(0);   
		}
		else if(global_val.linux_return_flag){
			start_task(global_val.linux_task_id,NULL);
		}
		else if(valid_params_flag && params_stack[0] == CALL_TRUSTZONE_API) {
			valid_params_flag = 0;
			get_api_context(&svc_id, &task_id, &cmd_id, &cmd_type);

			if((svc_id == OTZ_SVC_GLOBAL) && 
					(cmd_id == OTZ_GLOBAL_CMD_ID_BOOT_ACK)) {
				if(cmd_type == OTZ_CMD_TYPE_NS_TO_SECURE) {
					tzhyp_boot_ack_event();
				}
				set_secure_api_ret(SMC_SUCCESS);
			}
			else  if((svc_id == OTZ_SVC_GLOBAL) && 
					(cmd_id == OTZ_GLOBAL_CMD_ID_OPEN_SESSION)) {
				if(cmd_type == OTZ_CMD_TYPE_NS_TO_SECURE) {
					/*                    sw_buddy_print_state(); */
					ret_val = open_session_from_ns((void*)params_stack[1]);
					if(ret_val == SW_ERROR)
						ret_val = sw_geterrno();					
					if(ret_val == SW_ENOMEM)
						set_secure_api_ret(SMC_ENOMEM);
					else if(ret_val == SW_EIO)
						set_secure_api_ret(SMC_ERROR);
					else if(ret_val == SW_EINVAL)
						set_secure_api_ret(SMC_EINVAL_ARG);
					else
						set_secure_api_ret(SMC_SUCCESS);
				}
			}
			else if((svc_id == OTZ_SVC_GLOBAL) && 
					(cmd_id == OTZ_GLOBAL_CMD_ID_CLOSE_SESSION)) {
				if(cmd_type == OTZ_CMD_TYPE_NS_TO_SECURE) {
					ret_val = close_session_from_ns((void*)params_stack[1]);
					/*                    sw_buddy_print_state(); */
					if(ret_val == SW_ERROR)
						ret_val = sw_geterrno();					
					if(ret_val == SW_ENOMEM)
					set_secure_api_ret(SMC_ENOMEM);
					else if(ret_val == SW_EIO)
						set_secure_api_ret(SMC_ERROR);
					else if(ret_val == SW_EINVAL)
						set_secure_api_ret(SMC_EINVAL_ARG);
					else
						set_secure_api_ret(SMC_SUCCESS);
				}
			}
#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
			else if((svc_id == OTZ_SVC_GLOBAL) && 
					(cmd_id == OTZ_GLOBAL_CMD_ID_REGISTER_NOTIFY_MEMORY)) {
				if(cmd_type == OTZ_CMD_TYPE_NS_TO_SECURE) {
					ret_val = register_notify_data_api((void*)params_stack[1]);
					if(ret_val == SW_ERROR)
						ret_val = sw_geterrno();
					if(ret_val == SW_ENOMEM)
						set_secure_api_ret(SMC_ENOMEM);
					else if(ret_val == SW_EIO)
						set_secure_api_ret(SMC_ERROR);
					else if(ret_val == SW_EINVAL)
						set_secure_api_ret(SMC_EINVAL_ARG);
					else
						set_secure_api_ret(SMC_SUCCESS);
				}
			}
			else if((svc_id == OTZ_SVC_GLOBAL) && 
					(cmd_id == OTZ_GLOBAL_CMD_ID_UNREGISTER_NOTIFY_MEMORY)) {
				if(cmd_type == OTZ_CMD_TYPE_NS_TO_SECURE) {
					ret_val = unregister_notify_data_api();
					if(ret_val == SW_ERROR)
						ret_val = sw_geterrno();
					if(ret_val == SW_ENOMEM)
						set_secure_api_ret(SMC_ENOMEM);
					else if(ret_val == SW_EIO)
						set_secure_api_ret(SMC_ERROR);
					else if(ret_val == SW_EINVAL)
						set_secure_api_ret(SMC_EINVAL_ARG);
					else
						set_secure_api_ret(SMC_SUCCESS);
				}
			}
			else if((svc_id == OTZ_SVC_GLOBAL) && 
					(cmd_id == OTZ_GLOBAL_CMD_ID_RESUME_ASYNC_TASK)) {
				if(cmd_type == OTZ_CMD_TYPE_NS_TO_SECURE) {
					if(task_id)
						resume_async_task(task_id);
				}
			}
#endif
			else if(svc_id != OTZ_SVC_INVALID  && 
					task_id != 0 && 
					cmd_id != OTZ_GLOBAL_CMD_ID_INVALID) {
				start_task(task_id, params_stack);
			}
			else {
				sw_printk("SW: Invalid service id 0x%x\n", svc_id);
				set_secure_api_ret(0);
			}
		}
		schedule();
		if(global_val.g_dispatch_data->secure_api_set) {
			return_secure_api(global_val.g_dispatch_data->secure_api_ret_val);
			global_val.g_dispatch_data->secure_api_set = 0;
		}

	}
}

/**
 * @brief Task return value notification
 *
 * This function gets called after the API completion to notify the dispatcher 
 * task and it also sets the return value of API.
 * @param ret_val: API return value
 */
void set_secure_api_ret(int ret_val)
{
	global_val.g_dispatch_data->secure_api_set = TRUE;
	global_val.g_dispatch_data->secure_api_ret_val = ret_val;
	return;
}

