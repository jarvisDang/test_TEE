/*
 * Copyright (c) 2021, https://blog.csdn.net/weixin_42135087.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 */


#ifdef USE_TRUSTONIC

#include "taStd.h"
#include "tee_internal_api.h"
#include "GP_sample_protocol.h"
#include "hash_demo_uuid.h"
#include "buildTag.h"

TEE_UUID uuid = hash_demo_UUID;
DECLARE_TRUSTED_APPLICATION_MAIN_STACK(16384)

#define EMSG(fmt, args...) TEE_LogPrintf("hash_demo "fmt"\n", ##args)

#else

#include <tee_ta_api.h>
#include <trace.h>
#include <tee_api.h>
#include <tee_api_defines.h>

#endif

#include "ta_hash_demo.h"

TEE_Result TA_CreateEntryPoint(void)
{
	EMSG("TA_CreateEntryPoint!");
	/* Allocate some resources, init something, ... */
	/* Return with a status */
	return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
	/* Release resources if required before TA destruction */
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t ptype,
                                    TEE_Param param[4],
                                    void **session_id_ptr)
{
	(void)(ptype);
	(void)(param);
	(void)(session_id_ptr);
	
	EMSG("TA_OpenSessionEntryPoint!");
	/* Check client identity, and alloc/init some session resources if any */
	/* Return with a status */
	return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void *sess_ptr)
{
	(void)(sess_ptr);
	EMSG("TA_CloseSessionEntryPoint!");
	/* check client and handle session resource release, if any */
}

static TEE_Result sha_arithmetic(uint32_t mode, void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len)
{
	TEE_Result res;
	TEE_OperationHandle op;
	uint32_t keysize = 0;

	
	EMSG("%s enter", __func__);
	
	switch (mode)
	{
		case TEE_ALG_SHA1:
			keysize = 160;
			break;
		case TEE_ALG_SHA224:
			keysize = 224;
			break;
		case TEE_ALG_SHA256:
			keysize = 256;
			break;
		case TEE_ALG_SHA384:
			keysize = 384;
			break;
		case TEE_ALG_SHA512:
			keysize = 512;
			break;
	}
	
	EMSG("%s mode=0x%x, keysize=%d", __func__, mode, 0);
	res = TEE_AllocateOperation(&op, mode, TEE_MODE_DIGEST, 0);
	if (res) {
		EMSG("%s error!!! exit. res=0x%x", __func__, res);
		return res;
	}

	while (inbuf_len > (512/8)) {
		TEE_DigestUpdate(op, inbuf, (512/8));
		inbuf_len -= (512/8);
		inbuf += (512/8);
	}

	*outbuf_len = keysize / 8;
	res = TEE_DigestDoFinal(op, inbuf, inbuf_len, outbuf, outbuf_len);

	TEE_FreeOperation(op);

	EMSG("%s exit", __func__);
	return res;

}

#define SHA(name) \
TEE_Result sha##name(void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len) \
{ \
	return sha_arithmetic(TEE_ALG_SHA##name, inbuf, inbuf_len, outbuf, outbuf_len); \
}

SHA(1)
SHA(224)
SHA(256)
SHA(384)
SHA(512)

TEE_Result sha1(void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len);
TEE_Result sha224(void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len);
TEE_Result sha256(void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len);
TEE_Result sha384(void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len);
TEE_Result sha512(void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len);

static void test_hash(void)
{
	TEE_Result res;
	char in_buf[16] = "zhouhehetest";
	char out_buf[64] = {0};
	size_t out_sz = 64;
	
	EMSG("%s enter 11111", __func__);
	
	
	/* for sha256 */
	{
		/*
			test@test-21:~/workspace/test/keytest$ openssl dgst -sha256 1.txt
			SHA256(1.txt)= 1964016224508a6883194c9a92f9747213060919105fd38fae153febf195585a
		*/
		char expect_sha256_buf[32] = {
			0x19, 0x64, 0x01, 0x62, 0x24, 0x50, 0x8a, 0x68,
			0x83, 0x19, 0x4c, 0x9a, 0x92, 0xf9, 0x74, 0x72,
			0x13, 0x06, 0x09, 0x19, 0x10, 0x5f, 0xd3, 0x8f,
			0xae, 0x15, 0x3f, 0xeb, 0xf1, 0x95, 0x58, 0x5a
		};
		size_t expect_sha256_sz = (256 / 8);
	
		out_sz = 32;
		res = sha256(in_buf, 12, out_buf, &out_sz);
		EMSG("%s : res = %d, out_sz=%d", __func__, res, (uint32_t)out_sz);
		if(expect_sha256_sz == out_sz &&  TEE_MemCompare(expect_sha256_buf, out_buf, out_sz) == 0)
		{
			EMSG("Test %s-sha256 pass", __func__);
		} else {
			EMSG("Test %s-sha256 failed", __func__);
		}
	}

	
}

TEE_Result TA_InvokeCommandEntryPoint(void *session_id,
                                      uint32_t command_id,
                                      uint32_t param_types,
                                      TEE_Param parameters[4])
{
	(void)(session_id);
	(void)(parameters);

	EMSG("E TA enter!");
	
	if ((TEE_PARAM_TYPE_GET(param_types, 0) != TEE_PARAM_TYPE_MEMREF_INPUT) ||
		(TEE_PARAM_TYPE_GET(param_types, 1) != TEE_PARAM_TYPE_MEMREF_OUTPUT) ||
		(TEE_PARAM_TYPE_GET(param_types, 2) != TEE_PARAM_TYPE_NONE) ||
		(TEE_PARAM_TYPE_GET(param_types, 3) != TEE_PARAM_TYPE_NONE)) {
		EMSG("TEE_ERROR_BAD_PARAMETERS!");
		return TEE_ERROR_BAD_PARAMETERS;
	}
	
	switch (command_id) {
		case CMD_HASH_TEST:
			EMSG("TA_HASH_DEMO_CMD_1--!");
			test_hash();
			break;
		default:
			;
	}

	EMSG("TA exit!");
	/* Return with a status */
	return TEE_SUCCESS;
}
