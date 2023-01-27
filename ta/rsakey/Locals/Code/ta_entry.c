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
#include "GP_sample_protocol.h"
#include "rsakey_uuid.h"
#include "buildTag.h"

TEE_UUID uuid = rsakey_UUID;
DECLARE_TRUSTED_APPLICATION_MAIN_STACK(16384)

#define EMSG(fmt, args...) TEE_LogPrintf("rsakey "fmt"\n", ##args)

#else

#include <tee_ta_api.h>
#include <trace.h>
#include <tee_api.h>
#include <tee_api_defines.h>
#include <string.h>

#endif

#include "ta_rsakey.h"


struct rsa_2048_priv{
	uint8_t n[256];
	uint8_t e[3];
	uint8_t d[256];
	uint8_t p[128];
	uint8_t q[128];
	uint8_t dp[128];
	uint8_t dq[128];
	uint8_t qp[128];
};

struct rsa_3072_priv{
	uint8_t n[384];
	uint8_t e[3];
	uint8_t d[384];
	uint8_t p[196];
	uint8_t q[196];
	uint8_t dp[196];
	uint8_t dq[196];
	uint8_t qp[196];
};

struct rsa_4096_priv{
	uint8_t n[512];
	uint8_t e[3];
	uint8_t d[512];
	uint8_t p[256];
	uint8_t q[256];
	uint8_t dp[256];
	uint8_t dq[256];
	uint8_t qp[256];
};

struct key_list{
	char name[10];
	size_t size;
	uint32_t attributeID;
	char *addr;
};


struct der_array_2048
{
	char head[4];
	char version[3];

	char n_b[5]; //0x02, 0x82, 0x01, 0x01, 0x00
	char n[256];

	char e[2+3];

	char d_b[4]; //0x02, 0x82, 0x01, 0x00
	char d[256];

	char p_b[4]; //0x02, 0x81, 0x81, 0x00
	char p[128];

	char q_b[4]; //0x02, 0x81, 0x81, 0x00
	char q[128];

	char dp_b[3]; //0x02, 0x81, 0x80
	char dp[128];

	char dq_b[3]; //0x02, 0x81, 0x80
	char dq[128];

	char qp_b[3]; //0x02, 0x81, 0x80
	char qp[128];
};


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

/* TEE_ObjectHandle <---> Attrs <---> DER <---> PEM */


/* Attrs ---> der */
static int package_ders(void *key_array, const uint32_t key_type, uint32_t key_size, void *der, size_t *der_sz)
{
	TEE_Result res = 0;

	EMSG("%s enter", __func__);
	
	switch(key_type)
	{
		case TEE_TYPE_RSA_KEYPAIR:
			EMSG("%s TEE_TYPE_RSA_KEYPAIR--cmd", __func__);
			switch(key_size)
			{
				case 2048:
					EMSG("%s case 2048--cmd", __func__);
					struct rsa_2048_priv *pkey = (struct rsa_2048_priv *)key_array;
					struct der_array_2048 priv = {
					.head = {0x30, 0x82, 0x04, 0xa2},
					.version = {0x02, 0x01, 0x00},
					.n_b = {0x02, 0x82, 0x01, 0x01, 0x00},
					.e = {0x02, 0x03, 0x01, 0x00, 0x01},
					.d_b = {0x02, 0x82, 0x01, 0x00},
					.p_b = {0x02, 0x81, 0x81, 0x00},
					.q_b = {0x02, 0x81, 0x81, 0x00},
					.dp_b = {0x02, 0x81, 0x80},
					.dq_b = {0x02, 0x81, 0x80},
					.qp_b = {0x02, 0x81, 0x80}
					};
					
					memcpy(priv.n, pkey->n,256);
					memcpy(priv.d, pkey->d,256);
					memcpy(priv.p, pkey->p,128);
					memcpy(priv.q, pkey->q,128);
					memcpy(priv.dp,pkey->dp, 128);
					memcpy(priv.dq,pkey->dq, 128);
					memcpy(priv.qp,pkey->qp, 128);
					
					*der_sz = sizeof(struct der_array_2048);
					memcpy(der, &priv , sizeof(struct der_array_2048));

					break;
			}
		case TEE_TYPE_RSA_PUBLIC_KEY:
			break;
		case TEE_TYPE_ECDSA_KEYPAIR:
			break;
		case TEE_TYPE_ECDSA_PUBLIC_KEY:
			break;
		default:
			break;
	}

	return res;
}

/* Attrs array ---> TEE_ObjectHandle*/
static int set_attrs_to_obj_handle(TEE_ObjectHandle key, const uint32_t key_type, uint32_t key_size, void *data)
{
	TEE_Result res;
	int i = 0;

	EMSG("%s enter", __func__);
	
	switch(key_type)
	{
		case TEE_TYPE_AES:
		case TEE_TYPE_DES:
		case TEE_TYPE_DES3:
		//case TEE_TYPE_SM4: //TODO
		case TEE_TYPE_HMAC_MD5:
		case TEE_TYPE_HMAC_SHA1:
		case TEE_TYPE_HMAC_SHA224:
		case TEE_TYPE_HMAC_SHA256:
		case TEE_TYPE_HMAC_SHA384:
		case TEE_TYPE_HMAC_SHA512:
		//case TEE_TYPE_HMAC_SM3: //TODO
		case TEE_TYPE_GENERIC_SECRET:
			break;
		
		case TEE_TYPE_RSA_KEYPAIR:
			EMSG("%s TEE_TYPE_RSA_KEYPAIR", __func__);
			
			TEE_Attribute attrs[8];
			
			struct rsa_2048_priv *priv2048 = (struct rsa_2048_priv *)data;
			struct rsa_3072_priv *priv3072 = (struct rsa_3072_priv *)data;
			struct rsa_4096_priv *priv4096 = (struct rsa_4096_priv *)data;
		
			struct {
				char *n;
				char *e;
				char *d;
				char *p;
				char *q;
				char *dp;
				char *dq;
				char *qp;
			} priv = {0};
			switch(key_size)
			{
				case 2048:
					priv.n = (char *)priv2048->n;
					priv.e = (char *)priv2048->e;
					priv.d = (char *)priv2048->d;
					priv.p = (char *)priv2048->p;
					priv.q = (char *)priv2048->q;
					priv.dp = (char *)priv2048->dp;
					priv.dq = (char *)priv2048->dq;
					priv.qp = (char *)priv2048->qp;
					break;
				case 3072:
					priv.n = (char *)priv3072->n;
					priv.e = (char *)priv3072->e;
					priv.d = (char *)priv3072->d;
					priv.p = (char *)priv3072->p;
					priv.q = (char *)priv3072->q;
					priv.dp =(char *)priv3072->dp;
					priv.dq =(char *)priv3072->dq;
					priv.qp =(char *)priv3072->qp;
					break;
				case 4096:
					priv.n = (char *)priv4096->n;
					priv.e = (char *)priv4096->e;
					priv.d = (char *)priv4096->d;
					priv.p = (char *)priv4096->p;
					priv.q = (char *)priv4096->q;
					priv.dp = (char *)priv4096->dp;
					priv.dq = (char *)priv4096->dq;
					priv.qp = (char *)priv4096->qp;
					break;
			}
	
			struct key_list p_rsa_priv[] = {
				{"n", key_size/8, TEE_ATTR_RSA_MODULUS, priv.n},
				{"e", 3, TEE_ATTR_RSA_PUBLIC_EXPONENT, priv.e},
				{"d", key_size/8, TEE_ATTR_RSA_PRIVATE_EXPONENT, priv.d},
				{"p", key_size/16, TEE_ATTR_RSA_PRIME1, priv.p},
				{"q", key_size/16, TEE_ATTR_RSA_PRIME2, priv.q},
				{"dp", key_size/16, TEE_ATTR_RSA_EXPONENT1, priv.dp},
				{"dq", key_size/16, TEE_ATTR_RSA_EXPONENT2, priv.dq},
				{"qp",key_size/16, TEE_ATTR_RSA_COEFFICIENT, priv.qp},
				{"", 0, 0, 0}		
			};
		
			TEE_MemFill(attrs, 0, sizeof(attrs));
			for (i = 0;; i ++)
			{
				if (p_rsa_priv[i].size == 0)
				{
					break;
				}
				
				TEE_InitRefAttribute(&attrs[i], p_rsa_priv[i].attributeID, p_rsa_priv[i].addr,p_rsa_priv[i].size);
				
			}
			res = TEE_PopulateTransientObject(key, attrs, i);
			break;
		
		case TEE_TYPE_RSA_PUBLIC_KEY:
			break;
		case TEE_TYPE_ECDSA_KEYPAIR:
			break;
		case TEE_TYPE_ECDSA_PUBLIC_KEY:
			break;
		default:
			break;
	}

	return res;
}


/* TEE_ObjectHandle ---> Attrs */
static int get_attrs_from_obj_handle(TEE_ObjectHandle key, const uint32_t key_type, uint32_t key_size, void *data, size_t *data_sz)
{
	TEE_Result res;
	TEE_ObjectInfo object_info;
	size_t read_bytes = 0;
	int i = 0;

	EMSG("%s enter", __func__);
	
	switch(key_type)
	{
		case TEE_TYPE_AES:
		case TEE_TYPE_DES:
		case TEE_TYPE_DES3:
		//case TEE_TYPE_SM4:
		case TEE_TYPE_HMAC_MD5:
		case TEE_TYPE_HMAC_SHA1:
		case TEE_TYPE_HMAC_SHA224:
		case TEE_TYPE_HMAC_SHA256:
		case TEE_TYPE_HMAC_SHA384:
		case TEE_TYPE_HMAC_SHA512:
		//case TEE_TYPE_HMAC_SM3:
		case TEE_TYPE_GENERIC_SECRET:
			break;
		
		case TEE_TYPE_RSA_KEYPAIR:
			EMSG("%s TEE_TYPE_RSA_KEYPAIR--cmd", __func__);

			struct rsa_2048_priv *priv2048 = (struct rsa_2048_priv *)data;
			struct rsa_3072_priv *priv3072 = (struct rsa_3072_priv *)data;
			struct rsa_4096_priv *priv4096 = (struct rsa_4096_priv *)data;
		
			struct {
				char *n;
				char *e;
				char *d;
				char *p;
				char *q;
				char *dp;
				char *dq;
				char *qp;
			} priv = {0};
			switch(key_size)
			{
				case 2048:
					priv.n = (char *)priv2048 ->n;
					priv.e = (char *)priv2048 ->e;
					priv.d = (char *)priv2048 ->d;
					priv.p = (char *)priv2048 ->p;
					priv.q = (char *)priv2048 ->q;
					priv.dp = (char *)priv2048 ->dp;
					priv.dq = (char *)priv2048 ->dq;
					priv.qp = (char *)priv2048 ->qp;
					break;
				case 3072:
					priv.n = (char *)priv3072 ->n;
					priv.e = (char *)priv3072 ->e;
					priv.d = (char *)priv3072 ->d;
					priv.p = (char *)priv3072 ->p;
					priv.q = (char *)priv3072 ->q;
					priv.dp = (char *)priv3072 ->dp;
					priv.dq = (char *)priv3072 ->dq;
					priv.qp = (char *)priv3072 ->qp;
					break;
				case 4096:
					priv.n = (char *)priv4096 ->n;
					priv.e = (char *)priv4096 ->e;
					priv.d = (char *)priv4096 ->d;
					priv.p = (char *)priv4096 ->p;
					priv.q = (char *)priv4096 ->q;
					priv.dp = (char *)priv4096 ->dp;
					priv.dq = (char *)priv4096 ->dq;
					priv.qp = (char *)priv4096 ->qp;
					break;
			}
			
			struct key_list p_rsa_priv[] = {
				{"n", key_size/8, TEE_ATTR_RSA_MODULUS, priv.n},
				{"e", 3, TEE_ATTR_RSA_PUBLIC_EXPONENT, priv.e},
				{"d", key_size/8, TEE_ATTR_RSA_PRIVATE_EXPONENT, priv.d},
				{"p", key_size/16, TEE_ATTR_RSA_PRIME1, priv.p},
				{"q", key_size/16, TEE_ATTR_RSA_PRIME2, priv.q},
				{"dp", key_size/16, TEE_ATTR_RSA_EXPONENT1, priv.dp},
				{"dq", key_size/16, TEE_ATTR_RSA_EXPONENT2, priv.dq},
				{"qp",key_size/16, TEE_ATTR_RSA_COEFFICIENT, priv.qp},
				{"", 0, 0, 0}		
			};
			
			res = TEE_GetObjectInfo1(key, &object_info);
			if (res != TEE_SUCCESS) {
					EMSG("Failed to create persistent object, res=0x%08x", res);
					return -1;
			}
		
			if (object_info.dataSize > key_size) {
					res = TEE_ERROR_SHORT_BUFFER;
					return -1;
			}
			*data_sz = object_info.dataSize / 8;
		
			for (i = 0;; i ++)
			{
				if (p_rsa_priv[i].size == 0)
				{
					break;
				}
				
				read_bytes = p_rsa_priv[i].size;
				res = TEE_GetObjectBufferAttribute(key, p_rsa_priv[i].attributeID, p_rsa_priv[i].addr, &read_bytes);
				//EMSG("-------------------------%s size= [%d]", p_rsa_priv[i].name ,(unsigned int)read_bytes);
				//print_hex_data("-", (char *)p_rsa_priv[i].addr, read_bytes);
				
			}
			break;
		
		case TEE_TYPE_RSA_PUBLIC_KEY:
			break;
		case TEE_TYPE_ECDSA_KEYPAIR:
			break;
		case TEE_TYPE_ECDSA_PUBLIC_KEY:
			break;
		default:
			break;
	}

	return 0;
}

const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char * base64_encode( char * bindata, char * base64, size_t binlength )
{
    size_t i, j;
    unsigned char current;

    for ( i = 0, j = 0 ; i < binlength ; i += 3 )
    {
        current = (bindata[i] >> 2) ;
        current &= (unsigned char)0x3F;
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
        if ( i + 1 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+1] >> 4) ) & ( (unsigned char) 0x0F );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
        if ( i + 2 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+2] >> 6) ) & ( (unsigned char) 0x03 );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)bindata[i+2] ) & ( (unsigned char)0x3F ) ;
        base64[j++] = base64char[(int)current];
    }
    base64[j] = '\0';
    return base64;
}

int base64_decode( char * base64, char * bindata )
{
    size_t i, j;
    unsigned char k;
    unsigned char temp[4];
    for ( i = 0, j = 0; base64[i] != '\0' ; i += 4 )
    {
        memset( temp, 0xFF, sizeof(temp) );
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i] )
                temp[0]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+1] )
                temp[1]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+2] )
                temp[2]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+3] )
                temp[3]= k;
        }

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2))&0xFC)) |
                ((unsigned char)((unsigned char)(temp[1]>>4)&0x03));
        if ( base64[i+2] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4))&0xF0)) |
                ((unsigned char)((unsigned char)(temp[2]>>2)&0x0F));
        if ( base64[i+3] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6))&0xF0)) |
                ((unsigned char)(temp[3]&0x3F));
    }
    return j;
}


static TEE_ObjectHandle gen_key_rsa(const uint32_t key_type, uint32_t key_size)
{
	TEE_Result res;
	TEE_ObjectHandle key;

	EMSG("%s enter", __func__);

	res = TEE_AllocateTransientObject(key_type, key_size, &key);
	if (res) {
		EMSG("%s : key_size=%d, res=0x%08x", __func__, key_size, res);
		return NULL;
	}

	res = TEE_GenerateKey(key, key_size, NULL, 0);
	if (res) {
		EMSG("%s : key_size=%d, res=0x%08x", __func__, key_size, res);
		TEE_FreeTransientObject(key);
		return NULL;
	}

	return key;
}

#define GEN_RSAKEY(keysize) \
TEE_ObjectHandle gen_key_rsa_##keysize(void) \
{ \
	return gen_key_rsa(TEE_TYPE_RSA_KEYPAIR, keysize); \
}

GEN_RSAKEY(2048)
GEN_RSAKEY(3072)
GEN_RSAKEY(4096)

TEE_ObjectHandle gen_key_rsa_2048(void);
TEE_ObjectHandle gen_key_rsa_3072(void);
TEE_ObjectHandle gen_key_rsa_4096(void);


/*
example:
02 82 01 01 00 a5  --n
02 03 01 00 01  --e
02 82 01 00 50  --d
02 81 81 00 db  --q
02 81 81 00 c1  --p
02 81 81 00 a0  --dp
02 81 80 75  --dq
02 81 80 4c  --qp
*/
void *find_x_offset(void* keyparis, uint32_t keyparis_len)
{
	uint8_t *p;

	p = keyparis;
	while (true)
	{
		if((uint32_t)((unsigned long)p - (unsigned long)keyparis) > keyparis_len)
			break;
 		if (*(uint16_t *)p == 0x8202) {  /* 02 82 for N D */
			if (*(uint16_t *)(p+2) == 0x101) {
				return (p+5);
			} else if (*(uint16_t *)(p+2) == 0x1) { //0x100 : bit-little conversion
				return (p+4);
			}

		} else if (*(uint16_t *)p == 0x8102) { /* 02 81 for D P Q DP DQ QP */
			if (*(uint8_t *)(p+2) == 0x81) {
				return (p+4);
			} else if (*(uint8_t *)(p+2) == 0x80) {
				return (p+3);
			}

		} else if (*(uint16_t *)p == 0x302) { /* 02 03 for E */
			return (p+2);
		}
		p++;
	}
	EMSG("find_x_offset : There has been a serious mistake!!!!!!!");
	return NULL;
}
#define RSA_KEY_PQ_LENGTH 128
#define RSA_KEY_BYTE_LENGTH 256

/* der file ---> Attrs */
static int get_attrs_from_der_file(void *keyparis, size_t keyparis_len, void *attr_array)
{
		char *p_offset;
		size_t p_len = keyparis_len;
		struct rsa_2048_priv *attr = attr_array;
		
		p_offset = keyparis;
		p_len = keyparis_len;
		p_offset = find_x_offset(p_offset,p_len);
		if(p_offset) {
			memcpy(attr->n, p_offset, RSA_KEY_BYTE_LENGTH);
			p_offset += RSA_KEY_BYTE_LENGTH;
		} else {
			return  -1;
		}

		p_offset = find_x_offset(p_offset,(uint32_t)(p_len-((unsigned long)p_offset-(unsigned long)keyparis)));
		if(p_offset) {
			memcpy(attr->e, p_offset, 3);
			p_offset += 3;
		} else {
			return  -1;
		}

		p_offset = find_x_offset(p_offset,(uint32_t)(p_len-((unsigned long)p_offset-(unsigned long)keyparis)));
		if(p_offset) {
			memcpy(attr->d, p_offset, RSA_KEY_BYTE_LENGTH);
			p_offset += RSA_KEY_BYTE_LENGTH;
		} else {
			return  -1;
		}

		p_offset = find_x_offset(p_offset,(uint32_t)(p_len-((unsigned long)p_offset-(unsigned long)keyparis)));
		if(p_offset) {
			memcpy(attr->p, p_offset, RSA_KEY_PQ_LENGTH);
			p_offset += RSA_KEY_PQ_LENGTH;
		} else {
			return  -1;
		}

		p_offset = find_x_offset(p_offset,(uint32_t)(p_len-((unsigned long)p_offset-(unsigned long)keyparis)));
		if(p_offset) {
			memcpy(attr->q, p_offset, RSA_KEY_PQ_LENGTH);
			p_offset += RSA_KEY_PQ_LENGTH;
		} else {
			return -1;
		}

		p_offset = find_x_offset(p_offset,(uint32_t)(p_len-((unsigned long)p_offset-(unsigned long)keyparis)));
		if(p_offset) {
			memcpy(attr->dp, p_offset, RSA_KEY_PQ_LENGTH);
			p_offset += RSA_KEY_PQ_LENGTH;
		} else {
			return  -1;
		}

		p_offset = find_x_offset(p_offset,(uint32_t)(p_len-((unsigned long)p_offset-(unsigned long)keyparis)));
		if(p_offset) {
			memcpy(attr->dq, p_offset, RSA_KEY_PQ_LENGTH);
			p_offset += RSA_KEY_PQ_LENGTH;
		} else {
			return  -1;
		}

		p_offset = find_x_offset(p_offset,(uint32_t)(p_len-((unsigned long)p_offset-(unsigned long)keyparis)));
		if(p_offset) {
			memcpy(attr->qp, p_offset, RSA_KEY_PQ_LENGTH);
			p_offset += RSA_KEY_PQ_LENGTH;
		} else {
			return  -1;
		}
		
		return 0;

}


static TEE_Result rsa_arithmetic(TEE_ObjectHandle key, uint32_t mode, const void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len)
{
	TEE_Result res;

	TEE_OperationHandle op;
	TEE_ObjectInfo key_info;
	uint32_t alg = TEE_ALG_RSAES_PKCS1_V1_5;
	
	EMSG("%s enter", __func__);

	if (!key)
		return TEE_ERROR_BAD_STATE;

	res = TEE_GetObjectInfo1(key, &key_info);
	if (res) {
		EMSG("TEE_GetObjectInfo1 error! res=0x%x", res);
		return res;
	}
	
	switch (mode)
	{
		case TEE_MODE_ENCRYPT:
		case TEE_MODE_DECRYPT:
			alg = TEE_ALG_RSAES_PKCS1_V1_5;
			break;
		case TEE_MODE_SIGN:
		case TEE_MODE_VERIFY:
			alg = TEE_ALG_RSASSA_PKCS1_V1_5_SHA256;
			break;
	}

	res = TEE_AllocateOperation(&op, alg, mode, key_info.objectSize);
	if (res) {
		EMSG("TEE_AllocateTransientObject error! res=0x%x", res);
		return res;
	}

	res = TEE_SetOperationKey(op, key);
	if (res) {
		EMSG("TEE_SetOperationKey error! res=0x%x", res);
		TEE_FreeOperation(op);
		return TEE_ERROR_BAD_STATE;
	}

	switch (mode)
	{
		case TEE_MODE_ENCRYPT:
			res = TEE_AsymmetricEncrypt(op, NULL, 0, inbuf, inbuf_len, outbuf, outbuf_len);
			if (res) {
				EMSG("TEE_AsymmetricEncrypt error! res=0x%x", res);
			}
			break;
		case TEE_MODE_DECRYPT:
			res = TEE_AsymmetricDecrypt(op, NULL, 0, inbuf, inbuf_len, outbuf, outbuf_len);
			if (res) {
				EMSG("TEE_AsymmetricDecrypt error! res=0x%x", res);
			}
			break;
		case TEE_MODE_SIGN:
			res = TEE_AsymmetricSignDigest(op, NULL, 0, inbuf, inbuf_len, outbuf, outbuf_len);
			if (res) {
				EMSG("TEE_AllocateTransientObject error! res=0x%x", res);
			}
			break;
		case TEE_MODE_VERIFY:
			res = TEE_AsymmetricVerifyDigest(op, NULL, 0, inbuf, inbuf_len, outbuf, *outbuf_len);
			if (res) {
				EMSG("TEE_AsymmetricVerifyDigest error! res=0x%x", res);
			}
			break;
	}

	TEE_FreeOperation(op);
	
	EMSG("%s exit", __func__);
	return res;

}

#define RSA(name1,name2) \
TEE_Result rsa_##name1(TEE_ObjectHandle key, const void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len) \
{ \
	return rsa_arithmetic(key, TEE_MODE_##name2, inbuf, inbuf_len, outbuf, outbuf_len); \
}

RSA(enc, ENCRYPT)
RSA(dec, DECRYPT)
RSA(sign, SIGN)
RSA(verify, VERIFY)

TEE_Result rsa_enc(TEE_ObjectHandle key, const void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len);
TEE_Result rsa_dec(TEE_ObjectHandle key, const void *inbuf, uint32_t inbuf_len, void *outbuf, size_t *outbuf_len);
TEE_Result rsa_sign(TEE_ObjectHandle key, const void *inbuf, uint32_t inbuf_len, void *signature, size_t *signatureLen);
TEE_Result rsa_verify(TEE_ObjectHandle key, const void *digest, uint32_t digestLen, void *signature, size_t *signatureLen);

static void test_rsakey(void)
{
	
	struct rsa_2048_priv attr_priv2048; // attr array
	struct der_array_2048 priv2048_der; // der file
	size_t der_len;
	char pem_buf[2000] = {0}; // pem file
	
	size_t temp_len;
	char der_buf[2000] = {0}; // temp der buf
	struct rsa_2048_priv temp_attr_priv2048; // temp attr array
	TEE_ObjectHandle rsakey  = TEE_HANDLE_NULL;
	
	EMSG("%s ===================== 1. get rsakey (TEE_ObjectHandle",__func__);
	
	/* 1. get rsakey (TEE_ObjectHandle)*/
	TEE_ObjectHandle rsakey2048 = gen_key_rsa_2048();
	
	EMSG("%s ===================== 2. TEE_ObjectHandle ---> Attrs ",__func__);
	
	/* 2. TEE_ObjectHandle ---> Attrs */
	get_attrs_from_obj_handle(rsakey2048, TEE_TYPE_RSA_KEYPAIR, 2048, &attr_priv2048, &temp_len);
	
	EMSG("%s ===================== 3. Attrs array ---> der file ",__func__);
	
	/* 3. Attrs array ---> der file */
	package_ders(&attr_priv2048, TEE_TYPE_RSA_KEYPAIR, 2048, &priv2048_der, &der_len);
	
	EMSG("%s ===================== 4. der file ---> pem file ",__func__);
	
	/* 4. der file ---> pem file */
	base64_encode( (char *)&priv2048_der, (char *)pem_buf, der_len );
	
	/* ------------------------------------------------------------ */
	
	EMSG("%s ===================== 5. pem file ---> der file ",__func__);
	
	/* 5. pem file ---> der file */
	der_len = base64_decode(pem_buf, der_buf);
	
	EMSG("%s ===================== 6. der file ---> Attrs array ",__func__);
	
	/* 6. der file ---> Attrs array*/
	get_attrs_from_der_file(der_buf, der_len, &temp_attr_priv2048);
	
	EMSG("%s ===================== 7. Attrs array ---> TEE_ObjectHandle ",__func__);
	
	/* 7. Attrs array ---> TEE_ObjectHandle*/
	TEE_AllocateTransientObject(TEE_TYPE_RSA_KEYPAIR, 2048, &rsakey);
	set_attrs_to_obj_handle(rsakey, TEE_TYPE_RSA_KEYPAIR, 2048, &temp_attr_priv2048);
	
	EMSG("%s ===================== 8. rsa_enc/rsa_dec &&  rsa_sign/rsa_verify TEST ",__func__);
	
	{
		char in_buf[32+1] = "0123456789abcdef0123456789abcdef";
		uint32_t in_sz = 32;
		char tmp_buf[512] = {0};
		size_t tmp_sz = 512;
		char out_buf[48] = {0};
		size_t out_sz = 48;
		TEE_Result res;
		
		res = rsa_enc(rsakey, in_buf, in_sz, tmp_buf, &tmp_sz);
		EMSG("%s : res = %d, tmp_sz=%d", __func__, res, (int)tmp_sz);
		res = rsa_dec(rsakey, tmp_buf, tmp_sz, out_buf, &out_sz);
		EMSG("%s : res = %d, out_sz=%d", __func__, res, (int)out_sz);
		
		if(in_sz == out_sz &&  TEE_MemCompare(in_buf, out_buf, out_sz) == 0)
		{
			EMSG("Test %s-rsa_enc/rsa_dec pass", __func__);
		} else {
			EMSG("Test %s-rsa_enc/rsa_dec failed", __func__);
		}
		
		tmp_sz = 256;
		res = rsa_sign(rsakey, in_buf, in_sz, tmp_buf, &tmp_sz);
		EMSG("%s : res = %d, tmp_sz=%d", __func__, res, (int)tmp_sz);
		res = rsa_verify(rsakey, in_buf, in_sz, tmp_buf, &tmp_sz);
		EMSG("%s : res = %d", __func__, res);
		if(res == 0)
		{
			EMSG("Test %s-rsa_sign/rsa_verify pass", __func__);
		} else {
			EMSG("Test %s-rsa_sign/rsa_verify failed", __func__);
		}
		
	}
	
}



TEE_Result TA_InvokeCommandEntryPoint(void *session_id,
                                      uint32_t command_id,
                                      uint32_t param_types,
                                      TEE_Param parameters[4])
{
	(void)(session_id);
	(void)(parameters);

	EMSG("%s enter", __func__);

	if ((TEE_PARAM_TYPE_GET(param_types, 0) != TEE_PARAM_TYPE_MEMREF_INPUT) ||
		(TEE_PARAM_TYPE_GET(param_types, 1) != TEE_PARAM_TYPE_MEMREF_OUTPUT) ||
		(TEE_PARAM_TYPE_GET(param_types, 2) != TEE_PARAM_TYPE_NONE) ||
		(TEE_PARAM_TYPE_GET(param_types, 3) != TEE_PARAM_TYPE_NONE)) {
		EMSG("The param_types is error!");
		return TEE_ERROR_BAD_PARAMETERS;
	}

	switch (command_id) {
		case TA_RSAKEY_CMD_1:
			EMSG("--TA_RSAKEY_CMD_1!");
			test_rsakey();
			;
		case TA_RSAKEY_CMD_2:
			;
		case TA_RSAKEY_CMD_3:
			;
		default:
			;
	}

	EMSG("%s exit", __func__);

	return TEE_SUCCESS;
}
