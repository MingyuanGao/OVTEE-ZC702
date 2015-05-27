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
 * Crypto task implementation
 */

#include <sw_debug.h>
#include <sw_string_functions.h>
#include <sw_mem_functions.h>
#include <otz_id.h>
#include <otz_common.h>
#include <task_control.h>
#include <crypto_task.h>
#include <otz_app_eg.h>
#include "otz_tee_crypto_api.h"
#include "sw_types.h"
#include <sw_user_heap.h>
#include <sw_user_app_api.h>

extern void OpenSSL_add_all_ciphers(void);
/**
 * @brief 
 *
 * @param input_buf
 * @param input_len
 * @param output_buf
 * @param output_len
 *
 * @return 
 */
extern int otzone_rc4_algorithm(char *input_buf, int input_len,
		char *output_buf, int *output_len);

/**
 * @brief 
 */
void process_otz_crypto_load_libs()
{
	OpenSSL_add_all_ciphers();
	OpenSSL_add_all_digests();
}

/**
 * @brief 
 */
void process_otz_crypto_unload_libs()
{
	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
}


/**
 * @brief 
 *
 * @param req_buf
 * @param req_buf_len
 * @param res_buf
 * @param res_buf_len
 * @param meta_data
 * @param ret_res_buf_len
 * @param svc_cmd_id
 *
 * @return 
 */
int process_otz_crypto_digest_cmd(void *req_buf, sw_uint req_buf_len,
		void *res_buf, sw_uint res_buf_len,
		struct otzc_encode_meta *meta_data,
		sw_uint *ret_res_buf_len,sw_uint svc_cmd_id)
{
	int ret_val = SMC_SUCCESS;
	crypto_data_t *crypto_data = NULL;
	sw_short_int *out_buf;
	sw_int offset = 0,  mapped = 0, out_len =0, type = 0, pos = 0;
	sw_uint cmd_id = 0;

	crypto_data = (crypto_data_t*)sw_malloc(sizeof(crypto_data_t));
	if(!crypto_data) {
		sw_seterrno(SW_ENOMEM);
		ret_val = SMC_ENOMEM;
		goto crypto_cmd_ret;
	}

	while (offset <= req_buf_len) {
		if(decode_data(req_buf, meta_data, &type, &offset, &pos, &mapped, 
					(void**)&out_buf, &out_len)) {
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto crypto_cmd_ret;
		}
		if((type != OTZ_ENC_ARRAY) && (type != OTZ_MEM_REF)){ 
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto crypto_cmd_ret;
		}
		if(out_len < CRYPT_BUF_LEN) {
			sw_memcpy(crypto_data->data, out_buf, out_len);
		} else {
			sw_seterrno(SW_ENOMEM);
			ret_val = SMC_ENOMEM;
			goto crypto_cmd_ret;
		}
		if(type == OTZ_MEM_REF)
			free_decoded_data(out_buf);
		break;
	}
	crypto_data->len = CRYPT_BUF_LEN;
	switch(svc_cmd_id) {
		case OTZ_CRYPT_CMD_ID_MD5:
			cmd_id = TEE_ALG_MD5;
		case OTZ_CRYPT_CMD_ID_SHA1:
			if(cmd_id == 0) {
				cmd_id = TEE_ALG_SHA1;
			}
		case OTZ_CRYPT_CMD_ID_SHA224:
			if(cmd_id == 0) {
				cmd_id = TEE_ALG_SHA224;
			}
		case OTZ_CRYPT_CMD_ID_SHA256:
			if(cmd_id == 0) {
				cmd_id = TEE_ALG_SHA256;
			}
		case OTZ_CRYPT_CMD_ID_SHA384:
			if(cmd_id == 0) {
				cmd_id = TEE_ALG_SHA384;
			}
		case OTZ_CRYPT_CMD_ID_SHA512:
			if(cmd_id == 0) {
				cmd_id = TEE_ALG_SHA512;
			}
			if(test_digest_api(crypto_data->data,out_len,
						crypto_data->response,&(crypto_data->len),
						cmd_id) != TEEC_SUCCESS) {
				sw_memcpy(crypto_data->response,crypto_data->data,out_len);
				crypto_data->len = out_len;
			}
			break;
		default:
			break;
	}
	offset = 0, pos = OTZ_MAX_REQ_PARAMS;
	while (offset <= res_buf_len) {
		if(decode_data(res_buf, meta_data, &type, &offset, &pos, &mapped, 
					(void**)&out_buf, &out_len)) {
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto crypto_cmd_ret;
		}
		if((type != OTZ_ENC_ARRAY) && (type != OTZ_MEM_REF)){ 
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto crypto_cmd_ret;
		}
		if(out_len >= crypto_data->len) {
			sw_memcpy(out_buf,crypto_data->response, crypto_data->len);
			if(update_response_len(meta_data, pos, crypto_data->len)) {
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto crypto_cmd_ret;
			}
		} else {
			sw_seterrno(SW_ENOMEM);
			ret_val = SMC_ENOMEM;
			goto crypto_cmd_ret;
		}
		if(type == OTZ_MEM_REF)
			free_decoded_data(out_buf);
		break;
	}
	*ret_res_buf_len = crypto_data->len;

crypto_cmd_ret:
	if(crypto_data)	
		sw_free(crypto_data);
	return ret_val;
}


/**
 * @brief 
 *
 * @param req_buf
 * @param req_buf_len
 * @param res_buf
 * @param res_buf_len
 * @param meta_data
 * @param ret_res_buf_len
 * @param svc_cmd_id
 *
 * @return 
 */
int process_otz_crypto_hmac_cmd(void *req_buf, sw_uint req_buf_len,
		void *res_buf, sw_uint res_buf_len,
		struct otzc_encode_meta *meta_data,
		sw_uint *ret_res_buf_len,sw_uint svc_cmd_id)
{
	int ret_val = SMC_SUCCESS;
	crypto_data_t *crypto_data = NULL;
	sw_short_int *out_buf;
	sw_short_int key_buf[HMAC_KEY_LEN];
	sw_ushort key_len = 0,loop_cntr = 0;
	sw_int offset = 0, pos = 0, mapped = 0, type =0, out_len =0;
	sw_uint cmd_id = 0;

	crypto_data = (crypto_data_t*)sw_malloc(sizeof(crypto_data_t));
	if(!crypto_data) {
		sw_seterrno(SW_ENOMEM);
		ret_val = SMC_ENOMEM;
		goto hmac_cmd_ret;
	}

	while (offset <= req_buf_len) {
		if(decode_data(req_buf, meta_data, &type, &offset, &pos, &mapped,
					(void**)&out_buf, &out_len)) {
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto hmac_cmd_ret;

		}
		if((type != OTZ_ENC_ARRAY) && (type != OTZ_MEM_REF)){
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto hmac_cmd_ret;
		}
		if(out_len <= HMAC_KEY_LEN) {
			sw_memcpy(key_buf, out_buf, out_len);
			key_len = out_len;
		} else {
			sw_seterrno(SW_ENOMEM);
			ret_val = SMC_ENOMEM;
			goto hmac_cmd_ret;
		}
		if(type == OTZ_MEM_REF)
			free_decoded_data(out_buf);
		if(decode_data(req_buf, meta_data, &type, &offset, &pos, &mapped,
					(void**)&out_buf, &out_len)) {
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto hmac_cmd_ret;
		}
		if((type != OTZ_ENC_ARRAY) && (type != OTZ_MEM_REF)){
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto hmac_cmd_ret;
		}
		if(out_len < CRYPT_BUF_LEN) {
			sw_memcpy(crypto_data->data, out_buf, out_len);
		} else {
			sw_seterrno(SW_ENOMEM);
			ret_val = SMC_ENOMEM;
			goto hmac_cmd_ret;
		}
		if(type == OTZ_MEM_REF)
			free_decoded_data(out_buf);
		break;
	}
	crypto_data->len = CRYPT_BUF_LEN;
	switch(svc_cmd_id) {
		case OTZ_CRYPT_CMD_ID_HMAC_MD5:
			cmd_id = TEE_ALG_HMAC_MD5;
		case OTZ_CRYPT_CMD_ID_HMAC_SHA1:
			if(cmd_id == 0) {
				cmd_id = TEE_ALG_HMAC_SHA1;
			}
		case OTZ_CRYPT_CMD_ID_HMAC_SHA224:
			if(cmd_id == 0) {
				cmd_id = TEE_ALG_HMAC_SHA224;
			}
		case OTZ_CRYPT_CMD_ID_HMAC_SHA256:
			if(cmd_id == 0) {
				cmd_id = TEE_ALG_HMAC_SHA256;
			}
		case OTZ_CRYPT_CMD_ID_HMAC_SHA384:
			if(cmd_id == 0) {
				cmd_id = TEE_ALG_HMAC_SHA384;
			}
		case OTZ_CRYPT_CMD_ID_HMAC_SHA512:
			if(cmd_id == 0) {
				cmd_id = TEE_ALG_HMAC_SHA512;
			}
			if(test_hmac_api(crypto_data->data,out_len,key_buf,key_len,
						crypto_data->response,&(crypto_data->len),
						cmd_id) != TEEC_SUCCESS) {
				sw_memcpy(crypto_data->response,crypto_data->data,out_len);
				crypto_data->len = out_len;
			}
			if(compare_hmac_values(crypto_data->data,out_len,key_buf,key_len,
						crypto_data->response,&(crypto_data->len),
						cmd_id) != TEEC_SUCCESS) {
				sw_memcpy(crypto_data->response,crypto_data->data,out_len);
				crypto_data->len = out_len;
			}
			break;
		default:
			break;
	}
	offset = 0, pos = OTZ_MAX_REQ_PARAMS;
	while (offset <= res_buf_len) {
		if(decode_data(res_buf, meta_data, &type, &offset, &pos, &mapped,
					(void**)&out_buf, &out_len)) {
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto hmac_cmd_ret;
		}
		if((type != OTZ_ENC_ARRAY) && (type != OTZ_MEM_REF)){
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto hmac_cmd_ret;
		}
		if(out_len >= crypto_data->len) {
			sw_memcpy(out_buf,crypto_data->response, crypto_data->len);
			if(update_response_len(meta_data, pos, crypto_data->len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto hmac_cmd_ret;
			}
		} else {
			sw_seterrno(SW_ENOMEM);
			ret_val = SMC_ENOMEM;
			goto hmac_cmd_ret;

		}
		if(type == OTZ_MEM_REF)
			free_decoded_data(out_buf);
		break;
	}
	*ret_res_buf_len = crypto_data->len;
hmac_cmd_ret:
	if(crypto_data)	
		sw_free(crypto_data);
	return ret_val;
}


/**
 * @brief 
 *
 * @param req_buf
 * @param req_buf_len
 * @param res_buf
 * @param res_buf_len
 * @param meta_data
 * @param ret_res_buf_len
 * @param svc_cmd_id
 *
 * @return 
 */
int process_otz_crypto_cipher_cmd(void *req_buf, sw_uint req_buf_len,
		void *res_buf, sw_uint res_buf_len,
		struct otzc_encode_meta *meta_data,
		sw_uint *ret_res_buf_len,sw_uint svc_cmd_id)
{
	int ret_val = SMC_SUCCESS;
	crypto_data_t *crypto_data = NULL;
	sw_short_int *out_buf,*local_init_vector = NULL;
	sw_short_int *key_buf = NULL, *init_vector = NULL;
	sw_uint cipher_choice = 0;
	sw_ushort key_len = 0,init_vector_len=0;
	sw_int offset = 0, pos = 0, mapped = 0, type =0, out_len =0;
	sw_uint cmd_id = 0,init_vector_present = 0;

	crypto_data = (crypto_data_t*)sw_malloc(sizeof(crypto_data_t));
	if(!crypto_data) {
		sw_seterrno(SW_ENOMEM);
		ret_val = SMC_ENOMEM;
		goto cipher_cmd_ret;
	}

	key_buf = (sw_short_int*)sw_malloc(CRYPT_BUF_LEN);
	if(!key_buf) {
		sw_seterrno(SW_ENOMEM);
		ret_val = SMC_ENOMEM;
		goto cipher_cmd_ret;
	}

	init_vector = (sw_short_int*)sw_malloc(CRYPT_BUF_LEN);
	if(!init_vector) {
		sw_seterrno(SW_ENOMEM);
		ret_val = SMC_ENOMEM;
		goto cipher_cmd_ret;
	}

	while (offset <= req_buf_len) {
		if(decode_data(req_buf, meta_data, &type, &offset, &pos, &mapped,
					(void**)&out_buf, &out_len)) {
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto cipher_cmd_ret;
		}
		if((type != OTZ_ENC_ARRAY) && (type != OTZ_MEM_REF)){
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto cipher_cmd_ret;
		}
		cipher_choice = *((sw_uint*)out_buf);
		if(type == OTZ_MEM_REF)
			free_decoded_data(out_buf);
		if(decode_data(req_buf, meta_data, &type, &offset, &pos, &mapped,
					(void**)&out_buf, &out_len)) {
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto cipher_cmd_ret;
		}
		if((type != OTZ_ENC_ARRAY) && (type != OTZ_MEM_REF)){
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto cipher_cmd_ret;
		}
		init_vector_present = *((sw_uint*)out_buf);
		if(type == OTZ_MEM_REF)
			free_decoded_data(out_buf);
		if(init_vector_present == 1) {
			if(decode_data(req_buf, meta_data, &type, &offset, &pos, &mapped,
						(void**)&out_buf, &out_len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto cipher_cmd_ret;
			}
			if((type != OTZ_ENC_ARRAY) && (type != OTZ_MEM_REF)){
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto cipher_cmd_ret;
			}
			if(out_len <= CRYPT_BUF_LEN) {
				sw_memcpy(init_vector, out_buf, out_len);
				init_vector_len = out_len;
			} else {
				sw_seterrno(SW_ENOMEM);
				ret_val = SMC_ENOMEM;
				goto cipher_cmd_ret;
			}
			if(type == OTZ_MEM_REF)
				free_decoded_data(out_buf);
			local_init_vector = &(init_vector[0]);
		} else {
			init_vector_len = 0;
			local_init_vector = NULL;
		}
		if(decode_data(req_buf, meta_data, &type, &offset, &pos, &mapped,
					(void**)&out_buf, &out_len)) {
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto cipher_cmd_ret;

		}
		if((type != OTZ_ENC_ARRAY) && (type != OTZ_MEM_REF)){
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto cipher_cmd_ret;
		}
		if(out_len <= CRYPT_BUF_LEN) {
			sw_memcpy(key_buf, out_buf, out_len);
			key_len = out_len;
		} else {
			sw_seterrno(SW_ENOMEM);
			ret_val = SMC_ENOMEM;
			goto cipher_cmd_ret;
		}
		if(type == OTZ_MEM_REF)
			free_decoded_data(out_buf);
		if(decode_data(req_buf, meta_data, &type, &offset, &pos, &mapped,
					(void**)&out_buf, &out_len)) {
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto cipher_cmd_ret;
		}
		if((type != OTZ_ENC_ARRAY) && (type != OTZ_MEM_REF)){
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto cipher_cmd_ret;
		}
		if(out_len < CRYPT_BUF_LEN) {
			sw_memcpy(crypto_data->data, out_buf, out_len);
		} else {
			sw_seterrno(SW_ENOMEM);
			ret_val = SMC_ENOMEM;
			goto cipher_cmd_ret;
		}
		if(type == OTZ_MEM_REF)
			free_decoded_data(out_buf);
		break;
	}
  
	crypto_data->len = CRYPT_BUF_LEN;
	switch(svc_cmd_id) {
		case OTZ_CRYPT_CMD_ID_CIPHER_AES_128_CBC:
			cmd_id = TEE_ALG_AES_CBC_NOPAD;
		case OTZ_CRYPT_CMD_ID_CIPHER_AES_128_ECB:
			if(cmd_id == 0) {
				cmd_id = TEE_ALG_AES_ECB_NOPAD;
			}
		case OTZ_CRYPT_CMD_ID_CIPHER_AES_128_CTR:
			if(cmd_id == 0) {
				cmd_id = TEE_ALG_AES_CTR;
			}
		case OTZ_CRYPT_CMD_ID_CIPHER_AES_128_XTS:
			if(cmd_id == 0) {
				cmd_id = TEE_ALG_AES_XTS;
			}
		case OTZ_CRYPT_CMD_ID_CIPHER_DES_ECB:
			if(cmd_id == 0) {
				cmd_id = TEE_ALG_DES_ECB_NOPAD;
			}
		case OTZ_CRYPT_CMD_ID_CIPHER_DES_CBC:
			if(cmd_id == 0) {
				cmd_id = TEE_ALG_DES_CBC_NOPAD;
			}
		case OTZ_CRYPT_CMD_ID_CIPHER_DES3_ECB:
			if(cmd_id == 0) {
				cmd_id = TEE_ALG_DES3_ECB_NOPAD;
			}
		case OTZ_CRYPT_CMD_ID_CIPHER_DES3_CBC:
			if(cmd_id == 0) {
				cmd_id = TEE_ALG_DES3_CBC_NOPAD;
			}
			if(test_cipher_api(crypto_data->data,out_len,local_init_vector,
						init_vector_len,key_buf,key_len,
						crypto_data->response,&(crypto_data->len),
						cmd_id,cipher_choice) != TEEC_SUCCESS) {
				sw_memcpy(crypto_data->response,crypto_data->data,out_len);
				crypto_data->len = out_len;
			}
			break;
		default:
			break;
	}
	offset = 0, pos = OTZ_MAX_REQ_PARAMS;
	while (offset <= res_buf_len) {
		if(decode_data(res_buf, meta_data, &type, &offset, &pos, &mapped,
					(void**)&out_buf, &out_len)) {
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto cipher_cmd_ret;
		}
		if((type != OTZ_ENC_ARRAY) && (type != OTZ_MEM_REF)){
			sw_seterrno(SW_EINVAL);
			ret_val = SMC_EINVAL_ARG;
			goto cipher_cmd_ret;
		}
		if(out_len >= crypto_data->len) {
			sw_memcpy(out_buf,crypto_data->response, crypto_data->len);
			if(update_response_len(meta_data, pos, crypto_data->len)) {
				sw_seterrno(SW_EINVAL);
				ret_val = SMC_EINVAL_ARG;
				goto cipher_cmd_ret;
			}
		} else {
			sw_seterrno(SW_ENOMEM);
			ret_val = SMC_ENOMEM;
			goto cipher_cmd_ret;
		}
		if(type == OTZ_MEM_REF)
			free_decoded_data(out_buf);
		break;
	}
	*ret_res_buf_len = crypto_data->len;

cipher_cmd_ret:
	if(init_vector)	
		sw_free(init_vector);

	if(key_buf)	
		sw_free(key_buf);

	if(crypto_data)	
		sw_free(crypto_data);

	return ret_val;

}


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
		struct otzc_encode_meta *meta_data, sw_uint *ret_res_buf_len)
{
	int ret_val = SMC_ERROR,time_interval;
	if((req_buf_len == 0) || (res_buf_len == 0) ) {
		return(0);
	}
#ifdef PRINT_TIME	
	sw_timeval open_time;
	open_time.tval64=read_timestamp();
#endif	
	switch (svc_cmd_id) {
		case OTZ_CRYPT_CMD_ID_LOAD_LIBS:
			process_otz_crypto_load_libs();
			break;
		case OTZ_CRYPT_CMD_ID_UNLOAD_LIBS:
			process_otz_crypto_unload_libs();
			break;
		case OTZ_CRYPT_CMD_ID_ENCRYPT:
		case OTZ_CRYPT_CMD_ID_DECRYPT:
			break;
		case OTZ_CRYPT_CMD_ID_MD5:
		case OTZ_CRYPT_CMD_ID_SHA1:
		case OTZ_CRYPT_CMD_ID_SHA224:
		case OTZ_CRYPT_CMD_ID_SHA256:
		case OTZ_CRYPT_CMD_ID_SHA384:
		case OTZ_CRYPT_CMD_ID_SHA512:
			ret_val = process_otz_crypto_digest_cmd(req_buf,req_buf_len,
					resp_buf,res_buf_len,meta_data,
					ret_res_buf_len,svc_cmd_id);
			break;
		case OTZ_CRYPT_CMD_ID_HMAC_MD5:
		case OTZ_CRYPT_CMD_ID_HMAC_SHA1:
		case OTZ_CRYPT_CMD_ID_HMAC_SHA224:
		case OTZ_CRYPT_CMD_ID_HMAC_SHA256:
		case OTZ_CRYPT_CMD_ID_HMAC_SHA384:
		case OTZ_CRYPT_CMD_ID_HMAC_SHA512:
			ret_val = process_otz_crypto_hmac_cmd(req_buf,req_buf_len,
					resp_buf,res_buf_len,meta_data,
					ret_res_buf_len,svc_cmd_id);
			break;
		case OTZ_CRYPT_CMD_ID_CIPHER_AES_128_CBC:
		case OTZ_CRYPT_CMD_ID_CIPHER_AES_128_ECB:
		case OTZ_CRYPT_CMD_ID_CIPHER_AES_128_CTR:
		case OTZ_CRYPT_CMD_ID_CIPHER_AES_128_XTS:
		case OTZ_CRYPT_CMD_ID_CIPHER_DES_ECB:
		case OTZ_CRYPT_CMD_ID_CIPHER_DES_CBC:
		case OTZ_CRYPT_CMD_ID_CIPHER_DES3_ECB:
		case OTZ_CRYPT_CMD_ID_CIPHER_DES3_CBC:
			ret_val = process_otz_crypto_cipher_cmd(req_buf,req_buf_len,
					resp_buf,res_buf_len,meta_data,
					ret_res_buf_len,svc_cmd_id);
			break;
		default:
			ret_val = SMC_EOPNOTSUPP;
			break;
	}
#ifdef PRINT_TIME
	sw_timeval close_time;
	close_time.tval64=read_timestamp();
	sw_timeval clock_cycle;
	clock_cycle.tval64=subtract_time((const sw_timeval*)&close_time,
								(const sw_timeval*)&open_time);
	time_interval=clock_cycle.tval.nsec/1000000;
	sw_printk("time_interval %d milliseconds\n",time_interval);
#endif
	
	return ret_val;
}

/**
 * @brief Crypto task entry point
 *
 * This function implements the processing of crypto commands.
 *
 * @param task_id: task identifier
 * @param tls: Pointer to task local storage
 */
void crypto_task(int task_id, sw_tls* tls)
{
	task_init(task_id, tls);
	tls->ret_val = process_otzapi(task_id, tls);
	task_exit(task_id, tls);
	while(1);
}

