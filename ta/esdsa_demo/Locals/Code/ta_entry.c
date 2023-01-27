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
#include "esdsa_demo_uuid.h"
#include "buildTag.h"

TEE_UUID uuid = esdsa_demo_UUID;
DECLARE_TRUSTED_APPLICATION_MAIN_STACK(16384)

#define EMSG(fmt, args...) TEE_LogPrintf("esdsa_demo "fmt"\n", ##args)

#else

#include <tee_ta_api.h>
#include <trace.h>
#include <tee_api.h>
#include <tee_api_defines.h>

#endif

#include "ta_esdsa_demo.h"

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


#define MAX_EC_BYTELENGTH ((521 + 7) / 8)

TEE_Result gen_ecdsa_key(uint32_t curve, uint32_t bitlength,  TEE_ObjectHandle *priv, TEE_ObjectHandle *pub) {

	TEE_Result nResult = TEE_SUCCESS;
	TEE_ObjectHandle k = TEE_HANDLE_NULL;
	TEE_ObjectHandle k_pri = TEE_HANDLE_NULL;
	TEE_ObjectHandle k_pub = TEE_HANDLE_NULL;
	TEE_Attribute params[4];
	params[0].attributeID = TEE_ATTR_ECC_CURVE;
	params[0].content.value.a = curve;
	
	const uint32_t bytelength = (bitlength + 7) / 8;
	size_t xsize, ysize, asize;
	
	// 1. Generate key.
	TEE_AllocateTransientObject(TEE_TYPE_ECDSA_KEYPAIR, bitlength, &k);
	
	TEE_GenerateKey(k, bitlength, params, 1);

	{
		// 2. Extract the public and private components.
		uint8_t x[MAX_EC_BYTELENGTH] = {0};
		uint8_t y[MAX_EC_BYTELENGTH] = {0};
		uint8_t a[MAX_EC_BYTELENGTH] = {0};
		xsize = bytelength;
		TEE_GetObjectBufferAttribute(k, TEE_ATTR_ECC_PUBLIC_VALUE_X, x, &xsize);
		if(!(xsize <= bytelength))
		{
			return -1;
		}

		ysize = bytelength;
		TEE_GetObjectBufferAttribute(k, TEE_ATTR_ECC_PUBLIC_VALUE_Y, y, &ysize);
		if(!(ysize <= bytelength))
		{
			return -1;
		}

		asize = bytelength;
		TEE_GetObjectBufferAttribute(k, TEE_ATTR_ECC_PRIVATE_VALUE, a, &asize);
		if(!(asize <= bytelength))
		{
			return -1;
		}
		
		// 3. Create new k_pub from public components.
		params[1].attributeID = TEE_ATTR_ECC_PUBLIC_VALUE_X;
		params[1].content.ref.buffer = x;
		params[1].content.ref.length = xsize;
		params[2].attributeID = TEE_ATTR_ECC_PUBLIC_VALUE_Y;
		params[2].content.ref.buffer = y;
		params[2].content.ref.length = ysize;
		TEE_AllocateTransientObject(TEE_TYPE_ECDSA_PUBLIC_KEY, bitlength, &k_pub);
		TEE_PopulateTransientObject(k_pub, params, 3);
		
		// 4. Create new k_pri from public and private components.
		params[3].attributeID = TEE_ATTR_ECC_PRIVATE_VALUE;
		params[3].content.ref.buffer = a;
		params[3].content.ref.length = asize;
		TEE_AllocateTransientObject(TEE_TYPE_ECDSA_KEYPAIR, bitlength, &k_pri);
		TEE_PopulateTransientObject(k_pri, params, 4);
	}
	*priv = k_pri;
	*pub = k_pub;
	return nResult;
}

TEE_Result ecdsa_operation(uint32_t bitlength, uint32_t alg, int mode, TEE_ObjectHandle key, uint8_t *digest, size_t *digestLen, uint8_t *signature, size_t *signatureLen) { /*key is priv or pub*/
	
	TEE_Result res = TEE_SUCCESS;
	TEE_OperationHandle op_sig = NULL;
	TEE_OperationHandle op_ver = NULL;
	const uint32_t bytelength = (bitlength + 7) / 8;
	
	switch(alg) {
	case TEE_ALG_ECDSA_P192:
		*digestLen = 20; /* SHA1 */
		break;
	case TEE_ALG_ECDSA_P224:
		*digestLen = 28; /* SHA224 */
		break;
	case TEE_ALG_ECDSA_P256:
		*digestLen = 32; /* SHA256 */
		break;
	case TEE_ALG_ECDSA_P384:
		*digestLen = 48; /* SHA384 */
		break;
	default:
	case TEE_ALG_ECDSA_P521:
		*digestLen = 64; /* SHA512 */
		break;
	}

	if(mode)
	{
		// 6. Sign the data with k_pri.
		TEE_AllocateOperation(&op_sig, alg, TEE_MODE_SIGN, bitlength);
		TEE_SetOperationKey(op_sig, key);
		res = TEE_AsymmetricSignDigest(op_sig, NULL, 0, digest, *digestLen, signature, signatureLen);
		if(!(*signatureLen == 2 * bytelength))
		{
			return -1;
		}
	}
	else
	{
		// 7. Verify the signature with k_pub.
		TEE_AllocateOperation(&op_ver, alg, TEE_MODE_VERIFY, bitlength);
		TEE_SetOperationKey(op_ver, key);
		res = TEE_AsymmetricVerifyDigest(op_ver, NULL, 0, digest, *digestLen, signature, *signatureLen);
	}
	
	TEE_FreeOperation(op_sig);
	TEE_FreeOperation(op_ver);
	return res;
}

static void test_ecdsa(void)
{
	TEE_Result res;
	uint8_t signature[2 * MAX_EC_BYTELENGTH];
	size_t signatureLen = 2 * ((521 + 7) / 8);
	uint8_t digest[MAX_EC_BYTELENGTH];
	size_t digestLen = 64;
	
	TEE_ObjectHandle priv;
	TEE_ObjectHandle pub;	
	
	EMSG("%s enter 11111wwwwwwww", __func__);
	
	res = gen_ecdsa_key(TEE_ECC_CURVE_NIST_P521, 521,  &priv, &pub);
	
	TEE_MemFill(digest, 0xEE, digestLen);
	
	res = ecdsa_operation(521, TEE_ALG_ECDSA_P521, 1, priv, digest, &digestLen, signature, &signatureLen); // sign
	res = ecdsa_operation(521, TEE_ALG_ECDSA_P521, 0, priv, digest, &digestLen, signature, &signatureLen); //verify
	
	if(res == 0)
	{
		EMSG("Test %s pass", __func__);
	} else {
		EMSG("Test %s failed", __func__);
	}
	
}

/*
	TEE_FreeTransientObject(k);
	TEE_FreeTransientObject(k_pri);
	TEE_FreeTransientObject(k_pub);

*/


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
		case CMD_ECDSA_TEST:
			test_ecdsa();
			break;
		default:
			;
	}

	EMSG("%s exit", __func__);

	return TEE_SUCCESS;
}
