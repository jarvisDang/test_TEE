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
#include "cata_stress_uuid.h"
#include "buildTag.h"

#include "drAsyncExample_Api.h"
#include "tlAsyncExampleDriverApi.h"

TEE_UUID uuid = cata_stress_UUID;
DECLARE_TRUSTED_APPLICATION_MAIN_STACK(16384)

#define EMSG(fmt, args...) TEE_LogPrintf("cata_stress "fmt"\n", ##args)

#else

#include <tee_ta_api.h>
#include <trace.h>
#include <tee_api.h>
#include <tee_api_defines.h>

#endif

#include "ta_cata_stress.h"

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
	(void)(param_types);
	
	uint32_t count;
	uint32_t i;
	
	count = params[0].value.a;

	//EMSG("%s enter", __func__);

	

	switch (command_id){
		case TEST_CATA_COMMUNICATION:
			//EMSG("--TA_CATA_STRESS_CMD_1--!");
			return TEE_SUCCESS;
		case TEST_TADRIVER_COMMUNICATION:
			EMSG("--count=%d--!",count);
#ifdef USE_TRUSTONIC
			for(i=0;i<count;i++)
				tlApi_Async_OpenSession();
#endif
			break;
		default:
			break;
	}

	//EMSG("%s exit", __func__);

	return TEE_SUCCESS;
}
