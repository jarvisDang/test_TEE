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

#include <ta_persist_demo.h>

const TEEC_UUID persist_demo_user_ta_uuid = TA_PERSIST_DEMO_UUID;
TEEC_Context teec_ctx;

TEEC_Result persist_demo_teec_opensession(TEEC_Session *session,
				    const TEEC_UUID *uuid, TEEC_Operation *op,
				    uint32_t *ret_orig)
{
	return TEEC_OpenSession(&teec_ctx, session, uuid,
				TEEC_LOGIN_PUBLIC, NULL, op, ret_orig);
}

TEEC_Result persist_demo_teec_closession(TEEC_Session *session)
{
	(void)session;
	TEEC_CloseSession(session);
	return 0;
}


static void invoke_tee_wrap(TEEC_Session *session, char *src, size_t src_sz, char *dst, size_t *dst_sz)
{
	uint32_t ret_orig;
	uint32_t ret;

	TEEC_Operation op = { 0 };
	
	printf("%s enter\n", __func__);

	op.params[0].tmpref.buffer = src;
	op.params[0].tmpref.size = src_sz;
	op.params[1].tmpref.buffer = dst;
	op.params[1].tmpref.size = *dst_sz;
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
									TEEC_MEMREF_TEMP_OUTPUT,
									TEEC_NONE,
									TEEC_NONE);

	ret = TEEC_InvokeCommand(session,
						TA_PERSIST_WRITE,
						&op,
						&ret_orig);
	*dst_sz = op.params[1].tmpref.size;
	printf("ret=0x%x\n, ret_orig=0x%x\n", ret, ret_orig);
	printf("dst_sz=%zu\n", *dst_sz);
}

static void invoke_tee_unwrap(TEEC_Session *session, char *cipher, size_t cipher_sz)
{
	uint32_t ret_orig;
	uint32_t ret;

	TEEC_Operation op = { 0 };
	
	printf("%s enter\n", __func__);

	op.params[0].tmpref.buffer = cipher;
	op.params[0].tmpref.size = cipher_sz;
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
									TEEC_NONE,
									TEEC_NONE,
									TEEC_NONE);

	ret = TEEC_InvokeCommand(session,
						TA_PERSIST_READ,
						&op,
						&ret_orig);
	printf("ret=0x%x\n, ret_orig=0x%x\n", ret, ret_orig);
}

#define ATTEST_KEYBOX_FILE "/mnt/vendor/persist/test_hehe.data"
static int saveFile(const char *pPath, void* ppContent, size_t size)
{
	size_t res = 0;
	FILE *f = fopen(pPath, "wb");
	if (f == NULL) {
		printf("Error opening file: %s\n", pPath);
		return -3;
	}
	
	res = fwrite(ppContent, sizeof(uint8_t), size, f);
	fclose(f);
	if (res != size) {
		printf("Saving keybox failed: %zu != %zu\n", res, size);
		return -4;
	}

	return 0;
}


static long getFileContent(const char* pPath, void** ppContent)
{
	FILE*   pStream;
	long    filesize;
	uint8_t* content = NULL;
	
	/* Open the file */
	pStream = fopen(pPath, "rb");
	if (pStream == NULL)
	{
		fprintf(stderr, "Error: Cannot open file: %s.\n", pPath);
		goto error;
	}
	
	if (fseek(pStream, 0L, SEEK_END) != 0)
	{
		fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
		goto error;
	}
	
	filesize = ftell(pStream);
	if (filesize < 0)
	{
		fprintf(stderr, "Error: Cannot get the file size: %s.\n", pPath);
		goto error;
	}
	
	if (filesize == 0)
	{
		fprintf(stderr, "Error: Empty file: %s.\n", pPath);
		goto error;
	}
	
	/* Set the file pointer at the beginning of the file */
	if (fseek(pStream, 0L, SEEK_SET) != 0)
	{
		fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
		goto error;
	}
	
	/* Allocate a buffer for the content */
	content = (uint8_t*)malloc(filesize);
	if (content == NULL)
	{
		fprintf(stderr, "Error: Cannot read file: Out of memory.\n");
		goto error;
	}
	
	/* Read data from the file into the buffer */
	if (fread(content, (size_t)filesize, 1, pStream) != 1)
	{
		fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
		goto error;
	}
	
	/* Close the file */
	fclose(pStream);
	*ppContent = content;
	
	/* Return number of bytes read */
	return filesize;
	
error:
	if (content != NULL)
		free(content);
	
	if (pStream != NULL)
		fclose(pStream);
	
	return 0;
}




int main(int argc, char *argv[])
{
#ifndef USE_TRUSTONIC
	TEEC_Session session = { .ctx = 0 };
#else
	TEEC_Session session;
#endif
	
	uint32_t ret_orig;
	char dst_buf[1024] = {0};
	size_t dst_buf_sz = 1024;
	void *pContent = NULL;
	
	(void) argc;
	(void) argv;

	printf("\npersist_demo CA enter device=[%s] 22\n", _device);

	TEEC_InitializeContext(_device, &teec_ctx); //todo
	persist_demo_teec_opensession(&session, &persist_demo_user_ta_uuid, NULL, &ret_orig);

	invoke_tee_wrap(&session, "zhouhehetest", 13, dst_buf, &dst_buf_sz);
	saveFile(ATTEST_KEYBOX_FILE, dst_buf, dst_buf_sz);
	dst_buf_sz = getFileContent(ATTEST_KEYBOX_FILE, &pContent);
	invoke_tee_unwrap(&session, pContent, dst_buf_sz);
	
	free(pContent);
	persist_demo_teec_closession(&session);
	TEEC_FinalizeContext(&teec_ctx);

	printf("persist_demo CA exit!\n");
	return 0;
}

