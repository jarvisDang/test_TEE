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

/*
 * caFaceRecognition.h
 *
 */

#ifndef CAFACERECOGNITION_H_
#define CAFACERECOGNITION_H_

#include <tee_client_api.h>
#include "feature_type.h"

//#define LOG_TAG						"[CA FR] "
#define CMD_FEATURE_BEGIN       	0x10000
#define CMD_FEATURE_WRITE    		0x10001
#define CMD_FEATURE_READ           	0x10002
#define CMD_FEATURE_COMPARE        	0x10003
#define CMD_FEATURE_ADD        		0x10004
#define CMD_FEATURE_UPDATE     		0x10005
#define CMD_FEATURE_REMOVE     		0x10006
#define CMD_FEATURE_COUNT     		0x10007
#define CMD_FEATURE_VALID     		0x10008
#define CMD_GET_CHALLENGE       	0X10009
#define CMD_FEATURE_END       		0x1000F


//#define IS_HALF_FLAG_ENABLE			1

uint32_t  TEEAddFeature(MGULKSavedFeature *feature_data, int *id);
uint32_t  TEEUpdateFeature(MGULKSavedFeature *feature_data, int id);
uint32_t  TEERemoveFeature(int id);
uint32_t  TEEGetFeatureCount(int *count);

uint32_t  TEEWriteFeatures(MGFeature *feature_data);
uint32_t  TEEReadFeatures(MGFeature *feature_data);
#ifdef	IS_HALF_FLAG_ENABLE
uint32_t  TEECompareFeatures(MGULKSavedFeature *feature_data, int is_half, MGULKCompareResult *compare_result);
#else
uint32_t  TEECompareFeatures(MGULKSavedFeature *feature_data, MGULKCompareResult *compare_result);
#endif
uint32_t  TEEFeatureIsValid(int id, int *valid);
#endif