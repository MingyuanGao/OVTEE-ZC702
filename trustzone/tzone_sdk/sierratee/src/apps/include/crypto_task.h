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
 * Header for Crypto task implementation
 */

#ifndef __OTZ_APP_CRYPTO_TASK_H__
#define __OTZ_APP_CRYPTO_TASK_H__

#include <sw_types.h>
#include <task_control.h>
#include <tls.h>

/**
 * @brief
 *  Global variables for the task should be defined as a member of the global 
 *  structure 
 */
typedef struct crypto_global
{
}crypto_global;

/**
 * @brief Crypto task entry point
 *
 * This function implements the processing of crypto commands.
 *
 * @param task_id: task identifier
 * @param tls: Pointer to task local storage
 */
void crypto_task(int task_id, sw_tls* tls);


/**
 * @brief Encrypts the data for the user supplied buffer
 *
 * This function encrypts/decrypts the request buffer and 
 * response buffer will have encrypted/decrypted output.
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
int process_otz_crypto_cmd(void *req_buf, sw_uint req_buf_len, 
		void *res_buf, sw_uint res_buf_len,
		struct otzc_encode_meta *meta_data,
		sw_uint *ret_res_buf_len);

/**
 * @brief Process crypto service 
 *
 * This function process the crypto service commands
 *
 * @param svc_cmd_id: Command identifer to process the crypto service command
 * @param req_buf: Virtual address of the request buffer
 * @param req_buf_len: Request buffer length
 * @param resp_buf: Virtual address of the response buffer
 * @param res_buf_len: Response buffer length
 * @param meta_data: Virtual address of the meta data of the encoded data
 * @param ret_res_buf_len: Return length of the response buffer
 *
 * @return SMC return codes:
 * SMC_SUCCESS: Crypto service command processed successfully. \n
 * SMC_*: An implementation-defined error code for any other error.
 */
int process_otz_crypto_svc(sw_uint svc_cmd_id, void *req_buf, sw_uint req_buf_len, 
		void *resp_buf, sw_uint res_buf_len,
		struct otzc_encode_meta *meta_data, sw_uint *ret_res_buf_len);
#endif /* __OTZ_APP_TASK2_H__ */
