/*
 * Copyright (c) 2015 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef UTIL_H
#define UTIL_H

#include "taStd.h"
#include "tee_internal_api.h"

// Print the result message without leaving the current function
#define PRINTERROR_MSG(nResult,message) do {                        \
        TEE_DbgPrintLnf("\n############### %s ##############",TATAG); \
        TEE_DbgPrintLnf("\n Error In TA Function ");                  \
        TEE_DbgPrintLnf("\n %s ",message);                            \
        TEE_DbgPrintLnf("\n ERROR = %u  ",nResult);                   \
        TEE_DbgPrintLnf("\n in FILE = %s  ",__FILE__);                \
        TEE_DbgPrintLnf("\n FUNCTION = %s  ",__FUNCTION__);           \
        TEE_DbgPrintLnf("\n LINE = %u  ",__LINE__);                   \
        TEE_DbgPrintLnf("\n###############################################"); \
} while (FALSE)

// print and return an error if nresult != TEE_SUCCESS
// leave the current function
#define CHK_TEE_ERROR(nResult) do { \
    if (nResult != TEE_SUCCESS)     \
    {                               \
        TEE_DbgPrintLnf("\n############### %s ##############",TATAG);         \
        TEE_DbgPrintLnf("\n Error In TA Function ");                          \
        TEE_DbgPrintLnf("\n ERROR = 0x%x\r  ",nResult);                           \
        TEE_DbgPrintLnf("\n in FILE = %s  ",__FILE__);                        \
        TEE_DbgPrintLnf("\n FUNCTION = %s  ",__FUNCTION__);                   \
        TEE_DbgPrintLnf("\n LINE = %u  ",__LINE__);                           \
        TEE_DbgPrintLnf("\n###############################################"); \
        goto end;                                                     \
    }                                                                       \
    else if (nResult > TEE_SUCCESS)                                         \
    {                                                                       \
        TEE_DbgPrintLnf("\n############### %s ##############",TATAG);         \
        TEE_DbgPrintLnf("\n nResult Return In TA Function ");                 \
        TEE_DbgPrintLnf("\n nResult = 0x%x\r  ",nResult);                         \
        TEE_DbgPrintLnf("\n in FILE = %s  ",__FILE__);                        \
        TEE_DbgPrintLnf("\n FUNCTION = %s  ",__FUNCTION__);                   \
        TEE_DbgPrintLnf("\n LINE = %u  ",__LINE__);                           \
        TEE_DbgPrintLnf("\n###############################################"); \
    }                                                                       \
} while (FALSE)

// print and return an error if nresult != Wanted (expected) result
// print a message , generaly this message is the name of function that returns nResult
// leave the current function
#define CHK_TEE_RESULT_MSG(nResult,message) do {                    \
    if (nResult != TEE_SUCCESS)                                      \
    {                                                               \
        TEE_DbgPrintLnf("\n############### %s ##############",TATAG); \
        TEE_DbgPrintLnf("\n Error In TA Function ");                  \
        TEE_DbgPrintLnf("\n %s ",message);                            \
        TEE_DbgPrintLnf("\n ERROR = 0x%x\r  ",nResult);                   \
        TEE_DbgPrintLnf("\n in FILE = %s  ",__FILE__);                \
        TEE_DbgPrintLnf("\n FUNCTION = %s  ",__FUNCTION__);           \
        TEE_DbgPrintLnf("\n LINE = %u  ",__LINE__);                   \
        TEE_DbgPrintLnf("\n###############################################"); \
        goto end;                                                           \
    }                                                                       \
} while (FALSE)

#endif /* UTIL_H */
