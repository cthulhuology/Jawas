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
#include "auth.h"

str
md5sum(char* data, int len)
{
	str retval = char_str(NULL,16);
	MD5((unsigned char*)data,(unsigned long)len,(unsigned char*)retval->data);
	return retval;
}

str
md5hex(char* data, int len)
{
        int i;
        str tmp = md5sum(data,len);
        str retval = NULL;
        for (i = 0; i < 16; ++i) {
                int a = (tmp->data[i] & 0xff) / 16;
                int b = (tmp->data[i] & 0xff) % 16;
                retval =  retval ? Str("%s%h%h",retval,a,b) : Str("%h%h",a,b);
        }
        return retval;
}

str
base64(str s)
{
	str retval;
	BUF_MEM* ptr;
	BIO* src;
	BIO* dst;
	dst = BIO_new(BIO_f_base64());
	src = BIO_new(BIO_s_mem());
	dst = BIO_push(dst,src);
	BIO_write(dst,s->data,s->len);
	BIO_flush(dst);
	BIO_get_mem_ptr(dst,&ptr);
	retval = char_str(ptr->data,ptr->length-1);
	BIO_free_all(dst);
	return retval;
}	

str
hmac1(str secret, str data)
{
	str retval = char_str(NULL,20);
	HMAC_CTX ctx;
	HMAC_CTX_init(&ctx);
	HMAC(EVP_sha1(),secret->data,secret->len,(unsigned char*)data->data,data->len,(unsigned char*)retval->data,(unsigned int*)&retval->len);
	HMAC_CTX_cleanup(&ctx);	
	return retval;
}

