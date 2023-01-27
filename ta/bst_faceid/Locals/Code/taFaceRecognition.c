/*
 * Copyright (c) 2018 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain actual_read_lengthries.
 */

/* ----------------------------------------------------------------------------
 *   Includes
 * ---------------------------------------------------------------------------- */
#include <stdarg.h>
#include <string.h>
#include <taStd.h>
#include <float.h>
#include <taFaceRecognition.h>

#define SECOBJ_FILENAME "FeatureFile"
//#define TEST_VER 1
#define TEST_NUM 5000
unsigned int test_counter = 0;
TEE_Result taWriteFeatures(uint8_t* pInput, uint32_t nInputSize)
{
	TEE_Result nResult = TEE_ERROR_GENERIC;
	unsigned char objectID[] = SECOBJ_FILENAME;
	TEE_ObjectHandle object = TEE_HANDLE_NULL;
	unsigned char *write_data = NULL;
	uint32_t write_length = nInputSize;
	
#ifdef TEST_VER
	if (test_counter == TEST_NUM)
	{
		nResult = TEE_ERROR_GENERIC;
        TEE_DbgPrintLnf(TATAG "This is a test version, please reboot for use it again.");
		goto write_exit;
    }
	else
		test_counter++;
#endif

	TEE_DbgPrintLnf(TATAG "taWriteFeatures enter");
	if (pInput == NULL)
	{
		TEE_DbgPrintLnf(TATAG "Bad parameter in taWriteFeatures()");
		goto write_exit;
	}

	nResult =
		TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE, objectID, sizeof (objectID),
		TEE_HANDLE_FLAG_PERSISTENT | TEE_DATA_FLAG_ACCESS_WRITE, NULL, NULL, 0, &object);

	if (nResult != TEE_SUCCESS)
	{
		TEE_DbgPrintLnf(TATAG "TEE_CreatePersistentObject was not able to create object, it returns 0x%x ", nResult);
		goto write_exit;
	}
	TEE_DbgPrintLnf(TATAG "TEE_CreatePersistentObject success");

	write_data = TEE_Malloc(nInputSize, 0);
	if (!write_data)
	{
		nResult = TEE_ERROR_BAD_PARAMETERS;
		TEE_DbgPrintLnf(TATAG "malloc failed\n");
		goto write_exit;
	}
	
	memcpy(write_data, pInput , nInputSize);
	//TEE_DbgPrintLnf(TATAG "write_data[0] = 0x%x, write_data[1] = 0x%x, write_length = %d", write_data[0], write_data[1], write_length);//debug use
	nResult = TEE_WriteObjectData (object, write_data, write_length);
	if (TEE_SUCCESS == nResult)
	{
		TEE_DbgPrintLnf(TATAG "TEE_WriteObjectData success");
	}
	else
	{
		TEE_DbgPrintLnf(TATAG "TEE_WriteObjectData failed %x", nResult);
		goto write_exit;
	}
	TEE_DbgPrintLnf(TATAG "taWriteFeatures success");
	
write_exit:
	if (object)
		TEE_CloseObject(object);
	if (write_data)
		TEE_Free(write_data);	
	TEE_DbgPrintLnf(TATAG "taWriteFeatures exit");
	return nResult;
}

TEE_Result taReadFeatures(uint8_t* pOutput, uint32_t* nOutputSize)
{
	TEE_Result nResult = TEE_ERROR_GENERIC;
	unsigned char objectID[] = SECOBJ_FILENAME;
	TEE_ObjectHandle object = TEE_HANDLE_NULL;	
	unsigned char *read_data = NULL;
	uint32_t actual_read_length = 0;
	uint32_t read_length = 0;
	
#ifdef TEST_VER
	if (test_counter == TEST_NUM)
	{
		nResult = TEE_ERROR_GENERIC;
        TEE_DbgPrintLnf(TATAG "This is a test version, please reboot for use it again.");
		goto read_exit;
    }
	else
		test_counter++;	
#endif

	TEE_DbgPrintLnf(TATAG "taReadFeatures enter\n");

	if ((pOutput == NULL) || (nOutputSize == NULL))
	{
		TEE_DbgPrintLnf(TATAG "Bad parameter in taReadFeatures()");
		goto read_exit;
	}
	read_length = *nOutputSize;
	TEE_DbgPrintLnf(TATAG "read_length = %d\n", read_length);

 	//Open persistent object for read
	nResult =
		TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, objectID, sizeof (objectID),
		TEE_HANDLE_FLAG_PERSISTENT | TEE_DATA_FLAG_ACCESS_READ, &object);

	if (TEE_SUCCESS == nResult)
	{
		TEE_DbgPrintLnf(TATAG "TEE_OpenPersistentObject success");
	}
	else
	{
		TEE_DbgPrintLnf(TATAG "TEE_OpenPersistentObject failed, please make sure it writes(creates) the file before.");
		goto read_exit;
	}

	read_data = TEE_Malloc(read_length, 0);
	if (!read_data)
	{
		nResult = TEE_ERROR_BAD_PARAMETERS;
		TEE_DbgPrintLnf(TATAG "malloc failed\n");
		goto read_exit;
	}
	
	nResult = TEE_ReadObjectData(object, read_data, read_length, &actual_read_length);

    if (nResult != TEE_SUCCESS)
	{
		TEE_DbgPrintLnf(TATAG "TEE_ReadObjectData fail, nResult = 0x%x", nResult);		
		goto read_exit;
    }

	if (actual_read_length != read_length) //Check if read actual_read_length is correct
	{
		nResult = TEE_ERROR_GENERIC;
		TEE_DbgPrintLnf(TATAG "TEE_ReadObjectData fail");
		TEE_DbgPrintLnf(TATAG "actual_read_length = %d B", actual_read_length);
		goto read_exit;
	}
	
	memcpy(pOutput, read_data , read_length);
	*nOutputSize = read_length;	
	TEE_DbgPrintLnf(TATAG "taReadFeatures success");
	
read_exit:
	if(object)
		TEE_CloseObject(object);
	if(read_data)
		TEE_Free(read_data);
	TEE_DbgPrintLnf(TATAG "taReadFeatures exit\n");
	return nResult;
}

#ifdef	IS_HALF_FLAG_ENABLE
TEE_Result taCompareFeatures(uint8_t* pInput, uint32_t nInputSize, uint8_t* pOutput, uint32_t* nOutputSize, uint8_t* pSecondInput, uint32_t nSecondInputSize)
#else
TEE_Result taCompareFeatures(uint8_t* pInput, uint32_t nInputSize, uint8_t* pOutput, uint32_t* nOutputSize)
#endif
{

	TEE_Result nResult = TEE_ERROR_GENERIC;
	unsigned char objectID[] = SECOBJ_FILENAME;
	int index = -1;
	uint32_t actual_read_length = 0;
	uint32_t compared_features_length = sizeof(MGFeature);
	
#ifdef TEST_VER
	if (test_counter == TEST_NUM)
	{
		nResult = TEE_ERROR_GENERIC;
        TEE_DbgPrintLnf(TATAG "This is a test version, please reboot for use it again.");
		goto compare_exit;
    }
	else
		test_counter++;		
#endif

	MGFeature *compared_features = (MGFeature *)TEE_Malloc(sizeof(MGFeature), 0);
	if (!compared_features)
	{
		TEE_DbgPrintLnf(TATAG "malloc failed\n");
		goto compare_exit;
	}	
	
	MGULKSavedFeature *target_feature = (MGULKSavedFeature *)TEE_Malloc(nInputSize, 0);
	if (!target_feature)
	{
		TEE_DbgPrintLnf(TATAG "malloc failed\n");
		goto compare_exit;
	}	
#ifdef	IS_HALF_FLAG_ENABLE
	uint32_t is_half = *pSecondInput;
#endif	
    float min_l2_distance = FLT_MAX;
    float compareScore = FLT_MAX;
	MGULKTeeFaceCompareScore score = {0};
	MGULKSavedFeature* saved_feature = NULL;
	MGULKStatus compare_ret = MG_UNLOCK_FAILED;
	MGULKCompareResult* result = (MGULKCompareResult *)TEE_Malloc(*nOutputSize, 0);
	if (!result)
	{
		TEE_DbgPrintLnf(TATAG "malloc failed\n");
		goto compare_exit;
	}
	
	TEE_DbgPrintLnf(TATAG "taCompareFeatures enter\n");
	//Read features
	TEE_DbgPrintLnf(TATAG "compared_features_length = %d\n", compared_features_length);
 	taReadFeatures((uint8_t *)compared_features, &compared_features_length);
	//TEE_DbgPrintLnf(TATAG "compared_features->ids[0]:%d compared_features->features[0].version:0x%x",
	//					 compared_features->ids[0], compared_features->features[0].version);//debug use
	
	//Compare features
	memcpy(target_feature, (MGULKSavedFeature *)pInput , nInputSize);
	//TEE_DbgPrintLnf(TATAG "target_feature[0]:0x%x, target_feature[1]:0x%x",
	//					 target_feature[0], target_feature[1]);//debug use
	
	for (int i = 0; i < MAX_FEATURE_COUNT; i++) 
	{
		if (compared_features->ids[i] == 0)
			continue;

		//TEE_DbgPrintLnf(TATAG "compared_features->ids[%d]:%d compared_features->features[%d].version:0x%x",
		//				 i, compared_features->ids[i], i, compared_features->features[i].version);//debug use		
		
		saved_feature = &(compared_features->features[i]);
		compare_ret = FaceCompare(saved_feature, target_feature, &score);		
		result->compare_ret = compare_ret;
#ifndef IS_HALF_FLAG_ENABLE
		index = i;
		result->score.l2_full = score.l2_full;
		result->score.l2_half = score.l2_half;
		result->score.pd_score = score.pd_score;
		result->score.face_id = index;
#else
		is_half = is_half && target_feature->has_half;
	
		//Calculate score only at successful comparison
		if (compare_ret == MG_UNLOCK_OK) 
		{
			TEE_DbgPrintLnf(TATAG "face_search compare result:%d score0:%f,score1:%f",
						 compare_ret, score.l2_full, score.l2_half);
			compareScore = !is_half ? score.l2_full : score.l2_half;
			if (compareScore < min_l2_distance) 
			{
				min_l2_distance = compareScore;
				index = i;
				result->score.l2_full = score.l2_full;
				result->score.l2_half = score.l2_half;
				result->score.pd_score = score.pd_score;
				result->score.face_id = index;
			}
		} 
		else if (compare_ret == MG_UNLOCK_NEED_RESTORE_FEATURE) 
		{
			if(compared_features)
				TEE_Free(compared_features);
			if(target_feature)
				TEE_Free(target_feature);
			if(result)
				TEE_Free(result);				
			//Send notification for restore feature
			return compare_ret;
		}
#endif		
	}

	memcpy(pOutput, result, *nOutputSize);
	TEE_DbgPrintLnf(TATAG "taCompareFeatures success");
	
compare_exit:
	if(compared_features)
		TEE_Free(compared_features);
	if(target_feature)
		TEE_Free(target_feature);
	if(result)
		TEE_Free(result);	
	TEE_DbgPrintLnf(TATAG "taCompareFeatures exit, index = %d\n", index);
	return index == -1 ? MG_UNLOCK_COMPARE_FAILURE : MG_UNLOCK_OK;
}

TEE_Result taAddFeature(uint8_t* pInput, uint32_t nInputSize, uint8_t* pOutput, uint32_t* nOutputSize)
{

	TEE_Result nResult = TEE_ERROR_GENERIC;
	unsigned char objectID[] = SECOBJ_FILENAME;
	TEE_ObjectHandle object = TEE_HANDLE_NULL;	
	int index = -1;
	unsigned int i = 0;
	uint32_t actual_read_length = 0;
	uint32_t features_length = sizeof(MGFeature);
	
#ifdef TEST_VER
	if (test_counter == TEST_NUM)
	{
		nResult = TEE_ERROR_GENERIC;
        TEE_DbgPrintLnf(TATAG "This is a test version, please reboot for use it again.");
		goto add_exit;
    }
	else
		test_counter++;		
#endif

	MGFeature *features = (MGFeature *)TEE_Malloc(sizeof(MGFeature), 0);
	if (!features)
	{
		TEE_DbgPrintLnf(TATAG "malloc failed\n");
		goto add_exit;
	}	
	
	TEE_DbgPrintLnf(TATAG "taAddFeature enter\n");
	
	//Read features
	TEE_DbgPrintLnf(TATAG "features_length = %d\n", features_length);
 	nResult = taReadFeatures((uint8_t *)features, &features_length);
	if (nResult == TEE_SUCCESS)
	{		
		//TEE_DbgPrintLnf(TATAG "features->ids[0]:%d features->features[0].version:0x%x",
		//					 features->ids[0], features->features[0].version);//Debug use
						 
		for (i = 0; i < MAX_FEATURE_COUNT; i++) 
		{
			//Add features
			if (features->ids[i] == 0) 
			{
				memcpy(&(features->features[i]), pInput, nInputSize);
				features->ids[i] = 1;
				*pOutput = i;
				nResult = TEE_SUCCESS;
				break;
			}
		}
		
		//Can't find an available space 
		if (i == (MAX_FEATURE_COUNT + 1)) 		
		{	
			TEE_DbgPrintLnf(TATAG "taAddFeature fail");
			nResult = TEE_ERROR_GENERIC;
			goto add_exit;
		}	
		
		//Write features
		taWriteFeatures((uint8_t *)features, features_length);
		TEE_DbgPrintLnf(TATAG "taAddFeature success");
	}

add_exit:
	if(features)
		TEE_Free(features);
	TEE_DbgPrintLnf(TATAG "taAddFeature exit\n");	
	return nResult;
}

TEE_Result taUpdateFeature(uint8_t* pInput, uint32_t nInputSize, uint8_t* pSecondInput, uint32_t nSecondInputSize)
{
	TEE_Result nResult = TEE_ERROR_GENERIC;
	unsigned char objectID[] = SECOBJ_FILENAME;
	MGFeature *read_data = NULL;	
	MGFeature *update_data = NULL;
	uint32_t read_length = sizeof(MGFeature);
	uint32_t id = *pSecondInput;
	void *update_feature = NULL;
	
#ifdef TEST_VER	
	if (test_counter == TEST_NUM)
	{
		nResult = TEE_ERROR_GENERIC;
        TEE_DbgPrintLnf(TATAG "This is a test version, please reboot for use it again.");
		goto update_exit;
    }
	else
		test_counter++;	
#endif

	TEE_DbgPrintLnf(TATAG "taUpdateFeature enter\n");

 	//Read features
	read_data = (MGFeature *)TEE_Malloc(sizeof(MGFeature), 0);
	if (!read_data)
	{
		TEE_DbgPrintLnf(TATAG "malloc failed\n");
		goto update_exit;
	}	

	TEE_DbgPrintLnf(TATAG "read_length = %d\n", read_length);
 	nResult = taReadFeatures((uint8_t *)read_data, &read_length);
	if (nResult != TEE_SUCCESS)
		goto update_exit;
	
	//Update features
	update_data = (MGFeature *)read_data;
	update_data->ids[id] = 1;
	update_feature = (void *)&(update_data->features[id]);
	memcpy(update_feature, pInput, nInputSize);	

	//Write features
	nResult = taWriteFeatures((uint8_t *)update_data, read_length);
	if (nResult != TEE_SUCCESS)
		goto update_exit;
	else
		TEE_DbgPrintLnf(TATAG "taUpdateFeature success");
	
update_exit:
	if(read_data)
		TEE_Free(read_data);
	TEE_DbgPrintLnf(TATAG "taUpdateFeature exit\n");
	return nResult;
}

TEE_Result taRemoveFeature(uint8_t* pInput, uint32_t nInputSize)
{
	TEE_Result nResult = TEE_ERROR_GENERIC;
	unsigned char objectID[] = SECOBJ_FILENAME;
	MGFeature *read_data = NULL;	
	MGFeature *update_data = NULL;
	uint32_t read_length = sizeof(MGFeature);
	uint32_t id = *pInput;
	void *update_feature = NULL;
	
#ifdef TEST_VER
	if (test_counter == TEST_NUM)
	{
		nResult = TEE_ERROR_GENERIC;
        TEE_DbgPrintLnf(TATAG "This is a test version, please reboot for use it again.");
		goto update_exit;
    }
	else
		test_counter++;	
#endif

	TEE_DbgPrintLnf(TATAG "taRemoveFeature enter\n");

 	//Read features
	read_data = (MGFeature *)TEE_Malloc(sizeof(MGFeature), 0);
	if (!read_data)
	{
		TEE_DbgPrintLnf(TATAG "malloc failed\n");
		goto update_exit;
	}	

	TEE_DbgPrintLnf(TATAG "read_length = %d\n", read_length);
 	nResult = taReadFeatures((uint8_t *)read_data, &read_length);
	if (nResult != TEE_SUCCESS)
		goto update_exit;
	
	//Remove feature
	update_data = (MGFeature *)read_data;
	update_data->ids[id] = 0;
	update_feature = (void *)&(update_data->features[id]);
	memset(update_feature, 0, sizeof(MGULKSavedFeature));	

	//Write features
	nResult = taWriteFeatures((uint8_t *)update_data, read_length);
	if (nResult != TEE_SUCCESS)
		goto update_exit;
	else
		TEE_DbgPrintLnf(TATAG "taRemoveFeature success");
	
update_exit:
	if(read_data)
		TEE_Free(read_data);
	TEE_DbgPrintLnf(TATAG "taRemoveFeature exit\n");
	return nResult;
}

TEE_Result taGetFeatureCount(uint8_t* pOutput, uint32_t* nOutputSize)
{
	TEE_Result nResult = TEE_ERROR_GENERIC;
    int count = 0;
	MGFeature *read_data = NULL;
	uint32_t read_length = sizeof(MGFeature);
	*pOutput = 0;

	TEE_DbgPrintLnf(TATAG "taGetFeatureCount enter\n");	
 	//Read features
	read_data = (MGFeature *)TEE_Malloc(sizeof(MGFeature), 0);
	if (!read_data)
	{
		TEE_DbgPrintLnf(TATAG "malloc failed\n");
		goto count_exit;
	}	

	TEE_DbgPrintLnf(TATAG "read_length= %d\n", read_length);
 	nResult = taReadFeatures((uint8_t *)read_data, &read_length);
	if (nResult != TEE_SUCCESS)
		goto count_exit;

    for (int i = 0; i < MAX_FEATURE_COUNT; i++) 
	{
        if (read_data->ids[i] == 1) 
		{
            count++;
        }
    }
	
count_exit:
	if(read_data)
		TEE_Free(read_data);
	TEE_DbgPrintLnf(TATAG "taGetFeatureCount count = %d\n", count);
	*pOutput = count;
	return nResult;
}

TEE_Result taFeatureIsValid(uint8_t* pInput, uint32_t nInputSize, uint8_t* pOutput, uint32_t* nOutputSize)
{
	
	TEE_Result nResult = TEE_ERROR_GENERIC;
    int count = 0;
	MGFeature *read_data = NULL;
	uint32_t read_length = sizeof(MGFeature);
	uint32_t id = *pInput;
	*pOutput = -1;

	TEE_DbgPrintLnf(TATAG "taFeatureIsValid enter\n");	
 	//Read features
	read_data = (MGFeature *)TEE_Malloc(sizeof(MGFeature), 0);
	if (!read_data)
	{
		TEE_DbgPrintLnf(TATAG "malloc failed\n");
		goto valid_exit;
	}	


 	nResult = taReadFeatures((uint8_t *)read_data, &read_length);
	if (nResult != TEE_SUCCESS)
		goto valid_exit;

    if (id > MAX_FEATURE_COUNT - 1 || (read_data->ids[id] == 0)) 
	{
		*pOutput = -1;
    }
	else
		*pOutput = 0;
	
valid_exit:		
	TEE_DbgPrintLnf(TATAG "taFeatureIsValid exit\n");
	return nResult;	
}

TEE_Result RunFaceRecognitionCases(uint32_t CmdID, uint8_t* pInput, uint32_t nInputSize, uint8_t* pOutput, uint32_t* nOutputSize, uint8_t* pSecondInput, uint32_t nSecondInputSize)
{
	TEE_Result ret = TEE_ERROR_GENERIC;
	
	switch (CmdID)
	{
    	case CMD_FEATURE_WRITE:
      		ret = taWriteFeatures(pInput, nInputSize);
    		break;
    	case CMD_FEATURE_READ:
    		ret = taReadFeatures(pOutput, nOutputSize);
    		break;
		case CMD_FEATURE_COMPARE:
#ifdef	IS_HALF_FLAG_ENABLE
    		ret = taCompareFeatures(pInput, nInputSize, pOutput, nOutputSize, pSecondInput, nSecondInputSize);
#else		
    		ret = taCompareFeatures(pInput, nInputSize, pOutput, nOutputSize);
#endif		
    		break;
		case CMD_FEATURE_ADD:
    		ret = taAddFeature(pInput, nInputSize, pOutput, nOutputSize);
    		break;
		case CMD_FEATURE_UPDATE:
    		ret = taUpdateFeature(pInput, nInputSize, pSecondInput, nSecondInputSize);
    		break;	
		case CMD_FEATURE_REMOVE:
    		ret = taRemoveFeature(pInput, nInputSize);
    		break;
		case CMD_FEATURE_COUNT:
    		ret = taGetFeatureCount(pOutput, nOutputSize);
    		break;
		case CMD_FEATURE_VALID:
    		ret = taFeatureIsValid(pInput, nInputSize, pOutput, nOutputSize);
    		break;			
    	default:
    		TEE_DbgPrintLnf(TATAG "RunFaceRecognitionCases() can NOT recognize CMDID = 0x%x ....................", CmdID);
    		break;
	}
	TEE_DbgPrintLnf(TATAG "RunFaceRecognitionCases(), CMDID = 0x%x, ret= 0x%x", CmdID, ret);
	return ret;
}

TEE_Result CheckInputParameter(uint8_t* pInput, uint32_t nInputSize)
{
	TEE_Result ret = TEE_ERROR_GENERIC;
	
	if ((pInput == NULL) || (nInputSize == 0))
	{
		TEE_DbgPrintLnf(TATAG "Bad params input");
		return TEE_ERROR_BAD_PARAMETERS;
	}

	// Verify that NWd buffer is fully in NWd, and does not extend too far.
	ret = TEE_CheckMemoryAccessRights(TEE_MEMORY_ACCESS_READ |
									  TEE_MEMORY_ACCESS_ANY_OWNER,
									  pInput, nInputSize);
	if (ret != TEE_SUCCESS) 
		TEE_DbgPrintLnf(TATAG "Wrong input access rights!");
	return ret;
}

TEE_Result CheckOutputParameter(uint8_t* pOutput, uint32_t nOutputSize)
{
	TEE_Result ret = TEE_ERROR_GENERIC;
	if (pOutput == NULL)
	{
		TEE_DbgPrintLnf(TATAG "Bad params output");
		ret = TEE_ERROR_BAD_PARAMETERS;
		return ret;
	}

	ret = TEE_CheckMemoryAccessRights(TEE_MEMORY_ACCESS_WRITE |
									  TEE_MEMORY_ACCESS_ANY_OWNER,
									  pOutput, nOutputSize);
	if (ret != TEE_SUCCESS) 
		TEE_DbgPrintLnf(TATAG "Wrong output access rights!");
	return ret;
}
