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

#include <ta_hmac_demo.h>

const TEEC_UUID hmac_demo_user_ta_uuid = TA_HMAC_DEMO_UUID;
TEEC_Context teec_ctx;

TEEC_Result hmac_demo_teec_opensession(TEEC_Session *session,
				    const TEEC_UUID *uuid, TEEC_Operation *op,
				    uint32_t *ret_orig)
{
	return TEEC_OpenSession(&teec_ctx, session, uuid,
				TEEC_LOGIN_PUBLIC, NULL, op, ret_orig);
}

TEEC_Result hmac_demo_teec_closession(TEEC_Session *session)
{
	(void)session;
	TEEC_CloseSession(session);
	return 0;
}

static void hmac_demo_teec_invoke(TEEC_Session *session)
{
	uint32_t ret_orig;
	uint32_t ret;
	uint8_t hmac_demo_out[16];
	uint8_t hmac_demo_in[16] = { 0 };

	TEEC_Operation op = { 0 };
	
	printf("hmac_demo_teec_invoke  \n");

	op.params[0].tmpref.buffer = hmac_demo_in;
	op.params[0].tmpref.size = sizeof(hmac_demo_in);
	op.params[1].tmpref.buffer = hmac_demo_out;
	op.params[1].tmpref.size = sizeof(hmac_demo_out);
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
									TEEC_MEMREF_TEMP_OUTPUT,
									TEEC_NONE,
									TEEC_NONE);

	ret = TEEC_InvokeCommand(session,
						CMD_HMAC_TEST,
						&op,
						&ret_orig);
	printf("ret=0x%x\n, ret_orig=0x%x\n", ret, ret_orig);
}

void hmac_demo_do_handler()
{
#ifndef USE_TRUSTONIC
	TEEC_Session session = { .ctx = 0 };
#else
	TEEC_Session session;
#endif
	
	uint32_t ret_orig;

	hmac_demo_teec_opensession(&session, &hmac_demo_user_ta_uuid, NULL, &ret_orig);

	hmac_demo_teec_invoke(&session);

	hmac_demo_teec_closession(&session);
}

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;

	printf("\nhmac_demo CA enter device=[%s] 11111111111\n", _device);

	TEEC_InitializeContext(_device, &teec_ctx); //todo

	hmac_demo_do_handler();

	TEEC_FinalizeContext(&teec_ctx);

	printf("hmac_demo CA exit!\n");
	return 0;
}

