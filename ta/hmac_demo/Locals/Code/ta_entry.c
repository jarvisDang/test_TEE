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
#include "hmac_demo_uuid.h"
#include "buildTag.h"

TEE_UUID uuid = hmac_demo_UUID;
DECLARE_TRUSTED_APPLICATION_MAIN_STACK(16384)

#define EMSG(fmt, args...) TEE_LogPrintf("hmac_demo "fmt"\n", ##args)

#else

#include <tee_ta_api.h>
#include <trace.h>
#include <tee_api.h>
#include <tee_api_defines.h>

#endif

#include "ta_hmac_demo.h"

TEE_Result TA_CreateEntryPoint(void)
{
	EMSG("%s enter", __func__);
	return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
	EMSG("%s enter", __func__);
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t ptype,
                                    TEE_Param param[4],
                                    void **session_id_ptr)
{
	(void)(ptype);
	(void)(param);
	(void)(session_id_ptr);

	EMSG("%s enter", __func__);

	return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void *sess_ptr)
{
	(void)(sess_ptr);
	EMSG("%s enter", __func__);
}

static TEE_Result do_hmac(TEE_ObjectHandle hmac_key, uint32_t keysize, uint32_t alg,  void *inbuf, uint32_t inbuf_len, void *mac, size_t *maclen)
{
	TEE_Result res;
	TEE_OperationHandle op;
	
	EMSG("%s enter", __func__);
	
	if(keysize == 1) 
		keysize = 160;

	res = TEE_AllocateOperation(&op, alg, TEE_MODE_MAC, keysize);
	if(res != TEE_SUCCESS) {
		EMSG("%s error 1, res = [%x]", __func__, res);
		return res;
	}

	res = TEE_SetOperationKey(op, hmac_key);
	if(res != TEE_SUCCESS) {
		TEE_FreeOperation(op);
		EMSG("%s error 2, res = [%x]", __func__, res);
		return res;
	}

	TEE_MACInit(op, NULL, 0);
	TEE_MACUpdate(op, inbuf, inbuf_len);
	res = TEE_MACComputeFinal(op, NULL, 0, mac, maclen);
	
	return res;
}

#define HMAC(size) \
TEE_Result  hamc_sha##size(TEE_ObjectHandle hmac_key, void *inbuf, uint32_t inbuf_len, void *mac, size_t *maclen) \
{ \
	return do_hmac(hmac_key, size, TEE_ALG_HMAC_SHA##size, inbuf, inbuf_len, mac, maclen); \
}
HMAC(1)
HMAC(224)
HMAC(256)
HMAC(384)
HMAC(512)
TEE_Result hamc_sha1(TEE_ObjectHandle hmac_key, void *inbuf, uint32_t inbuf_len, void *mac, size_t *maclen);
TEE_Result hamc_sha224(TEE_ObjectHandle hmac_key, void *inbuf, uint32_t inbuf_len, void *mac, size_t *maclen);
TEE_Result hamc_sha256(TEE_ObjectHandle hmac_key, void *inbuf, uint32_t inbuf_len, void *mac, size_t *maclen);
TEE_Result hamc_sha384(TEE_ObjectHandle hmac_key, void *inbuf, uint32_t inbuf_len, void *mac, size_t *maclen);
TEE_Result hamc_sha512(TEE_ObjectHandle hmac_key, void *inbuf, uint32_t inbuf_len, void *mac, size_t *maclen);

static TEE_ObjectHandle gen_key_hmac(uint32_t key_type, size_t key_size)
{
	TEE_Result res;
	TEE_ObjectHandle key;

	EMSG("%s enter", __func__);
	
	if(key_size == 1) 
		key_size = 160;

	res = TEE_AllocateTransientObject(key_type, key_size, &key);
	if (res) {
		EMSG("TEE_AllocateTransientObject error! res=0x%x", res);
		return NULL;
	}

	res = TEE_GenerateKey(key, key_size, NULL, 0);
	if (res) {
		EMSG("TEE_GenerateKey error! res=0x%x", res);
		TEE_FreeTransientObject(key);
		return NULL;
	}

	return key;
}

#define GEN_HMAC_KEY(size) \
TEE_ObjectHandle gen_key_hmac_sha##size(void) \
{ \
	return gen_key_hmac(TEE_TYPE_HMAC_SHA##size, size); \
}

GEN_HMAC_KEY(1)
GEN_HMAC_KEY(224)
GEN_HMAC_KEY(256)
GEN_HMAC_KEY(384)
GEN_HMAC_KEY(512)

TEE_ObjectHandle gen_key_hmac_sha1(void);
TEE_ObjectHandle gen_key_hmac_sha224(void);
TEE_ObjectHandle gen_key_hmac_sha256(void);
TEE_ObjectHandle gen_key_hmac_sha384(void);
TEE_ObjectHandle gen_key_hmac_sha512(void);

static void test_hmac(void)
{
	TEE_Result res;
	char in_buf[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	uint32_t in_sz = 16;
	char mac[64] = {0};
	size_t maclen = 64;
	TEE_ObjectHandle hmackey = gen_key_hmac_sha256();
	
	EMSG("%s enter 11111", __func__);
	
	res = hamc_sha256(hmackey, in_buf, in_sz, mac, &maclen);
	EMSG("%s : res = %d, maclen=%d", __func__, res, (int)maclen);

	
	if(res == 0)
	{
		EMSG("Test %s pass", __func__);
	} else {
		EMSG("Test %s failed", __func__);
	}
	
}

TEE_Result TA_InvokeCommandEntryPoint(void *session_id,
                                      uint32_t command_id,
                                      uint32_t param_types,
                                      TEE_Param parameters[4])
{
	(void)(session_id);
	(void)(parameters);

	EMSG("%s enter", __func__);

	if ((TEE_PARAM_TYPE_GET(param_types, 0) != TEE_PARAM_TYPE_MEMREF_INPUT) ||
		(TEE_PARAM_TYPE_GET(param_types, 1) != TEE_PARAM_TYPE_MEMREF_OUTPUT) ||
		(TEE_PARAM_TYPE_GET(param_types, 2) != TEE_PARAM_TYPE_NONE) ||
		(TEE_PARAM_TYPE_GET(param_types, 3) != TEE_PARAM_TYPE_NONE)) {
		EMSG("The param_types is error!");
		return TEE_ERROR_BAD_PARAMETERS;
	}

	switch (command_id) {
		case CMD_HMAC_TEST:
			EMSG("--CMD_HMAC_TEST--!");
			test_hmac();
			break;
		default:
			;
	}

	EMSG("%s exit", __func__);

	return TEE_SUCCESS;
}
