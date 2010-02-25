// auth.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include "include.h"
#include "defines.h"
#include "memory.h"
#include "str.h"
#include "log.h"
#include "auth.h"

str
md5sum(char* data, int l)
{
	str retval = blank(16);
	MD5((unsigned char*)data,(unsigned long)l,(unsigned char*)retval->data);
	return retval;
}

str
md5hex(char* data, int l)
{
	str retval = blank(16);
	MD5((unsigned char*)data,(unsigned long)l,(unsigned char*)retval->data);
	return hex(retval);
}

str
hex(str data)
{
        str retval = NULL;
        for (int i = 0; i < data->length; ++i) 
		retval = retval ? _("%s%h%h",retval,(0x0f0 & data->data[i]) >> 4, 0x0f & data->data[i]):
			_("%h%h",(0x0f0 & data->data[i]) >> 4, 0x0f & data->data[i]);
        return retval;
}

str
base64(str s)
{
	str retval;
	str t;
	BUF_MEM* ptr;
	BIO* src;
	BIO* dst;
	dst = BIO_new(BIO_f_base64());
	src = BIO_new(BIO_s_mem());
	dst = BIO_push(dst,src);
	BIO_write(dst,t->data,t->length);
	if (1>BIO_flush(dst)) return NULL;
	BIO_get_mem_ptr(dst,&ptr);
	retval = copy(ptr->data,ptr->length-1);
	BIO_free_all(dst);
	return retval;
}

str
hmac1(str secret, str data)
{
	int sl = len(secret);
	int dl = len(data);
	str retval = blank(256);
	HMAC_CTX ctx;
	HMAC_CTX_init(&ctx);
	HMAC(EVP_sha1(),secret->data,sl,(unsigned char*)data->data,dl,(unsigned char*)retval->data,(unsigned int*)&retval->length);
	HMAC_CTX_cleanup(&ctx);
	return retval;
}
