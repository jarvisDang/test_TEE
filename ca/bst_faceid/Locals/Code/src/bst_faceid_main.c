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
#include <getopt.h> 

#include <tee_client_api.h>

#include "buildTag.h"


char *_device = NULL;

//#define COOSEA_MTK_BSP

#ifdef COOSEA_MTK_BSP
#include "bstfaceid.h"
#include <log/log.h>

#define LOG_I ALOGD

#else 
#include "log.h"

#define CANNOT_SKIP_MAIN_FUNCTION
#endif


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

#include <ta_bst_faceid.h>

const TEEC_UUID bst_faceid_user_ta_uuid = TA_BST_FACEID_UUID;
TEEC_Context teec_ctx;

TEEC_Result bst_faceid_teec_opensession(TEEC_Session *session,
				    const TEEC_UUID *uuid, TEEC_Operation *op,
				    uint32_t *ret_orig)
{
	return TEEC_OpenSession(&teec_ctx, session, uuid,
				TEEC_LOGIN_PUBLIC, NULL, op, ret_orig);
}

TEEC_Result bst_faceid_teec_closession(TEEC_Session *session)
{
	(void)session;
	TEEC_CloseSession(session);
	return 0;
}


#define FEATURE_DATA_LENGH	(128*4+4)
typedef struct __attribute__((__packed__)) {
    uint8_t version;  // Current version is 0
    uint64_t challenge;
    uint64_t user_id;             // secure user ID, not Android user ID
    uint64_t authenticator_id;    // secure authenticator ID
    uint32_t authenticator_type;  // hw_authenticator_type_t, in network order
    uint64_t timestamp;           // in network order
    uint8_t hmac[32];
} hw_auth_token_t;

typedef struct bst_feature {
	float feature[128];
	int size;
} bst_feature_t;

struct InOutParams
{
	bst_feature_t feature_data;
	uint32_t fid;
	uint64_t challenge;
	hw_auth_token_t token;
	uint8_t result[8];
};

int32_t addFeature(const char* feature_data, int* fid,const hw_auth_token_t *hat)
{
#ifndef USE_TRUSTONIC
	TEEC_Session session = { .ctx = 0 };
#else
	TEEC_Session session;
#endif
	TEEC_Operation op = { 0 };
	uint32_t ret;
	struct InOutParams param;
	struct InOutParams *r_param;
	
	LOG_E("%s:%d  enter \n", __func__, __LINE__);
	
	memset(&param, 0, sizeof(struct InOutParams));
	memcpy(&param.feature_data, feature_data, sizeof(bst_feature_t));  // TODO sizeof(bst_feature_t) must be 128*4+4
	memcpy(&param.token, hat, sizeof(hw_auth_token_t));
	param.fid = 0;
	param.challenge = 0;

	bst_faceid_teec_opensession(&session, &bst_faceid_user_ta_uuid, NULL, NULL);

	
	
	printf("bst_faceid_teec_invoke  \n");

	op.params[0].tmpref.buffer = &param;
	op.params[0].tmpref.size = sizeof(struct InOutParams);
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT,
									TEEC_NONE,
									TEEC_NONE,
									TEEC_NONE);

	ret = TEEC_InvokeCommand(&session,
						CMD_FACE_ADD,
						&op,
						NULL);

	r_param = (struct InOutParams *)op.params[0].tmpref.buffer;
	
	*fid = r_param->fid;
	
	printf("ret=0x%x\n, fid=0x%08x\n", ret, *fid);
	
	bst_faceid_teec_closession(&session);
	
	return ret;
}
int32_t removeFeature(uint32_t fid)
{
#ifndef USE_TRUSTONIC
	TEEC_Session session = { .ctx = 0 };
#else
	TEEC_Session session;
#endif
	TEEC_Operation op = { 0 };
	uint32_t ret;
	struct InOutParams param;
	
	LOG_E("%s:%d  enter \n", __func__, __LINE__);

	memset(&param, 0, sizeof(struct InOutParams));
	param.fid = fid;

	bst_faceid_teec_opensession(&session, &bst_faceid_user_ta_uuid, NULL, NULL);

	printf("bst_faceid_teec_invoke  \n");

	op.params[0].tmpref.buffer = &param;
	op.params[0].tmpref.size = sizeof(struct InOutParams);
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT,
									TEEC_NONE,
									TEEC_NONE,
									TEEC_NONE);

	ret = TEEC_InvokeCommand(&session,
						CMD_FACE_REMOVE,
						&op,
						NULL);
	
	printf("ret=0x%x\n\n", ret);
	
	bst_faceid_teec_closession(&session);
	
	return ret;
}
int32_t compare(const char* feature_data)
{
	#ifndef USE_TRUSTONIC
	TEEC_Session session = { .ctx = 0 };
#else
	TEEC_Session session;
#endif
	TEEC_Operation op = { 0 };
	uint32_t ret;
	struct InOutParams param;
	
	LOG_E("%s:%d  enter \n", __func__, __LINE__);
	
	memset(&param, 0, sizeof(struct InOutParams));
	memcpy(&param.feature_data, feature_data,  sizeof(bst_feature_t));  // TODO sizeof(bst_feature_t) must be 128*4+4
	param.fid = 0;
	param.challenge = 0;

	bst_faceid_teec_opensession(&session, &bst_faceid_user_ta_uuid, NULL, NULL);

	
	
	printf("bst_faceid_teec_invoke  \n");

	op.params[0].tmpref.buffer = &param;
	op.params[0].tmpref.size = sizeof(struct InOutParams);
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT,
									TEEC_NONE,
									TEEC_NONE,
									TEEC_NONE);

	ret = TEEC_InvokeCommand(&session,
						CMD_FACE_COMPARE,
						&op,
						NULL);
	
	printf("ret=0x%x\n\n", ret);
	
	bst_faceid_teec_closession(&session);
	
	return ret;
	
}

uint64_t get_challenge(void)
{
#ifndef USE_TRUSTONIC
	TEEC_Session session = { .ctx = 0 };
#else
	TEEC_Session session;
#endif
	TEEC_Operation op = { 0 };
	uint32_t ret;
	struct InOutParams param;
	struct InOutParams *r_param;
	
	LOG_E("%s:%d  enter \n", __func__, __LINE__);
	
	memset(&param, 0, sizeof(struct InOutParams));

	bst_faceid_teec_opensession(&session, &bst_faceid_user_ta_uuid, NULL, NULL);

	
	
	printf("bst_faceid_teec_invoke  \n");

	op.params[0].tmpref.buffer = &param;
	op.params[0].tmpref.size = sizeof(struct InOutParams);
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT,
									TEEC_NONE,
									TEEC_NONE,
									TEEC_NONE);

	ret = TEEC_InvokeCommand(&session,
						CMD_FACE_GET_CHALLENGE,
						&op,
						NULL);

	r_param = (struct InOutParams *)op.params[0].tmpref.buffer;
	
	printf("ret=0x%x\n, challenge=0x%lx\n", ret, (unsigned long)r_param->challenge);
	
	bst_faceid_teec_closession(&session);
	
	return r_param->challenge;
}



int32_t open_tee_faceid()
{
	TEEC_InitializeContext(_device, &teec_ctx);
	return 0;
	
}
void close_tee_faceid()
{
	TEEC_FinalizeContext(&teec_ctx);
}

// ====================== for test ==================================


#ifdef CANNOT_SKIP_MAIN_FUNCTION

uint32_t type;
uint32_t g_fid;
uint32_t test_flag;
int parse_paras(int argc,char *argv[])       
{      
	int ch;       
	opterr=0;
	printf("argc=%d, argv=%s\n", argc, argv[1]);
	while((ch=getopt(argc,argv,"ed:cvt:"))!=-1)       
	{       
		printf("optind:%d\n",optind);       
		printf("optarg:%s\n",optarg);       
		printf("ch:%c\n",ch);       
		switch(ch)     
		{       
			case 'e':      //add or enroll
				type = CMD_FACE_ADD;
				break;       
			case 'd':      //delete or remove
				type = CMD_FACE_REMOVE;
				printf("11 option e:%s\n",optarg);
				sscanf(optarg, "0x%x", &g_fid);
				printf("g_fid = 0x%08x\n",g_fid);
				break;
			case 'c':      //get challenge
				type = CMD_FACE_GET_CHALLENGE;
				break;       
			case 'v':      //verify or compare
				type = CMD_FACE_COMPARE;
				break;   
			case 't':      //test
				type = CMD_FACE_TEST;
				printf("option e:%s\n",optarg);
				sscanf(optarg, "0x%x", &test_flag);  //0x1-ca_test, 0x2-ta_test
				printf("test_flag = 0x%08x\n",test_flag);
				break; 
			default:       
				printf("Knowed option:%c\n",ch);
		}
		printf("optopt+%c\n",optopt);       
	}
 
	return 0;
}

#define FILE_LENGTH	(512)
int getFileContent(char* path, char *buf)
{
	FILE*   pStream;
	
	/* Open the file */
	pStream = fopen(path, "rb");
	if (pStream == NULL)
	{
		fprintf(stderr, "Error: Cannot open file: %s.\n", path);
		return -1;
	}

	
	/* Read data from the file into the buffer */
	if (fread(buf, FILE_LENGTH, 1, pStream) != 1)
	{
		fprintf(stderr, "Error: Cannot read file: %s.\n", path);
		fclose(pStream);
		return -1;
	}
	
	/* Close the file */
	fclose(pStream);
	
	return 0;
}

int32_t ta_test()
{
#ifndef USE_TRUSTONIC
	TEEC_Session session = { .ctx = 0 };
#else
	TEEC_Session session;
#endif
	TEEC_Operation op = { 0 };
	uint32_t ret;
	struct InOutParams param;
	
	memset(&param, 0, sizeof(struct InOutParams));

	bst_faceid_teec_opensession(&session, &bst_faceid_user_ta_uuid, NULL, NULL);

	
	
	printf("bst_faceid_teec_invoke  \n");

	op.params[0].tmpref.buffer = &param;
	op.params[0].tmpref.size = sizeof(struct InOutParams);
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT,
									TEEC_NONE,
									TEEC_NONE,
									TEEC_NONE);

	ret = TEEC_InvokeCommand(&session,
						CMD_FACE_TEST,
						&op,
						NULL);
	
	printf("ret=0x%x\n\n", ret);
	
	bst_faceid_teec_closession(&session);
	
	return ret;
	
}

int main(int argc, char *argv[])
{
	
	hw_auth_token_t token;
	uint32_t fid = 0;
	uint32_t challenge;
	bst_feature_t feature_data;

	//printf("\nbst_faceid CA enter device=[%s] 11111111111\n", _device);
	printf("\nbst_faceid CA enter \n");

	open_tee_faceid();
	
	parse_paras(argc,argv);
	printf("type=0x%x\n", type);

	switch(type)
	{
		case CMD_FACE_ADD:
			printf("=====CMD_FACE_ADD====\n");
			memset(&token, 0, sizeof(hw_auth_token_t));
			memset(&feature_data, 0xa5, sizeof(feature_data));
		
			addFeature((void *)&feature_data, (int *)&fid, &token);
			printf("fid = %d\n", fid);
			break;
		case CMD_FACE_REMOVE:
			removeFeature(g_fid);
			;
			break;
		case CMD_FACE_COMPARE:
			printf("=====CMD_FACE_COMPARE====\n");
			memset(&feature_data, 0xa5, sizeof(feature_data));
			compare((void *)&feature_data);
			break;
		case CMD_FACE_GET_CHALLENGE:
			challenge = get_challenge();
			printf("111  challenge = 0x%lx\n", (unsigned long)challenge);
			;
			break;
		case CMD_FACE_TEST:
			switch(test_flag)
			{
				case 1:  // ca testcase
					memset(&token, 0, sizeof(hw_auth_token_t));
					memset(&feature_data, 0x00, sizeof(feature_data));
					getFileContent("save_feature.bin", (char *)&feature_data);
					addFeature((void *)&feature_data, (int *)&fid, &token);
					
					getFileContent("new_feature.bin", (char *)&feature_data);
					compare((void *)&feature_data);
					
					memset(&feature_data, 0xa5, sizeof(feature_data));
					compare((void *)&feature_data);
					break;
				case 2: // ta testcase
					ta_test();
					break;
			}
			;
			break;
		default:
			printf("=====defult====\n");
			break;
	}	

	close_tee_faceid();
	printf("bst_faceid CA exit!\n");
	return 0;
}

#endif

