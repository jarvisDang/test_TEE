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
#include "aes_demo_uuid.h"
#include "buildTag.h"

TEE_UUID uuid = aes_demo_UUID;
DECLARE_TRUSTED_APPLICATION_MAIN_STACK(16384)

#define EMSG(fmt, args...) TEE_LogPrintf("aes_demo "fmt"\n", ##args)

#else

#include <tee_ta_api.h>
#include <trace.h>
#include <tee_api.h>
#include <tee_api_defines.h>

#endif

#include "ta_aes_demo.h"

TEE_Result TA_CreateEntryPoint(void)
{
	EMSG("aes_demo TA_CreateEntryPoint!");
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
	
	EMSG("aes_demo TA_OpenSessionEntryPoint!");
	/* Check client identity, and alloc/init some session resources if any */
	/* Return with a status */
	return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void *sess_ptr)
{
	(void)(sess_ptr);
	EMSG("aes_demo TA_CloseSessionEntryPoint!");
	/* check client and handle session resource release, if any */
}



static TEE_Result aes_arithmetic(TEE_ObjectHandle aes_key, uint32_t keysize, uint32_t alg, uint32_t mode, void *iv, uint32_t iv_len, void *inbuf, uint32_t inbuf_len, void *outbuf, uint32_t *outbuf_len)
{
	TEE_Result res;
	TEE_OperationHandle op;
	size_t destLen;
	int outsize;
	uint32_t block = 128;
	
	EMSG("%s enter", __func__);

	res = TEE_AllocateOperation(&op, mode, alg, keysize);
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

	TEE_CipherInit(op, iv, iv_len);

	outsize = 0;
	destLen = *outbuf_len;

	while (inbuf_len > (block/8)) {
		res = TEE_CipherUpdate(op, inbuf, (block/8), outbuf, &destLen);
		if(res != TEE_SUCCESS) {
			TEE_FreeOperation(op);
			EMSG("%s error 3, res = [%x]", __func__, res);
			return res;
		}
		inbuf_len -= (block/8);
		inbuf += (block/8);
		outbuf += destLen;
		outsize += destLen;
		destLen = *outbuf_len - outsize;
	}

	res = TEE_CipherDoFinal(op, inbuf, inbuf_len, outbuf, &destLen);
	if(res != TEE_SUCCESS) {
		TEE_FreeOperation(op);
		EMSG("%s error 4, res = [%x]", __func__, res);
		return res;
	}
	*outbuf_len = outsize + destLen;

	TEE_FreeOperation(op);
	
	EMSG("%s exit", __func__);
	return res;
}

static uint8_t iv[16] = {0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5,0xa5};


#define AES(alg, mode, length) \
TEE_Result AES_##alg##_##mode##_##length(TEE_ObjectHandle aes_key, void *inbuf, uint32_t inbuf_len, void *outbuf, uint32_t *outbuf_len) \
{ \
	return aes_arithmetic(aes_key, 256, TEE_MODE_##alg, TEE_ALG_AES_##mode##_NOPAD, iv, sizeof(iv), inbuf, inbuf_len, outbuf, outbuf_len); \
}

AES(ENCRYPT, ECB, 128)
AES(ENCRYPT, ECB, 192)
AES(ENCRYPT, ECB, 256)
AES(DECRYPT, ECB, 128)
AES(DECRYPT, ECB, 192)
AES(DECRYPT, ECB, 256)
AES(ENCRYPT, CBC, 128)
AES(ENCRYPT, CBC, 192)
AES(ENCRYPT, CBC, 256)
AES(DECRYPT, CBC, 128)
AES(DECRYPT, CBC, 192)
AES(DECRYPT, CBC, 256)

TEE_Result AES_ENCRYPT_ECB_128(TEE_ObjectHandle aes_key, void *inbuf, uint32_t inbuf_len, void *outbuf, uint32_t *outbuf_len);
TEE_Result AES_ENCRYPT_ECB_192(TEE_ObjectHandle aes_key, void *inbuf, uint32_t inbuf_len, void *outbuf, uint32_t *outbuf_len);
TEE_Result AES_ENCRYPT_ECB_256(TEE_ObjectHandle aes_key, void *inbuf, uint32_t inbuf_len, void *outbuf, uint32_t *outbuf_len);
TEE_Result AES_DECRYPT_ECB_128(TEE_ObjectHandle aes_key, void *inbuf, uint32_t inbuf_len, void *outbuf, uint32_t *outbuf_len);
TEE_Result AES_DECRYPT_ECB_192(TEE_ObjectHandle aes_key, void *inbuf, uint32_t inbuf_len, void *outbuf, uint32_t *outbuf_len);
TEE_Result AES_DECRYPT_ECB_256(TEE_ObjectHandle aes_key, void *inbuf, uint32_t inbuf_len, void *outbuf, uint32_t *outbuf_len);
TEE_Result AES_ENCRYPT_CBC_128(TEE_ObjectHandle aes_key, void *inbuf, uint32_t inbuf_len, void *outbuf, uint32_t *outbuf_len);
TEE_Result AES_ENCRYPT_CBC_192(TEE_ObjectHandle aes_key, void *inbuf, uint32_t inbuf_len, void *outbuf, uint32_t *outbuf_len);
TEE_Result AES_ENCRYPT_CBC_256(TEE_ObjectHandle aes_key, void *inbuf, uint32_t inbuf_len, void *outbuf, uint32_t *outbuf_len);
TEE_Result AES_DECRYPT_CBC_128(TEE_ObjectHandle aes_key, void *inbuf, uint32_t inbuf_len, void *outbuf, uint32_t *outbuf_len);
TEE_Result AES_DECRYPT_CBC_192(TEE_ObjectHandle aes_key, void *inbuf, uint32_t inbuf_len, void *outbuf, uint32_t *outbuf_len);
TEE_Result AES_DECRYPT_CBC_256(TEE_ObjectHandle aes_key, void *inbuf, uint32_t inbuf_len, void *outbuf, uint32_t *outbuf_len);

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

static void test_ecb(void)
{
	TEE_Result res;
	char in_buf[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	uint32_t in_sz = 16;
	char tmp_buf[48] = {0};
	uint32_t tmp_sz = 48;
	char out_buf[48] = {0};
	uint32_t out_sz = 48;
	TEE_ObjectHandle aeskey = gen_key_aes_128();
	
	EMSG("%s enter 11111", __func__);
	
	res = AES_ENCRYPT_ECB_128(aeskey, in_buf, in_sz, tmp_buf, &tmp_sz);
	EMSG("%s : res = %d, tmp_sz=%d", __func__, res, tmp_sz);
	res = AES_DECRYPT_ECB_128(aeskey, tmp_buf, tmp_sz, out_buf, &out_sz);
	EMSG("%s : res = %d, out_sz=%d", __func__, res, out_sz);
	
	if(in_sz == out_sz &&  TEE_MemCompare(in_buf, out_buf, out_sz) == 0)
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

	EMSG("E TA enter!");
	
	if ((TEE_PARAM_TYPE_GET(param_types, 0) != TEE_PARAM_TYPE_MEMREF_INPUT) ||
		(TEE_PARAM_TYPE_GET(param_types, 1) != TEE_PARAM_TYPE_MEMREF_OUTPUT) ||
		(TEE_PARAM_TYPE_GET(param_types, 2) != TEE_PARAM_TYPE_NONE) ||
		(TEE_PARAM_TYPE_GET(param_types, 3) != TEE_PARAM_TYPE_NONE)) {
		EMSG("TEE_ERROR_BAD_PARAMETERS!");
		return TEE_ERROR_BAD_PARAMETERS;
	}
	
	switch (command_id) {
		case CMD_AES_TEST:
			EMSG("-CMD_AES_TEST--!");
			test_ecb();
			break;
		default:
			;
	}

	EMSG("TA exit!");
	/* Return with a status */
	return TEE_SUCCESS;
}
