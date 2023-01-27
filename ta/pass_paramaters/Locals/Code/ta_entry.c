/*
 * Copyright (c) 2014, hehe.zhou@trustonic.com
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
#include "tee_internal_api.h"
#include "GP_sample_protocol.h"
#include "pass_paramaters_uuid.h"
#include "buildTag.h"

TEE_UUID uuid = pass_paramaters_UUID;
DECLARE_TRUSTED_APPLICATION_MAIN_STACK(16384)

#define EMSG(fmt, args...) TEE_LogPrintf("pass_paramaters "fmt"\n", ##args)

#else

#include <tee_ta_api.h>
#include <trace.h>
#include <tee_api.h>
#include <tee_api_defines.h>

#endif

#include "ta_pass_paramaters.h"

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

TEE_Result TA_InvokeCommandEntryPoint(void *session_id,
                                      uint32_t command_id,
                                      uint32_t param_types,
                                      TEE_Param params[4])
{
	(void)(session_id);
	(void)(params);
	uint8_t *in = NULL;
	uint8_t *out = NULL;

	EMSG("%s enter", __func__);

	switch (command_id) {
		case CMD_TEST_VALUE:
			EMSG("--CMD_TEST_VALUE--!");
			if ((TEE_PARAM_TYPE_GET(param_types, 0) != TEE_PARAM_TYPE_VALUE_INPUT ) ||
				(TEE_PARAM_TYPE_GET(param_types, 1) != TEE_PARAM_TYPE_VALUE_OUTPUT ) ||
				(TEE_PARAM_TYPE_GET(param_types, 2) != TEE_PARAM_TYPE_NONE) ||
				(TEE_PARAM_TYPE_GET(param_types, 3) != TEE_PARAM_TYPE_NONE)) {
				EMSG("The param_types is error!");
				return TEE_ERROR_BAD_PARAMETERS;
			}
			params[1].value.a = params[0].value.a + params[0].value.b;
			params[1].value.b = params[0].value.a * params[0].value.b;
			break;
		case CMD_TEST_MEMREF:
			EMSG("--CMD_TEST_MEMREF--!");
			if ((TEE_PARAM_TYPE_GET(param_types, 0) != TEE_PARAM_TYPE_MEMREF_INPUT ) ||
				(TEE_PARAM_TYPE_GET(param_types, 1) != TEE_PARAM_TYPE_MEMREF_OUTPUT) ||
				(TEE_PARAM_TYPE_GET(param_types, 2) != TEE_PARAM_TYPE_NONE) ||
				(TEE_PARAM_TYPE_GET(param_types, 3) != TEE_PARAM_TYPE_NONE)) {
				EMSG("The param_types is error!");
				return TEE_ERROR_BAD_PARAMETERS;
			}

			out = params[1].memref.buffer;
			in = params[0].memref.buffer;
			EMSG("in[0]=%d, in[1]=%d", in[0], in[1]);
			out[0] = in[0] + in[1];
			out[1] = in[0] * in[1];
			params[1].memref.size = 2;
			break;
		case CMD_TEST_TMPREF:
			EMSG("--CMD_TEST_TMPREF--!");
			if ((TEE_PARAM_TYPE_GET(param_types, 0) != TEE_PARAM_TYPE_MEMREF_INPUT) ||
				(TEE_PARAM_TYPE_GET(param_types, 1) != TEE_PARAM_TYPE_MEMREF_OUTPUT) ||
				(TEE_PARAM_TYPE_GET(param_types, 2) != TEE_PARAM_TYPE_NONE) ||
				(TEE_PARAM_TYPE_GET(param_types, 3) != TEE_PARAM_TYPE_NONE)) {
				EMSG("The param_types is error!");
				return TEE_ERROR_BAD_PARAMETERS;
			}
			
			out = params[1].memref.buffer;
			in = params[0].memref.buffer;
			out[0] = in[0] + in[1];
			out[1] = in[0] * in[1];
			params[1].memref.size = 2;
			break;
		default:
			;
	}

	EMSG("%s exit", __func__);

	return TEE_SUCCESS;
}
