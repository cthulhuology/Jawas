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
#include "log.h"
#include "files.h"
#include "dates.h"
#include "sockets.h"
#include "server.h"
#include "requests.h"
#include "events.h"
#include "wand.h"
#include "amazon.h"

str
md5sum(char* data, int len)
{
	str retval = char_str(NULL,16);
	MD5((unsigned char*)data,(unsigned long)len,(unsigned char*)retval->data);
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
s3_put_auth_string(str verb, str mime, str date, str bucket, str filename)
{
	str sts = Str("%s\n\n\%s\n%s\nx-amz-acl:public-read\n/%s%s",verb,mime,date,bucket,filename);
	debug("STS is %s",sts);
	return Str("AWS %s:%s",srv->s3key,base64(hmac1(srv->s3secret,sts)));
}

str
s3_put_jpeg(str bucket, str filename)
{
	int off;
	File fc = load(filename);
	str date = Date();
	debug("Date is %s",date);
	str md5 = base64(md5sum(fc->data,fc->st.st_size));
	debug("MD5 is %s",md5);
	str file = Str("%c.jpg",filename->data+4);
	debug("File is %s",file);
	str mime = Str("image/jpeg");
	debug("MIME is %s",mime);
	str auth = s3_put_auth_string(Str("PUT"),mime,date,bucket,file);
	debug("AUTH is %s", auth);
	str size = Str("%i",fc->st.st_size);
	debug("Size is %s",size);
	str cmd = Str("PUT %s HTTP/1.1\r\nx-amz-acl: public-read\r\nContent-Type: %s\r\nContent-Length: %s\r\nHost: %s.s3.amazonaws.com\r\nDate: %s\r\nAuthorization: %s\r\n\r\n", file, mime,size,bucket,date,auth);
	debug("CMD is %s",cmd);
	if(!fork()) {
		Socket sc = connect_socket("s3.amazonaws.com",80);
		write_socket(sc,cmd->data,cmd->len);
		for (off = 0; off < fc->st.st_size; off += write_socket(sc,fc->data+off,min(fc->st.st_size-off,MAX_WRITE_SIZE))) {
			debug("Writing %i of %i",off, fc->st.st_size);
		}
		debug("Wrote %i of %i",off,fc->st.st_size);
		str output = char_str(NULL,4000);
		output->len = recv(sc->fd,output->data,output->len,0);
		debug("[AMAZON] %s",output);
		close_socket(sc);
		exit(0);
	}
	return cmd;
}

str
s3_put_thumb(str bucket, str filename)
{
	str thumb = Str("%s-thumb",filename);
	if (create_thumb(filename->data,thumb->data)) {
		error("Failed to create thumbnail %s",thumb);
		return NULL;
	}
	return s3_put_jpeg(bucket,thumb);
}



