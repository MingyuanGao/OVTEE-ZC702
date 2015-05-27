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

 /* User space application to test the LibC functionality  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <otz_id.h>
#include <otz_tee_client_api.h>

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

/**
 * @brief The function to test mutexes. We do not need any parameters or return
 * values here.
 *
 **/
int test_otz_mutex()
{
	TEEC_Context context;
	TEEC_Session session;
	TEEC_Operation operation;
	TEEC_SharedMemory sharedMem;	
	TEEC_Result result;
	TEEC_UUID svc_id = OTZ_SVC_TEST_SUITE_USER;
	int len = 0;


	result = TEEC_InitializeContext(
			NULL,
			&context);

	if(result != TEEC_SUCCESS) {
		goto cleanup_1;
	}

	result = TEEC_OpenSession(
			&context,
			&session,
			&svc_id,
			TEEC_LOGIN_PUBLIC,
			NULL,
			NULL,
			NULL);

	if(result != TEEC_SUCCESS) {
		goto cleanup_2;
	}

	printf("session id 0x%x\n", session.session_id);

	sharedMem.size = 1024 * 2;
	sharedMem.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT ;

	result = TEEC_AllocateSharedMemory(
			&context,
			&sharedMem);	

	strcpy(sharedMem.buffer, 
			"mutex testing");
	len = strlen(
			"mutex testing") + 1;	

	operation.paramTypes = TEEC_PARAM_TYPES(
			TEEC_VALUE_INPUT,
			TEEC_MEMREF_PARTIAL_INPUT,
			TEEC_MEMREF_PARTIAL_OUTPUT,			
			TEEC_NONE);

	operation.started = 1;
	operation.params[0].value.a = len;
	operation.params[0].value.b = TEEC_VALUE_UNDEF;

	operation.params[1].memref.parent = &sharedMem;
	operation.params[1].memref.offset = 0;
	operation.params[1].memref.size = len;

	operation.params[2].memref.parent = &sharedMem;
	operation.params[2].memref.offset = 1024;
	operation.params[2].memref.size = len;

	result = TEEC_InvokeCommand(
			&session,
			OTZ_TEST_SUITE_CMD_ID_MUTEX,
			&operation,
			NULL);
	if (result != TEEC_SUCCESS)
	{
		goto cleanup_3;
	}

	printf("command success\n");
cleanup_3:
	TEEC_ReleaseSharedMemory(&sharedMem);
	TEEC_CloseSession(&session);
cleanup_2:
	TEEC_FinalizeContext(&context);
cleanup_1:
	return 0;
}

int test_shm(void)
{
	TEEC_Context context;
	TEEC_Session session;
	TEEC_Operation operation;

	TEEC_Result result;

	TEEC_UUID svc_id = OTZ_SVC_TEST_SUITE_USER;


	result = TEEC_InitializeContext(
			NULL,
			&context);

	if(result != TEEC_SUCCESS) {
		goto cleanup_1;
	}

	result = TEEC_OpenSession(
			&context,
			&session,
			&svc_id,
			TEEC_LOGIN_PUBLIC,
			NULL,
			NULL,
			NULL);

	if(result != TEEC_SUCCESS) {
		goto cleanup_2;
	}

	printf("session id 0x%x\n", session.session_id);

	operation.paramTypes = TEEC_PARAM_TYPES(
			TEEC_NONE,
			TEEC_NONE,
			TEEC_NONE,
			TEEC_NONE);

	operation.started = 1;

	result = TEEC_InvokeCommand(
			&session,
			OTZ_TEST_SUITE_CMD_ID_SHM,
			&operation,
			NULL);
	if (result != TEEC_SUCCESS)
	{
		goto cleanup_3;
	}

	printf("command success\n");
cleanup_3:
	TEEC_CloseSession(&session);
cleanup_2:
	TEEC_FinalizeContext(&context);
cleanup_1:
	return 0;
}

int main(){
	printf("Creating task for testing otz mutexes \n");
	test_otz_mutex();
	printf("Mutex testing finished \n"); 
	printf("Creating task for testing shared memory\n");
	test_shm();
	printf("Shared memory testing finished \n"); 
	return 0;
}
