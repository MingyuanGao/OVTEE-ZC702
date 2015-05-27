#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "otz_id.h"
#include "otz_tee_client_api.h"

int main(int argc, char* argv[])
{   
	// CPU Affinity
	int cpu = 0;
	cpu_set_t mask; 
	int ret; 
	if (argc > 1 && (strcmp(argv[1], "cpu") == 0)) {
		cpu = atoi(argv[1]);
	}
	CPU_ZERO(&mask); 
	CPU_SET((int)cpu, &mask); 
	ret = sched_setaffinity(0, sizeof mask, &mask);


	TEEC_Context context;
	TEEC_Session session;
	TEEC_Operation operation;
	TEEC_Result result;
	TEEC_UUID svc_id = OTZ_SVC_ECHO;

	result = TEEC_InitializeContext(
			NULL,
			&context);

	if(result != TEEC_SUCCESS) {
		return 1;	
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
		TEEC_FinalizeContext(&context);
		return 1;
	}

	printf("session id 0x%x\n", session.session_id);

	operation.paramTypes = TEEC_PARAM_TYPES(
			TEEC_VALUE_INPUT,
			TEEC_MEMREF_TEMP_INPUT,
			TEEC_MEMREF_TEMP_OUTPUT,
			TEEC_NONE);


	char testData[256];
	char resultData[256];
	
	uint32_t len;
	strcpy(testData, 
			"test global platform client api: testing temp memory reference");
	len = strlen(
			"test global platform client api: testing temp memory reference") + 1;

	operation.params[0].value.a = len;
	operation.params[0].value.b = TEEC_VALUE_UNDEF;

	operation.params[1].tmpref.buffer = testData;
	operation.params[1].tmpref.size = len;

	operation.params[2].tmpref.buffer = resultData;
	operation.params[2].tmpref.size = len;

	result = TEEC_InvokeCommand(
			&session,
			OTZ_ECHO_CMD_ID_SEND_CMD,
			&operation,
			NULL);
	if (result != TEEC_SUCCESS)
	{
	 	TEEC_CloseSession(&session);
		TEEC_FinalizeContext(&context);
		return 1;
	}

	printf("TEEC output buffer %s \n", (char*)resultData);
	
	return TEEC_SUCCESS;
}
