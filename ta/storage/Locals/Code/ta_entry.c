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
#include "tee_internal_api_ext.h"
#include "GP_sample_protocol.h"
#include "storage_uuid.h"
#include "buildTag.h"

#define TEE_PROTECTED_STORAGE_ID	TEE_TT_STORAGE_PROTECTED

TEE_UUID uuid = storage_UUID;
DECLARE_TRUSTED_APPLICATION_MAIN_STACK(16384)

#define EMSG(fmt, args...) TEE_LogPrintf("storage "fmt"\n", ##args)

#else

#include <tee_ta_api.h>
#include <trace.h>
#include <tee_api.h>
#include <tee_api_defines.h>
#include <string.h>

// TODO： TEE_STORAGE_PROTECTED=0x00000003,Optee has not implemented this feature 
#define TEE_PROTECTED_STORAGE_ID	0x00000003

#endif

// storage ID ma be: 
//			TEE_STORAGE_PRIVATE
//			TEE_PROTECTED_STORAGE_ID


#include "ta_storage.h"

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




#define DELETE_FLAG	(TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE_META)
#define READ_FLAG	(TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_SHARE_READ)
#define WRITE_FLAG	(TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_OVERWRITE)
TEE_Result write_raw_object(char *obj_id,size_t obj_id_sz,char *data,size_t data_sz)
{
	TEE_ObjectHandle object;
	TEE_Result res;

	EMSG("%s : enter", __func__);

	res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE, obj_id, obj_id_sz, WRITE_FLAG, TEE_HANDLE_NULL, NULL, 0, &object);
	if (res != TEE_SUCCESS) {
		EMSG("TEE_CreatePersistentObject failed 0x%08x", res);
		return res;
	}

	res = TEE_WriteObjectData(object, data, data_sz);
	if (res != TEE_SUCCESS) {
		EMSG("TEE_WriteObjectData failed 0x%08x", res);
		TEE_CloseAndDeletePersistentObject1(object);
	} else {
		TEE_CloseObject(object);
	}
	
	EMSG("%s : exit", __func__);
	return res;
}

TEE_Result read_raw_object(char *obj_id,size_t obj_id_sz,char *data,size_t *data_sz)
{
	TEE_ObjectHandle object;
	TEE_ObjectInfo object_info;
	TEE_Result res;
	
	EMSG("%s : enter", __func__);

	res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, obj_id, obj_id_sz, READ_FLAG, &object);
	if (res != TEE_SUCCESS) {
		EMSG("Failed to open persistent object, res=0x%08x", res);
		return res;
	}

	res = TEE_GetObjectInfo1(object, &object_info);
	if (res != TEE_SUCCESS) {
		EMSG("Failed to create persistent object, res=0x%08x", res);
		TEE_CloseObject(object);
		return res;
	}

	if (object_info.dataSize > *data_sz) {
		TEE_CloseObject(object);
		return TEE_ERROR_SHORT_BUFFER;
	}

	res = TEE_ReadObjectData(object, data, object_info.dataSize, (uint32_t* )data_sz);
	if (res != TEE_SUCCESS || *data_sz != object_info.dataSize) {
		EMSG("Failed to TEE_ReadObjectData, res=0x%08x", res);
		TEE_CloseObject(object);
		return res;
	}

	TEE_CloseObject(object);
	
	EMSG("%s : exit", __func__);
	return res;
}

TEE_Result delete_object(char *obj_id,size_t obj_id_sz)
{
	TEE_ObjectHandle object;
	TEE_Result res;

	res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, obj_id, obj_id_sz, DELETE_FLAG, &object);
	if (res != TEE_SUCCESS) {
		EMSG("Failed to open persistent object, res=0x%08x", res);
		return res;
	}

	TEE_CloseAndDeletePersistentObject1(object);
	return res;
}

TEE_Result list_object(void)
{
	TEE_ObjectEnumHandle hEnumerator = TEE_HANDLE_NULL;
	TEE_Result nResult = TEE_SUCCESS;
	char     szName[64];
	TEE_ObjectInfo sObjectInfo;
	size_t   nPersistentObjectLength = 0;

	nResult = TEE_AllocatePersistentObjectEnumerator(&hEnumerator);
	
	TEE_ResetPersistentObjectEnumerator(hEnumerator);

	nResult = TEE_StartPersistentObjectEnumerator(hEnumerator, TEE_STORAGE_PRIVATE);

	do {
		nResult = TEE_GetNextPersistentObject(hEnumerator, &sObjectInfo, szName, &nPersistentObjectLength);
	
		szName[nPersistentObjectLength] = '\0';
		EMSG("%s Found file name: %s : %zd\n", __func__, szName, nPersistentObjectLength);
	} while (nResult != TEE_ERROR_ITEM_NOT_FOUND);
	
	TEE_FreePersistentObjectEnumerator(hEnumerator);
	
	return nResult;
}


void test_storeage(void)
{
	char buf[10] = {0};
	size_t buf_sz = 0;
	EMSG("-------------------------------------------------------(www)");
	write_raw_object("id1",3,"zhouhehe1",9);
	write_raw_object("id2",3,"zhouhehe2",9);
	write_raw_object("id3",3,"zhouhehe3",9);
	write_raw_object("id4",3,"zhouhehe4",9);
	write_raw_object("id5",3,"zhouhehe5",9);
	write_raw_object("id6",3,"zhouhehe6",9);
	write_raw_object("id7",3,"zhouhehe7",9);
	write_raw_object("id8",3,"zhouhehe8",9);
	write_raw_object("id9",3,"zhouhehe9",9);
	write_raw_object("ida",3,"zhouhehea",9);
	
	write_raw_object("idb",3,"zhouhehe7",9);
	write_raw_object("idc",3,"zhouhehe8",9);
	write_raw_object("idd",3,"zhouhehe9",9);
	write_raw_object("ide",3,"zhouhehea",9);
	
	list_object();
	
	EMSG("==================================================(www)");
	delete_object("id4",3);
	delete_object("id5",3);
	list_object();

	buf_sz = 10;
	read_raw_object("id1",3,buf,&buf_sz);
	EMSG("%s--buf=%s",__func__,buf);
}

#define TEST_SIZE	(10)
TEE_Result TA_InvokeCommandEntryPoint(void *session_id,
                                      uint32_t command_id,
                                      uint32_t param_types,
                                      TEE_Param parameters[4])
{
	(void)(session_id);
	(void)(parameters);

	EMSG("%s enter", __func__);

	if ((TEE_PARAM_TYPE_GET(param_types, 0) != TEE_PARAM_TYPE_MEMREF_INOUT) ||
		(TEE_PARAM_TYPE_GET(param_types, 1) != TEE_PARAM_TYPE_MEMREF_OUTPUT) ||
		(TEE_PARAM_TYPE_GET(param_types, 2) != TEE_PARAM_TYPE_NONE) ||
		(TEE_PARAM_TYPE_GET(param_types, 3) != TEE_PARAM_TYPE_NONE)) {
		EMSG("The param_types is error!");
		return TEE_ERROR_BAD_PARAMETERS;
	}

	switch (command_id) {
		case TA_STORAGE_CMD_1:
			EMSG("--TA_STORAGE_CMD_1--!");
			test_storeage();
			;
		case TA_STORAGE_CMD_2:
			;
		case TA_STORAGE_CMD_3:
			;
		default:
			;
	}

	EMSG("%s exit", __func__);

	return TEE_SUCCESS;
}
