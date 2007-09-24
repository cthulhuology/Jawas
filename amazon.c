// amazon.c
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
#include "alloc.h"
#include "str.h"
#include "files.h"
#include "dates.h"
// #include "amazon.h"

str
md5sum(unsigned char* data, unsigned long len)
{
	str retval = char_str(NULL,16);
	MD5(data,len,(unsigned char*)retval->data);
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

str 
auth_string(str key, str secret, str verb, str md5, str mime, str date, str bucket, str filename)
{
	str sts = Str("%s\n%s\n\%s\n%s\n/%s/%s\n",verb,md5,mime,date,bucket,filename);
	return Str("AWS %s:%s",key,base64(hmac1(secret,sts)));
}

str
put_object(str key, str secret, str host, str bucket, File fc, str mime)
{
	str date = Date();
	str md5 = md5sum((unsigned char*)fc->data,(unsigned long)fc->st.st_size);
	str auth = auth_string(key,secret,Str("PUT"),md5,mime,date,bucket,Str("%c",fc->name));
	str cmd = Str("PUT /%s HTTP/1.1\r\nContent-Length: %i\r\nHost: %s\r\nDate: %s\r\nAuthorization: %s\r\nContent-Type: %s\r\nContent-MD5: %s\r\n\r\n%s", fc->name, fc->st.st_size,host,date,auth,mime,md5);
	return cmd;
}
