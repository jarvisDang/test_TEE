/*
 * Copyright (c) 2014, STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef USE_TRUSTONIC

#include "taStd.h"
#include "TlApi/TlApi.h"
#include "tee_internal_api.h"
#include "GP_sample_protocol.h"
#include "persist_demo_uuid.h"
#include "buildTag.h"

TEE_UUID uuid = persist_demo_UUID;
DECLARE_TRUSTED_APPLICATION_MAIN_STACK(16384)

#define EMSG(fmt, args...) TEE_LogPrintf(fmt"\n", ##args)

#else

#include <tee_ta_api.h>
#include <trace.h>
#include <tee_api.h>
#include <tee_api_defines.h>

#define TLAPI_UNWRAP_DEFAULT (0)
#define MC_SO_CONTEXT_TLT (0)
#define MC_SO_LIFETIME_PERMANENT (0)
#define TLAPI_WRAP_DEFAULT (0)
static __attribute__((unused)) inline TEE_Result tlApiWrapObject(
				const void *src,
				size_t plainLen,
				size_t encryptedLen,
				void *dest,
				size_t *destLen,
				uint32_t context,
				uint32_t lifetime,
				void *consumer,
				uint32_t flags)
{
	*destLen = encryptedLen;
	TEE_MemMove(dest, src, encryptedLen);
	return 0;
}

static __attribute__((unused)) inline TEE_Result tlApiUnwrapObject(
				void *src,
				size_t srcLen,
				void *dest,
				size_t *destLen,
				uint32_t flags)
{
	*destLen = srcLen;
	TEE_MemMove(dest, src, srcLen);
	return 0;
}

#endif

#include "ta_persist_demo.h"

TEE_Result TA_CreateEntryPoint(void)
{
	EMSG("persist_demo TA_CreateEntryPoint!");
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
	
	EMSG("persist_demo TA_OpenSessionEntryPoint!");
	/* Check client identity, and alloc/init some session resources if any */
	/* Return with a status */
	return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void *sess_ptr)
{
	(void)(sess_ptr);
	EMSG("persist_demo TA_CloseSessionEntryPoint!");
	/* check client and handle session resource release, if any */
}

TEE_Result TA_InvokeCommandEntryPoint(void *session_id,
                                      uint32_t command_id,
                                      uint32_t param_types,
                                      TEE_Param params[4])
{
	TEE_Result res = TEE_SUCCESS;
	char *src;
	size_t src_sz;
	char *dst;
	size_t dst_sz;
	char *inter_buf;
	size_t inter_buf_sz;


	(void)(session_id);
	(void)(params);

	EMSG("persist_demo E TA enter!");
	
	
	src = params[0].memref.buffer;
	src_sz = params[0].memref.size;
	
	dst = params[1].memref.buffer;
	dst_sz = params[1].memref.size = 0;
	 
	
	switch (command_id) {
		case TA_PERSIST_READ:
			EMSG("persist_demo --TA_PERSIST_READ--!");
			if ((TEE_PARAM_TYPE_GET(param_types, 0) != TEE_PARAM_TYPE_MEMREF_INPUT) ||
				(TEE_PARAM_TYPE_GET(param_types, 1) != TEE_PARAM_TYPE_NONE) ||
				(TEE_PARAM_TYPE_GET(param_types, 2) != TEE_PARAM_TYPE_NONE) ||
				(TEE_PARAM_TYPE_GET(param_types, 3) != TEE_PARAM_TYPE_NONE)) {
				EMSG("persist_demo TEE_ERROR_BAD_PARAMETERS!");
				return TEE_ERROR_BAD_PARAMETERS;
			}
			inter_buf = TEE_Malloc(1024, 0);
			TEE_MemFill(inter_buf,0,1024);
			inter_buf_sz = 1024;
			res = tlApiUnwrapObject(src, src_sz, inter_buf, &inter_buf_sz, TLAPI_UNWRAP_DEFAULT);
			
			EMSG("%s : persist_demo:  res=0x%x, read buf=%s : len=%d",__func__, res, inter_buf, (int)inter_buf_sz);
			
			TEE_Free(inter_buf);
			;
			break;
		case TA_PERSIST_WRITE:
			EMSG("persist_demo --TA_PERSIST_WRITE--111111111111");
			if ((TEE_PARAM_TYPE_GET(param_types, 0) != TEE_PARAM_TYPE_MEMREF_INPUT) ||
				(TEE_PARAM_TYPE_GET(param_types, 1) != TEE_PARAM_TYPE_MEMREF_OUTPUT) ||
				(TEE_PARAM_TYPE_GET(param_types, 2) != TEE_PARAM_TYPE_NONE) ||
				(TEE_PARAM_TYPE_GET(param_types, 3) != TEE_PARAM_TYPE_NONE)) {
				EMSG("persist_demo TEE_ERROR_BAD_PARAMETERS!");
				return TEE_ERROR_BAD_PARAMETERS;
			}
			EMSG("%s : persist_demo:  write buf=%s : len=%d",__func__, src, (int)src_sz);
			
			inter_buf = TEE_Malloc(1024, 0);
			inter_buf_sz = 1024;
			res = tlApiWrapObject(
						src,
						0,
						src_sz,
						inter_buf,
						&inter_buf_sz,
						MC_SO_CONTEXT_TLT,
						MC_SO_LIFETIME_PERMANENT,
						NULL,
						TLAPI_WRAP_DEFAULT); //private wrapping
			EMSG("www persist_demo:  res=%d, inter_buf_sz = %d", res, (int)inter_buf_sz);
			TEE_MemMove(dst, inter_buf, inter_buf_sz);
			dst_sz = inter_buf_sz;
			break;
		case TA_PERSIST_DEMO_CMD_3:
			;
			break;
		default:
			;
	}
	
	params[1].memref.size = dst_sz;

	EMSG("persist_demo TA exit####");
	/* Return with a status */
	return res;
}
