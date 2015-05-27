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

/* 	Test application for Trustzone API specification Mutex and
	Asynchronous notification functionality */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "otz_id.h"
#include "otz_api.h"
#include <otz_app_eg.h>


static const char* otz_service_errorlist[] =
{       
	"Service Success",
	"Service Pending",
	"Service Interrupted",
	"Service Error",
	"Service - Invalid Argument",
	"Service -Invalid Address",
	"Service No Support",
	"Service No Memory",
};

char* otz_strerror(int index) 
{                           
	return (char*)otz_service_errorlist[index]; 
}

#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
/**
 * @brief The function to test notification from secure world. 
 *
 **/
int test_notification()
{
	echo_data_t echo_data;
	otz_device_t device_otz;
	otz_session_t session_otz;
	otz_operation_t operation_otz;
	otz_return_t ret=0, service_ret;
	unsigned int out_data_len;
	char *out_data;

	device_otz.ui_state = OTZ_STATE_UNDEFINED;
	ret = otz_device_open("/dev/otz_client", (void*)O_RDWR, &device_otz);
	if (ret != OTZ_SUCCESS){
		perror("device open failed\n");
		return 0;
	}
	session_otz.ui_state = OTZ_STATE_UNDEFINED;
	operation_otz.ui_state = OTZ_STATE_UNDEFINED;
	ret = otz_operation_prepare_open(&device_otz, OTZ_SVC_TEST_SUITE_KERNEL,
			NULL, NULL, &session_otz, &operation_otz);
	if(ret != OTZ_SUCCESS) {
		goto end_func;
	}
	/* Call tz_operation_perform to open session */
	ret = otz_operation_perform(&operation_otz ,&service_ret);
	if(ret != OTZ_SUCCESS){
		if(ret == OTZ_ERROR_SERVICE)
			printf("%s \n",otz_strerror(service_ret));
		else
			perror("session open  failed\n");
		session_otz.ui_state = OTZ_STATE_UNDEFINED;
	}
	otz_operation_release(&operation_otz);
	if(ret != OTZ_SUCCESS){
		goto end_func;
	}
	operation_otz.ui_state = OTZ_STATE_UNDEFINED;
	ret = otz_operation_prepare_invoke(&session_otz,
			OTZ_TEST_SUITE_CMD_ID_ASYNC, NULL,&operation_otz);
	if (ret != OTZ_SUCCESS) {
		goto handle_error_2;
	}
	memcpy(echo_data.data,"test notification", strlen("test notification")+1);
	echo_data.len = strlen("test notification")+1;
	otz_encode_uint32(&operation_otz, (void*)&echo_data.len, 
			OTZ_PARAM_IN);

	if(operation_otz.enc_dec.enc_error_state != OTZ_SUCCESS) {
		perror("encode failed \n");
		goto handle_error_1;
	}

	otz_encode_array(&operation_otz, echo_data.data,
			echo_data.len, OTZ_PARAM_IN);
	if(operation_otz.enc_dec.enc_error_state != OTZ_SUCCESS) {
		perror("encode failed \n");
		goto handle_error_1;
	}

	otz_encode_array(&operation_otz, echo_data.response,
			echo_data.len, OTZ_PARAM_OUT);
	if(operation_otz.enc_dec.enc_error_state != OTZ_SUCCESS) {
		goto handle_error_1;
	}
	ret = otz_operation_perform(&operation_otz, &service_ret);
	if(ret != OTZ_SUCCESS) {
		if(ret == OTZ_ERROR_SERVICE)
			printf("%s \n",otz_strerror(service_ret));
		else
			perror("session open  failed\n");
		goto handle_error_1;
	}
	out_data = otz_decode_array_space(&operation_otz,(uint32_t *)&out_data_len);
	if(operation_otz.enc_dec.enc_error_state != OTZ_SUCCESS) {
		perror("decode error\n");
		goto handle_error_1;
	}

handle_error_1:
	otz_operation_release(&operation_otz);
handle_error_2:
	operation_otz.ui_state = OTZ_STATE_UNDEFINED;
	ret = otz_operation_prepare_close(&session_otz, &operation_otz);
	if(ret != OTZ_SUCCESS) {
		perror("operation prepare close failed \n");
	}
	ret = otz_operation_perform(&operation_otz, &service_ret);
	if(ret != OTZ_SUCCESS) {
		if(ret == OTZ_ERROR_SERVICE)
			printf("%s \n",otz_strerror(service_ret));
		else
			perror("operation close failed \n");
		operation_otz.ui_state = OTZ_STATE_INVALID;
	}
	otz_operation_release(&operation_otz);
end_func:
	ret = otz_device_close(&device_otz);
	if (ret != OTZ_SUCCESS){
		printf("device close failed\n");
	} else{
		printf("device close successful\n");
	}
	return(0);
}
#endif

/**
 * @brief 
 *
 * @return 
 */
int main()
{
#ifdef OTZONE_ASYNC_NOTIFY_SUPPORT
	printf("Creating task for testing secure kernel notification feature\n");
	test_notification();
	printf("Notification testing finished \n");
#endif
	return(0);
}
