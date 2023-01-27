/*
 * Copyright (c) 2018 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef TAFACERECOGNITION_H_
#define TAFACERECOGNITION_H_

#include <tee_internal_api.h>
#include "feature_type.h"
/**
 * parameters
 */

#define TATAG 						"[TA FR] "
#define CMD_FEATURE_BEGIN       	0x10000
#define CMD_FEATURE_WRITE    		0x10001
#define CMD_FEATURE_READ           	0x10002
#define CMD_FEATURE_COMPARE        	0x10003
#define CMD_FEATURE_ADD        		0x10004
#define CMD_FEATURE_UPDATE     		0x10005
#define CMD_FEATURE_REMOVE     		0x10006
#define CMD_FEATURE_COUNT     		0x10007
#define CMD_FEATURE_VALID     		0x10008
#define CMD_FEATURE_END       		0x1000F

//#define IS_HALF_FLAG_ENABLE			1

#define CMD_FACE_RECOGNITION_PTYPES 	TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT, TEE_PARAM_TYPE_MEMREF_OUTPUT, TEE_PARAM_TYPE_MEMREF_INPUT, TEE_PARAM_TYPE_NONE)

MGULKStatus FaceCompare(     const MGULKSavedFeature* feature,
                             const MGULKSavedFeature* target_feature,
                             MGULKTeeFaceCompareScore* score);
#endif // TAFACERECOGNITION_H_
