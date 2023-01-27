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
#include "rsa_demo_uuid.h"
#include "buildTag.h"

TEE_UUID uuid = rsa_demo_UUID;
DECLARE_TRUSTED_APPLICATION_MAIN_STACK(16384)

#define EMSG(fmt, args...) TEE_LogPrintf("rsa_demo "fmt"\n", ##args)

#else

#include <tee_ta_api.h>
#include <trace.h>
#include <tee_api.h>
#include <tee_api_defines.h>

#endif

#include "ta_rsa_demo.h"

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


static TEE_ObjectHandle gen_key_rsa(uint32_t key_size)
{
	TEE_Result res;
	TEE_ObjectHandle key;

	EMSG("%s enter", __func__);

	res = TEE_AllocateTransientObject(TEE_TYPE_RSA_KEYPAIR, key_size, &key);
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

#define GEN_RSAKEY(size) \
TEE_ObjectHandle gen_key_rsa_##size(void) \
{ \
	return gen_key_rsa(size); \
}

GEN_RSAKEY(2048)
GEN_RSAKEY(3072)
GEN_RSAKEY(4096)

TEE_ObjectHandle gen_key_rsa_2048(void);
TEE_ObjectHandle gen_key_rsa_3072(void);
TEE_ObjectHandle gen_key_rsa_4096(void);


static TEE_Result rsa_arithmetic(TEE_ObjectHandle key, uint32_t mode, const void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len)
{
	TEE_Result res;

	TEE_OperationHandle op;
	TEE_ObjectInfo key_info;
	uint32_t alg = TEE_ALG_RSAES_PKCS1_V1_5;
	
	EMSG("%s enter", __func__);

	if (!key)
		return TEE_ERROR_BAD_STATE;

	res = TEE_GetObjectInfo1(key, &key_info);
	if (res) {
		EMSG("TEE_GetObjectInfo1 error! res=0x%x", res);
		return res;
	}
	
	switch (mode)
	{
		case TEE_MODE_ENCRYPT:
		case TEE_MODE_DECRYPT:
			alg = TEE_ALG_RSAES_PKCS1_V1_5;
			break;
		case TEE_MODE_SIGN:
		case TEE_MODE_VERIFY:
			alg = TEE_ALG_RSASSA_PKCS1_V1_5_SHA256;
			break;
	}

	res = TEE_AllocateOperation(&op, alg, mode, key_info.objectSize);
	if (res) {
		EMSG("TEE_AllocateTransientObject error! res=0x%x", res);
		return res;
	}

	res = TEE_SetOperationKey(op, key);
	if (res) {
		EMSG("TEE_SetOperationKey error! res=0x%x", res);
		TEE_FreeOperation(op);
		return TEE_ERROR_BAD_STATE;
	}

	switch (mode)
	{
		case TEE_MODE_ENCRYPT:
			res = TEE_AsymmetricEncrypt(op, NULL, 0, inbuf, inbuf_len, outbuf, outbuf_len);
			if (res) {
				EMSG("TEE_AsymmetricEncrypt error! res=0x%x", res);
			}
			break;
		case TEE_MODE_DECRYPT:
			res = TEE_AsymmetricDecrypt(op, NULL, 0, inbuf, inbuf_len, outbuf, outbuf_len);
			if (res) {
				EMSG("TEE_AsymmetricDecrypt error! res=0x%x", res);
			}
			break;
		case TEE_MODE_SIGN:
			res = TEE_AsymmetricSignDigest(op, NULL, 0, inbuf, inbuf_len, outbuf, outbuf_len);
			if (res) {
				EMSG("TEE_AllocateTransientObject error! res=0x%x", res);
			}
			break;
		case TEE_MODE_VERIFY:
			res = TEE_AsymmetricVerifyDigest(op, NULL, 0, inbuf, inbuf_len, outbuf, *outbuf_len);
			if (res) {
				EMSG("TEE_AsymmetricVerifyDigest error! res=0x%x", res);
			}
			break;
	}

	TEE_FreeOperation(op);
	
	EMSG("%s exit", __func__);
	return res;

}

#define RSA(name1,name2) \
TEE_Result rsa_##name1(TEE_ObjectHandle key, const void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len) \
{ \
	return rsa_arithmetic(key, TEE_MODE_##name2, inbuf, inbuf_len, outbuf, outbuf_len); \
}

RSA(enc, ENCRYPT)
RSA(dec, DECRYPT)
RSA(sign, SIGN)
RSA(verify, VERIFY)

TEE_Result rsa_enc(TEE_ObjectHandle key, const void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len);
TEE_Result rsa_dec(TEE_ObjectHandle key, const void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len);
TEE_Result rsa_sign(TEE_ObjectHandle key, const void *inbuf, uint32_t inbuf_len, void *signature, size_t *signatureLen);
TEE_Result rsa_verify(TEE_ObjectHandle key, const void *digest, uint32_t digestLen, void *signature, size_t *signatureLen);

static void test_rsa(void)
{
	TEE_Result res;
	char in_buf[32+1] = "0123456789abcdef0123456789abcdef";
	uint32_t in_sz = 32;
	char tmp_buf[512] = {0};
	size_t tmp_sz = 512;
	char out_buf[48] = {0};
	size_t out_sz = 48;
	TEE_ObjectHandle rsakey = gen_key_rsa_2048();
	
	EMSG("%s enter 11111", __func__);
	
	res = rsa_enc(rsakey, in_buf, in_sz, tmp_buf, &tmp_sz);
	EMSG("%s : res = %d, tmp_sz=%d", __func__, res, (int)tmp_sz);
	res = rsa_dec(rsakey, tmp_buf, tmp_sz, out_buf, &out_sz);
	EMSG("%s : res = %d, out_sz=%d", __func__, res, (int)out_sz);
	
	if(in_sz == out_sz &&  TEE_MemCompare(in_buf, out_buf, out_sz) == 0)
	{
		EMSG("Test %s-rsa_enc/rsa_dec pass", __func__);
	} else {
		EMSG("Test %s-rsa_enc/rsa_dec failed", __func__);
	}
	
	tmp_sz = 256;
	res = rsa_sign(rsakey, in_buf, in_sz, tmp_buf, &tmp_sz);
	EMSG("%s : res = %d, tmp_sz=%d", __func__, res, (int)tmp_sz);
	res = rsa_verify(rsakey, in_buf, in_sz, tmp_buf, &tmp_sz);
	EMSG("%s : res = %d", __func__, res);
	if(res == 0)
	{
		EMSG("Test %s-rsa_sign/rsa_verify pass", __func__);
	} else {
		EMSG("Test %s-rsa_sign/rsa_verify failed", __func__);
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
		case CMD_RSA_TEST:
			EMSG("--CMD_RSA_TEST--!");
			test_rsa();
			break;
		default:
			;
	}

	EMSG("%s exit", __func__);

	return TEE_SUCCESS;
}
