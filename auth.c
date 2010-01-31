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
	char* tmp = calloc(16,1);
	MD5((unsigned char*)data,(unsigned long)l,(unsigned char*)tmp);
	return hex(copy(tmp,16));
}

str
hex(str data)
{
        int i;
        str retval = NULL;
        for (i = 0; i < data->length; ++i) {
		retval = append(retval,Str("%h%h", (0x0f0 & data->data[i]) >> 4, 0x0f & data->data[i]));
        }
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
	for (t = s; t; t = t->next) BIO_write(dst,t->data,t->length);
	BIO_flush(dst);
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
	char* secret_data = dump(secret);
	char* data_data = dump(data);
	str retval = blank(4000);
	HMAC_CTX ctx;
	HMAC_CTX_init(&ctx);
	HMAC(EVP_sha1(),secret_data,sl,(unsigned char*)data_data,dl,(unsigned char*)retval->data,(unsigned int*)&retval->length);
	HMAC_CTX_cleanup(&ctx);
	free(secret_data);
	free(data_data);
	return retval;
}
