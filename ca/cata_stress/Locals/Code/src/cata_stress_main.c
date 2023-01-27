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
#include <time.h>

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

#include <ta_cata_stress.h>

const TEEC_UUID cata_stress_user_ta_uuid = TA_CATA_STRESS_UUID;
TEEC_Context teec_ctx;

TEEC_Result cata_stress_teec_opensession(TEEC_Session *session,
				    const TEEC_UUID *uuid, TEEC_Operation *op,
				    uint32_t *ret_orig)
{
	return TEEC_OpenSession(&teec_ctx, session, uuid,
				TEEC_LOGIN_PUBLIC, NULL, op, ret_orig);
}

TEEC_Result cata_stress_teec_closession(TEEC_Session *session)
{
	(void)session;
	TEEC_CloseSession(session);
	return 0;
}

static void cata_stress_teec_invoke(TEEC_Session *session, int cmd, uint32_t count)
{
	uint32_t ret;
	uint32_t i;
	unsigned long s_seconds;
	unsigned long e_seconds;

	TEEC_Operation op = { 0 };
	
	printf("cata_stress_teec_invoke  \n");

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);
	
	s_seconds = time((time_t*)NULL);
    printf("count=%d, start_time=%ld\n", count, s_seconds);

	for(i=0;i<count;i++)
	{
		ret = TEEC_InvokeCommand(session, cmd, &op, NULL);
	}
	e_seconds = time((time_t*)NULL);
	printf("count=%d, end_time=%ld, diff_time=%ld\n", count, e_seconds, e_seconds - s_seconds);
}

static void tadriver_stress_teec_invoke(TEEC_Session *session, int cmd, uint32_t count)
{
	uint32_t ret;
	unsigned long s_seconds;
	unsigned long e_seconds;

	TEEC_Operation op = { 0 };
	
	printf("cata_stress_teec_invoke  \n");

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);
	
	// for value
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
					TEEC_VALUE_OUTPUT,
					TEEC_NONE,
					TEEC_NONE);
	
	op.params[0].value.a = count;
	
	s_seconds = time((time_t*)NULL);
    printf("count=%d, start_time=%ld\n", count, s_seconds);

	ret = TEEC_InvokeCommand(session, cmd, &op, NULL);

	e_seconds = time((time_t*)NULL);
	printf("count=%d, end_time=%ld, diff_time=%ld\n", count, e_seconds, e_seconds - s_seconds);
}

void cata_stress_do_handler(int flag, uint32_t count)
{
#ifndef USE_TRUSTONIC
	TEEC_Session session = { .ctx = 0 };
#else
	TEEC_Session session;
#endif
	
	uint32_t ret_orig;

	cata_stress_teec_opensession(&session, &cata_stress_user_ta_uuid, NULL, &ret_orig);

if(flag == TEST_CATA_COMMUNICATION)
	cata_stress_teec_invoke(&session, flag, count);
else if(flag == TEST_TADRIVER_COMMUNICATION)
	tadriver_stress_teec_invoke(&session, flag, count);

	cata_stress_teec_closession(&session);
}

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;
	char *cmd;
	uint32_t count;
	int flag;

	printf("\ncata_stress CA enter device=[%s] 11111111111\n", _device);
	
	cmd = argv[1];
	sscanf(argv[2],"0x%x",&count);
	
	if (memcmp(cmd,"test_cata",9) == 0) {
		flag = TEST_CATA_COMMUNICATION;
	}
	if (memcmp(cmd,"test_tadriver",13) == 0) {
		flag = TEST_TADRIVER_COMMUNICATION;
	}
	

	TEEC_InitializeContext(_device, &teec_ctx); //todo

	cata_stress_do_handler(flag, count);

	TEEC_FinalizeContext(&teec_ctx);

	printf("cata_stress CA exit!\n");
	return 0;
}

