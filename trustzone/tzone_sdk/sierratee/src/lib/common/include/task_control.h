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
 * Helper function to process the service commands
 */

#ifndef __OTZ_APP_TASK_CONTROL_H__
#define __OTZ_APP_TASK_CONTROL_H__

#include <sw_types.h>
#include <tls.h>

/**
 * @brief Encode command structure
 */
struct otz_secure_encode_cmd {
	unsigned int len;
	void* data;
	int   param_type;
};

/**
 * @brief Encode context
 */
struct otz_secure_encode_ctx {
	void *req_addr;
	unsigned int req_len;
	void *res_addr;
	unsigned int res_len;

	sw_uint  enc_req_offset;
	sw_uint  enc_res_offset;
	sw_uint  enc_req_pos;
	sw_uint  enc_res_pos;
};

/**
 * @brief Helper function to decode the data which got passed from 
 * non-secure world
 *
 * This helper function decodes the data which got encoded from non-secure 
 * application.
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
		int *mapped, void **out_data, int *out_len);

/*
 * @brief Free's mapped ns memory during data decoding
 * param : ns_addr - mapped ns addresss
 */
int free_decoded_data(sw_uint* ns_addr);

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
int update_response_len(struct otzc_encode_meta *meta_data, int pos, int len);


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
		int encode_type);

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
		struct otzc_encode_meta *meta_data);

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
int process_otzapi(int session_id, sw_tls* tls);

#endif /* __OTZ_APP_TASK_CONTROL_H__ */
