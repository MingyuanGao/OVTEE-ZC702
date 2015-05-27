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
 * Task controller implementation
 */

#include <sw_types.h>
#include <sw_debug.h>
#include <sw_string_functions.h>
#include <sw_mem_functions.h>
#include <otz_common.h>
#include <otz_id.h>
#include <otz_app_eg.h>
#include <task_control.h>

#ifndef	SW_KERNEL
#include <sw_syscall.h>
#endif

#ifdef SW_KERNEL
extern int unmap_from_ns(sw_vir_addr);
extern int map_to_ns(sw_phy_addr, sw_vir_addr*);
#endif

/**
 * @brief Helper function to decode the data which got passed from 
 * non-secure world
 *
 * This helper function decodes the data which got encoded from non-secure or
 * secure application.
 *
 * @param data: Encoded data
 * @param meta_data: Meta data helps to identify the encoded data
 * @param type: Data type of the decoded data as output parameter
 * @param offset: Current offset as input parameter and 
 * Next offset as output parameter
 * @param pos: Current position as input parameter and 
 * Next position as output parameter
 * @param mapped: Decoded data is shared memory or not as output parameter
 * @param out_data: Decoded data as output parameter
 * @param out_len: Decoded data length as output parameter
 *
 * @return otz_return_t:
 * OTZ_OK - Decoded data successfully\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int decode_data(void *data, 
		struct otzc_encode_meta *meta_data,
		int *type, int *offset, int *pos, 
		int *mapped, void **out_data, int *out_len)
{
	int ret = SW_OK;
	int temp_offset = *offset, temp_pos = *pos;
	*mapped = 0;

	switch(meta_data[temp_pos].type) {
		case OTZ_ENC_UINT32: {
								 *offset = temp_offset + sizeof(sw_uint);
								 *pos = temp_pos + 1;
								 *type = OTZ_ENC_UINT32;
								 *out_data =  data + temp_offset;
								 *out_len =  meta_data[temp_pos].len;
								 break;
							 }
		case OTZ_ENC_ARRAY: {
								*offset = temp_offset + meta_data[temp_pos].len;
								*pos = temp_pos + 1;
								*type = OTZ_ENC_ARRAY;
								*out_data =  data + temp_offset;
								*out_len =  meta_data[temp_pos].len;
								break;
							}
		case OTZ_MEM_REF: {
							  if(map_to_ns((sw_phy_addr)(*((sw_uint*)data + temp_offset)), (sw_vir_addr*)out_data) != 0)
								  return SW_ERROR;
							  *offset = temp_offset + sizeof(sw_uint);
							  *type = OTZ_MEM_REF;
							  *pos = temp_pos + 1;
							  *mapped = 1;
							  *out_len =  meta_data[temp_pos].len;
							  break;
						  }
		case OTZ_SECURE_MEM_REF: {
									 *out_data = (void*)(*((sw_uint*)data + temp_offset));
									 *offset = temp_offset + sizeof(sw_uint);
									 *type = OTZ_MEM_REF;
									 *pos = temp_pos + 1;
									 *mapped = 0;
									 *out_len =  meta_data[temp_pos].len;
									 break;
								 }
		default:
								 ret = SW_ERROR;
								 break;
	}
	return ret;
}

/*
 * @brief This function free's all the mapped ns memory from the last
 * decode_data
 *
 * Param: mapped ns memory address
 */
int free_decoded_data(sw_uint* ns_addr){
	return unmap_from_ns((sw_vir_addr)ns_addr);
}

/**
 * @brief Helper function to update the response length
 *
 * @param meta_data: Meta data helps to identify the encoded data
 * @param pos: Current position 
 * @param len: new response length
 *
 * @return otz_return_t:
 * OTZ_OK - Updated responsed length successfully\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int update_response_len(struct otzc_encode_meta *meta_data, int pos, int len)
{
	if(pos <= OTZ_MAX_REQ_PARAMS + OTZ_MAX_RES_PARAMS) {
		meta_data[pos -1].ret_len = len;
		return SW_OK;
	}
	else
		sw_seterrno(SW_EINVAL);
		return SW_ERROR;
}

/**
 * @brief Helper function to encode the data which originate from secure
 * application
 *
 * This function encodes the data into a encoder stream and this function gets
 * called from secure application. This will be used for IPI or secure to
 * non-secure communication.
 *
 * @param enc: Encode command structure
 * @param pmeta_data: Meta data which get populated based on the encoding
 * @param penc_context: Encoder context which contains the encoded stream for
 * request and response buffer and its details.
 * @param encode_type: Encoding data type - UINT32 or Shared memory
 *
 * @return otz_return_t:
 * OTZ_OK - Encoded data successfully\n
 * OTZ_* - An implementation-defined error code for any other error.\n
 */
int otz_encode_data(struct otz_secure_encode_cmd *enc, 
		struct otzc_encode_meta **pmeta_data,
		struct otz_secure_encode_ctx **penc_context,
		int encode_type)
{
	int ret_val = SW_OK;
	struct otzc_encode_meta *meta_data = NULL;
	struct otz_secure_encode_ctx *enc_context = NULL;

	meta_data = *pmeta_data;
	enc_context = *penc_context;

	if(enc == NULL) {
		sw_seterrno(SW_EINVAL);
		ret_val = SW_ERROR;
		goto ret_func;
	}

	if(encode_type != OTZ_ENC_UINT32 && encode_type != OTZ_SECURE_MEM_REF) {
		sw_seterrno(SW_EINVAL);
		ret_val = SW_ERROR;
		goto ret_func;
	}

	if(enc_context == NULL) {
		enc_context = (struct otz_secure_encode_ctx *)sw_malloc(
				sizeof(struct otz_secure_encode_ctx));

		if(!enc_context) {
			sw_seterrno(SW_ENOMEM);
			ret_val = SW_ERROR;
			goto ret_free_mem;
		}
		sw_memset(enc_context, 0, sizeof(struct otz_secure_encode_ctx));
	} 

	if(meta_data == NULL) {
		meta_data = (struct otzc_encode_meta *)sw_malloc(
				sizeof(struct otzc_encode_meta) *
				(OTZ_MAX_RES_PARAMS + OTZ_MAX_REQ_PARAMS));
		if(!meta_data) {
			sw_seterrno(SW_ENOMEM);
			ret_val = SW_ERROR;
			goto ret_free_mem;
		}
		sw_memset(meta_data, 0, sizeof(struct otzc_encode_meta) *
				(OTZ_MAX_RES_PARAMS + OTZ_MAX_REQ_PARAMS));
	} 

	if(enc->param_type == OTZC_PARAM_IN) {  
		if(!enc_context->req_addr) {
			enc_context->req_addr = (void *)sw_malloc(OTZ_1K_SIZE);
			if(!enc_context->req_addr) {
				sw_printf("SW: otz_client_encode: request addr malloc failed\n");
				sw_seterrno(SW_ENOMEM);
				ret_val = SW_ERROR;
				goto ret_free_mem;
			}
			enc_context->enc_req_offset = 0;
			enc_context->enc_req_pos = 0;
		}
		if((enc_context->enc_req_offset + sizeof(sw_uint) <= 
					OTZ_1K_SIZE) &&
				(enc_context->enc_req_pos < OTZ_MAX_REQ_PARAMS)) {
			*((sw_uint*)enc_context->req_addr + enc_context->enc_req_offset) =
				(sw_uint)enc->data;
			enc_context->enc_req_offset += sizeof(sw_uint);

			meta_data[enc_context->enc_req_pos].type = encode_type;
			meta_data[enc_context->enc_req_pos].len = enc->len;
			enc_context->enc_req_pos++;
		}
		else {
			sw_seterrno(SW_ENOMEM);
			ret_val = SW_ERROR;
			goto ret_free_mem;
		}
	}
	else if(enc->param_type == OTZC_PARAM_OUT) {   
		if(!enc_context->res_addr) {
			enc_context->res_addr = (void *)sw_malloc(OTZ_1K_SIZE);
			if(!enc_context->res_addr) {
				sw_printf("SW: otz_client_encode: response addr malloc failed\n");
				sw_seterrno(SW_ENOMEM);
				ret_val = SW_ERROR;
				goto ret_free_mem;
			}

			enc_context->enc_res_pos = OTZ_MAX_REQ_PARAMS;
			enc_context->enc_res_offset = 0;
		}

		if((enc_context->enc_res_offset + sizeof(sw_uint)
					<= OTZ_1K_SIZE) &&
				(enc_context->enc_res_pos < 
				 (OTZ_MAX_RES_PARAMS + OTZ_MAX_REQ_PARAMS ))) {
			*((sw_uint*)enc_context->res_addr + 
					enc_context->enc_res_offset) 
				= (sw_uint)enc->data;
			enc_context->enc_res_offset += sizeof(sw_uint);
			meta_data[enc_context->enc_res_pos].type 
				=  encode_type;
			meta_data[enc_context->enc_res_pos].len = enc->len;
			enc_context->enc_res_pos++;
		}
		else {
			sw_seterrno(SW_ENOMEM);
			ret_val = SW_ERROR;
			goto ret_free_mem;
		}   
	}
	else {
		sw_seterrno(SW_EINVAL);
		ret_val = SW_ERROR;
		goto ret_func;
	}
	goto ret_func;

ret_free_mem:
	if(enc_context) {
		if(enc_context->req_addr) {
			sw_free(enc_context->req_addr);
			enc_context->req_addr = NULL;
		}
		if(enc_context->res_addr) {
			sw_free(enc_context->res_addr);
			enc_context->res_addr = NULL;
		}
		sw_free(enc_context);
	} 
	if(meta_data) {
		sw_free(meta_data);
	} 
ret_func:
	if(! (*pmeta_data))
		*pmeta_data = meta_data;
	if(! (*penc_context))
		*penc_context = enc_context;

	return ret_val;
}

/**
 * @brief Release the encoded data
 *
 * This function releases the resources which got allocated in encoding function.
 *
 * @param enc_context: Encode context
 * @param meta_data: Meta data which got populated in encoding
 *
 */
void otz_release_data(struct otz_secure_encode_ctx *enc_context, 
		struct otzc_encode_meta *meta_data)
{
	if(enc_context) {
		if(enc_context->req_addr) {
			sw_free(enc_context->req_addr);
			enc_context->req_addr = NULL;
		}
		if(enc_context->res_addr) {
			sw_free(enc_context->res_addr);
			enc_context->res_addr = NULL;
		}
		sw_free(enc_context);
	} 
	if(meta_data) {
		sw_free(meta_data);
	} 
}


/**
 * @brief Process API
 *
 * This function process the API and also verify the validity of session 
 * identifer
 * @param session_id: Session identifer for the API
 * @param tls: Pointer to task local storage
 *
 * @return SMC return codes:
 * SMC_SUCCESS: API processed successfully. \n
 * SMC_*: An implementation-defined error code for any other error.
 */
int process_otzapi(int session_id, sw_tls *tls)
{
	int ret_val = SW_OK;
	sw_phy_addr cmd_phy;
	sw_uint *params;
	struct otz_smc_cmd *cmd = NULL;
	void *req_buf = NULL, *res_buf = NULL;
	struct otzc_encode_meta *meta_data = NULL;

	sw_uint svc_id, svc_cmd_id;

	params = tls->params;
	if(!params){
		ret_val = SMC_ENOMEM;
		goto ret_func;
	}

	if(params[2] == OTZ_CMD_TYPE_NS_TO_SECURE) {
		cmd_phy = (sw_phy_addr) params[1];
		if(!cmd) {
			if(map_to_ns(cmd_phy, (sw_vir_addr*) &cmd)) {
				ret_val = SMC_ENOMEM;
				goto ret_func;
			}
		}

		if(cmd->req_buf_len >  0) {
			if(map_to_ns(cmd->req_buf_phys, (sw_vir_addr*)&req_buf) != 0) {
				ret_val = SMC_ENOMEM;
				goto handle_err1;
			}
		}

		if(cmd->resp_buf_len >  0) {
			if(map_to_ns(cmd->resp_buf_phys, (sw_vir_addr*)&res_buf) != 0) {
				ret_val = SMC_ENOMEM;
				goto handle_err2;
			}
		}

		if(map_to_ns(cmd->meta_data_phys, (sw_vir_addr*)&meta_data) != 0) {
			ret_val = SMC_ENOMEM;
			goto handle_err2;
		}
	}
	else {
		cmd = (struct otz_smc_cmd *)params[1];
		req_buf = (void*)cmd->req_buf_phys;
		res_buf = (void*)cmd->resp_buf_phys;
		meta_data = (void*)cmd->meta_data_phys;
	}

	if(cmd->context != session_id) {
		cmd->cmd_status = OTZ_STATUS_COMPLETE;
		return SMC_EINVAL_ARG;
	}

	svc_id = ((cmd->id >> 10) & (0x3ff));
	svc_cmd_id = (cmd->id & 0x3ff);
	if(tls->process)
		ret_val = tls->process( svc_cmd_id, 
				req_buf, cmd->req_buf_len, 
				res_buf, cmd->resp_buf_len, 
				meta_data, &cmd->ret_resp_buf_len);

	if(!ret_val)
		cmd->cmd_status = OTZ_STATUS_COMPLETE;

	if(ret_val == SMC_PENDING) {
	}

handle_err2:
	if(params[2] == OTZ_CMD_TYPE_NS_TO_SECURE) {
		if(req_buf)
			unmap_from_ns((sw_vir_addr)req_buf);
		if(res_buf)
			unmap_from_ns((sw_vir_addr)res_buf);
		if(meta_data)
			unmap_from_ns((sw_vir_addr)meta_data);
	}

handle_err1:
	if(params[2] == OTZ_CMD_TYPE_NS_TO_SECURE) {
		unmap_from_ns((sw_vir_addr)cmd);
	}

ret_func:
	return ret_val;
}
