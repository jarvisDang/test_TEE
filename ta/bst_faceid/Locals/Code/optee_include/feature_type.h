/*
 * Copyright (c) 2018 Face++ LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * Face++ LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with Face++ LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#define MAX_FEATURE_COUNT 1
#define FEATURE_WRAPPER_SIZE 10000

typedef enum MGULKStatus {
    MG_UNLOCK_OK = 0x0000,
    MG_UNLOCK_INVALID_ARGUMENT,
    MG_UNLOCK_INVALID_HANDLE,
    MG_UNLOCK_FAILURE,

    // return value for compare and save_feature
    MG_UNLOCK_FACE_BAD_QUALITY = 0x1000,
    MG_UNLOCK_FACE_NOT_FOUND,
    MG_UNLOCK_FACE_SCALE_TOO_SMALL,
    MG_UNLOCK_FACE_SCALE_TOO_LARGE,
    MG_UNLOCK_FACE_OFFSET_LEFT,
    MG_UNLOCK_FACE_OFFSET_TOP,
    MG_UNLOCK_FACE_OFFSET_RIGHT,
    MG_UNLOCK_FACE_OFFSET_BOTTOM,
    MG_UNLOCK_ATTR_BLUR,
    MG_UNLOCK_ATTR_EYE_CLOSE,
    MG_UNLOCK_FACE_NOT_COMPLETE,
    MG_UNLOCK_LIVENESS_FAILURE,
    MG_UNLOCK_KEEP,
    MG_UNLOCK_DARKLIGHT,

    // return value for save_feature
    MG_UNLOCK_ATTR_EYE_OCCLUSION = 0x2000,
    MG_UNLOCK_ATTR_MOUTH_OCCLUSION,
    MG_UNLOCK_FACE_ROTATED_LEFT,
    MG_UNLOCK_FACE_RISE,
    MG_UNLOCK_FACE_ROTATED_RIGHT,
    MG_UNLOCK_FACE_DOWN,
    MG_UNLOCK_FACE_MULTI,
    MG_UNLOCK_FACE_BLUR,
    MG_UNLOCK_HIGHLIGHT,
    MG_UNLOCK_HALF_SHADOW,

    // return value for compare
    MG_UNLOCK_COMPARE_FAILURE = 0x3000,
    MG_UNLOCK_NEED_RESTORE_FEATURE,

    // return value for old version API
    MG_ATTR_BLUR = 0xa000,
    MG_ATTR_EYE_OCCLUSION,
    MG_ATTR_EYE_CLOSE,
    MG_ATTR_MOUTH_OCCLUSION,
    MG_UNLOCK_FEATURE_MISS,
    MG_UNLOCK_FEATURE_VERSION_ERROR,
    MG_UNLOCK_BAD_LIGHT,
    MG_UNLOCK_FAILED,
    MG_UNLOCK_FACE_QUALITY,

}MGULKStatus;

typedef struct MGULKPdArgs {
    float v[6];
}MGULKPdArgs;

typedef struct MGULKSavedFeature {
    uint32_t version;
    uint32_t feature_size;
    uint8_t feature[8192];
    MGULKPdArgs pd;
    uint32_t has_half;
    uint8_t padding[1772];
}MGULKSavedFeature;

typedef struct MGFeature {
    //uint8_t hash[MAX_FEATURE_COUNT];
    uint8_t ids[MAX_FEATURE_COUNT];
    MGULKSavedFeature features[MAX_FEATURE_COUNT];
}MGFeature;

typedef struct MGULKTeeFaceCompareScore {
    float l2_full;
    float l2_half;
    float pd_score;
    uint32_t face_id;
}MGULKTeeFaceCompareScore;

typedef struct MGULKCompareResult {
    MGULKTeeFaceCompareScore score;
    MGULKStatus compare_ret;
}MGULKCompareResult;