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
#include "aes_auth_demo_uuid.h"
#include "buildTag.h"

TEE_UUID uuid = aes_auth_demo_UUID;
DECLARE_TRUSTED_APPLICATION_MAIN_STACK(16384)

#define EMSG(fmt, args...) TEE_LogPrintf("aes_auth_demo "fmt"\n", ##args)

#else

#include <tee_ta_api.h>
#include <trace.h>
#include <tee_api.h>
#include <tee_api_defines.h>

#endif

#include "ta_aes_auth_demo.h"

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

#define GEN_AESKEY(key_size) \
TEE_ObjectHandle gen_key_aes_##key_size(void) \
{ \
	TEE_Result res; \
	TEE_ObjectHandle key; \
 \
 \
	res = TEE_AllocateTransientObject(TEE_TYPE_AES, key_size, &key);  \
	if (res) {  \
		return NULL;  \
	}  \
  \
	res = TEE_GenerateKey(key, key_size, NULL, 0);  \
	if (res) {  \
		TEE_FreeTransientObject(key);  \
		return NULL;  \
	}  \
  \
	return key;  \
}
GEN_AESKEY(128)
GEN_AESKEY(192)
GEN_AESKEY(256)

TEE_ObjectHandle gen_key_aes_128(void);
TEE_ObjectHandle gen_key_aes_192(void);
TEE_ObjectHandle gen_key_aes_256(void);

static uint8_t iv[16] = {0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5};
static TEE_Result aes_authenticated_arithmetic(TEE_ObjectHandle aes_key, uint32_t alg, void *iv, uint32_t iv_len, void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len,void *tag, size_t *tag_len)
{
	TEE_Result res;
	TEE_OperationHandle op;
	TEE_ObjectInfo KeyInfo = {0};
	
	EMSG("%s enter", __func__);
	
	TEE_GetObjectInfo1(aes_key, &KeyInfo);

	res = TEE_AllocateOperation(&op, TEE_ALG_AES_GCM, alg, KeyInfo.maxObjectSize);
	if(res != TEE_SUCCESS) {
		EMSG("%s error 1, res = [%x]", __func__, res);
		return res;
	}

	res = TEE_SetOperationKey(op, aes_key);
	if(res != TEE_SUCCESS) {
		TEE_FreeOperation(op);
		EMSG("%s error 2, res = [%x]", __func__, res);
		return res;
	}

	res = TEE_AEInit(op, iv, iv_len, 128, 0, 0); /* tagLen:128*/
	if(res != TEE_SUCCESS) {
		TEE_FreeOperation(op);
		EMSG("%s error 2, res = [%x]", __func__, res);
		return res;
	}

	res = TEE_AEUpdate(op, inbuf, inbuf_len, outbuf, outbuf_len);
	if(res != TEE_SUCCESS) {
		TEE_FreeOperation(op);
		EMSG("%s error 3, res = [%x]", __func__, res);
		return res;
	}

	res = TEE_AEEncryptFinal(op, inbuf, inbuf_len, outbuf, outbuf_len, tag, tag_len);
	if(res != TEE_SUCCESS) {
		TEE_FreeOperation(op);
		EMSG("%s error 4, res = [%x]", __func__, res);
		return res;
	}

	TEE_FreeOperation(op);
	
	EMSG("%s exit", __func__);
	return res;
}




#define AES_GCM(alg) \
TEE_Result AES_GCM_##alg(TEE_ObjectHandle aes_key, void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len,void *tag, size_t *tag_len) \
{ \
	return aes_authenticated_arithmetic(aes_key,  TEE_MODE_##alg,  iv, sizeof(iv), inbuf, inbuf_len, outbuf, outbuf_len, tag, tag_len); \
}
AES_GCM(ENCRYPT)
AES_GCM(DECRYPT)

TEE_Result AES_GCM_ENCRYPT(TEE_ObjectHandle aes_key, void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len,void *tag, size_t *tag_len);
TEE_Result AES_GCM_DECRYPT(TEE_ObjectHandle aes_key, void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len,void *tag, size_t *tag_len) ;


static void test_gcm(void)
{
	TEE_Result res;
	char in_buf[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	uint32_t in_sz = 16;
	char tag[48] = {0};
	size_t tag_sz = 48;
	char out_buf[48] = {0};
	size_t out_sz = 48;
	TEE_ObjectHandle aeskey = gen_key_aes_128();
	
	EMSG("%s enter 11111", __func__);
	
	res = AES_GCM_ENCRYPT(aeskey, in_buf,in_sz, out_buf, &out_sz, tag, &tag_sz);
	EMSG("%s : res = %d, out_buf=%d, tag_sz=%d", __func__, res, (int)out_sz, (int)tag_sz);

	
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
		case CMD_AES_AUTH_TEST:
			EMSG("-CMD_AES_AUTH_TEST--!");
			test_gcm();
			break;
		default:
			;
	}

	EMSG("%s exit", __func__);

	return TEE_SUCCESS;
}
