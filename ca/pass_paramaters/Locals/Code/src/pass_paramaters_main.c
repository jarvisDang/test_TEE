/*
 * Copyright (c) 2014, STMicroelectronics International N.V.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifdef USE_TRUSTONIC

//#include <string>

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <tee_client_api.h>

#include "buildTag.h"
#include "log.h"

char *_device = NULL;

#else
#include <stdio.h>
#include <string.h>
#include <limits.h>

#ifdef USER_SPACE
#include <pthread.h>
#include <unistd.h>
#endif

#include <signed_hdr.h>
#include <tee_client_api.h>
#include <tee_api_defines.h>

static const char gdevname_tz[] = "optee-tz";
char *_device = (char *)gdevname_tz;

#endif

#include <ta_pass_paramaters.h>

const TEEC_UUID pass_paramaters_user_ta_uuid = TA_PASS_PARAMATERS_UUID;
TEEC_Context teec_ctx;

TEEC_Result pass_paramaters_teec_opensession(TEEC_Session *session,
				    const TEEC_UUID *uuid, TEEC_Operation *op,
				    uint32_t *ret_orig)
{
	return TEEC_OpenSession(&teec_ctx, session, uuid,
				TEEC_LOGIN_PUBLIC, NULL, op, ret_orig);
}

TEEC_Result pass_paramaters_teec_closession(TEEC_Session *session)
{
	(void)session;
	TEEC_CloseSession(session);
	return 0;
}

static void pass_paramaters_teec_invoke(TEEC_Session *session)
{
	uint32_t ret_orig;
	uint32_t ret;
	uint8_t pass_paramaters_out[16];
	uint8_t pass_paramaters_in[16] = { 0 };
	TEEC_SharedMemory in_shm = { };
	TEEC_SharedMemory out_shm = { };
	
	TEEC_SharedMemory shm = { };
	size_t page_size = 4096;
	uint8_t *in = NULL,*out = NULL;

	TEEC_Operation op = { 0 };
	
	printf("pass_paramaters_teec_invoke  \n");
	
	{	
		// for value
		op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
						TEEC_VALUE_OUTPUT,
						TEEC_NONE,
						TEEC_NONE);
	
		op.params[0].value.a = 15;
		op.params[0].value.b = 4;
		ret = TEEC_InvokeCommand(session, CMD_TEST_VALUE, &op, &ret_orig);
		printf("value ret=0x%x. a=%d,b=%d, (a+b)=%d, (a*b)=%d\n", ret,op.params[0].value.a, op.params[0].value.b, op.params[1].value.a, op.params[1].value.b);
	}
	
	{
		// for temref
		pass_paramaters_in[0]=20;pass_paramaters_in[1]=2;
		memset(pass_paramaters_out, 0, sizeof(pass_paramaters_out));
		op.params[0].tmpref.buffer = pass_paramaters_in;
		op.params[0].tmpref.size = sizeof(pass_paramaters_in);
		op.params[1].tmpref.buffer = pass_paramaters_out;
		op.params[1].tmpref.size = sizeof(pass_paramaters_out);
		op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
										TEEC_MEMREF_TEMP_OUTPUT,
										TEEC_NONE,
										TEEC_NONE);
	
		ret = TEEC_InvokeCommand(session, CMD_TEST_TMPREF, &op, &ret_orig);
		printf("temref : ret=0x%x. a=%d,b=%d, (a+b)=%d, (a*b)=%d\n", ret, pass_paramaters_in[0],pass_paramaters_in[1], pass_paramaters_out[0],pass_paramaters_out[1]);
	}
	
	
	{
		// for memref  --registersharememory
		pass_paramaters_in[0]=30;pass_paramaters_in[1]=3;
		memset(pass_paramaters_out, 0, sizeof(pass_paramaters_out));
		in_shm.buffer = pass_paramaters_in;
		in_shm.flags = TEEC_MEM_INPUT;
		in_shm.size = sizeof(pass_paramaters_in);
		TEEC_RegisterSharedMemory(&teec_ctx, &in_shm);
	
		out_shm.buffer = pass_paramaters_out;
		out_shm.flags = TEEC_MEM_OUTPUT;
		out_shm.size = sizeof(pass_paramaters_out);
		TEEC_RegisterSharedMemory(&teec_ctx, &out_shm);
		
		op.params[0].memref.parent = &in_shm;
		op.params[0].memref.size = sizeof(pass_paramaters_in);
		op.params[0].memref.offset = 0;
		op.params[1].memref.parent = &out_shm;
		op.params[1].memref.size = sizeof(pass_paramaters_out);
		op.params[1].memref.offset = 0;
	
		op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_INPUT,
										TEEC_MEMREF_PARTIAL_OUTPUT,
										TEEC_NONE,
										TEEC_NONE);
	
		ret = TEEC_InvokeCommand(session, CMD_TEST_MEMREF, &op, &ret_orig);
		printf("memref:registersharememory ret=0x%x. a=%d,b=%d, (a+b)=%d, (a*b)=%d\n", ret, pass_paramaters_in[0],pass_paramaters_in[1], pass_paramaters_out[0],pass_paramaters_out[1]);
		
		TEEC_ReleaseSharedMemory(&in_shm);
		TEEC_ReleaseSharedMemory(&out_shm);
	}
	
	{
		// for memref  --AllocateSharedMemory
		shm.size = 2 * page_size;
		shm.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
		TEEC_AllocateSharedMemory(&teec_ctx, &shm);
		
		in = (uint8_t *)shm.buffer;
		out = (uint8_t *)shm.buffer + page_size;
		in[0]=33;in[1]=4;
		memset(out, 0, page_size);

		op.params[0].memref.parent = &shm;
		op.params[0].memref.size = sizeof(pass_paramaters_in);
		op.params[0].memref.offset = 0;
		op.params[1].memref.parent = &shm;
		op.params[1].memref.size = sizeof(pass_paramaters_out);
		op.params[1].memref.offset = page_size;
	
		op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_INPUT,
										TEEC_MEMREF_PARTIAL_OUTPUT,
										TEEC_NONE,
										TEEC_NONE);
	
		ret = TEEC_InvokeCommand(session, CMD_TEST_MEMREF, &op, &ret_orig);
		printf("memref:AllocateSharedMemory ret=0x%x. a=%d,b=%d, (a+b)=%d, (a*b)=%d\n", ret, in[0],in[1], out[0],out[1]);
		
		TEEC_ReleaseSharedMemory(&in_shm);
		TEEC_ReleaseSharedMemory(&out_shm);
	}

}

void pass_paramaters_do_handler()
{
#ifndef USE_TRUSTONIC
	TEEC_Session session = { .ctx = 0 };
#else
	TEEC_Session session;
#endif
	
	uint32_t ret_orig;

	pass_paramaters_teec_opensession(&session, &pass_paramaters_user_ta_uuid, NULL, &ret_orig);

	pass_paramaters_teec_invoke(&session);

	pass_paramaters_teec_closession(&session);
}

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;

	printf("\npass_paramaters CA enter device=[%s] 11111111111\n", _device);

	TEEC_InitializeContext(_device, &teec_ctx); //todo

	pass_paramaters_do_handler();

	TEEC_FinalizeContext(&teec_ctx);

	printf("pass_paramaters CA exit!\n");
	return 0;
}

