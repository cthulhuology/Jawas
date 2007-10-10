// amazon.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

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
#include "auth.h"
#include "amazon.h"

str s3_key;
str s3_secret;

void
s3_auth(str key, str secret)
{
	s3_key = key;
	s3_secret = secret;
}

str 
s3_put_auth_string(str verb, str mime, str date, str bucket, str filename)
{
	str sts = Str("%s\n\n\%s\n%s\nx-amz-acl:public-read\n/%s%s",verb,mime,date,bucket,filename);
	debug("STS is %s",sts);
	return Str("AWS %s:%s",s3_key,base64(hmac1(s3_secret,sts)));
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
		str output = readstr_socket(sc);
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

str
s3_put_resized(str bucket, str filename)
{
	str resized = Str("%s-resized",filename);
	if (resize_image(filename->data,resized->data)) {
		error("Failed to create resized %s",resized->data);
		return NULL;
	}
	return s3_put_jpeg(bucket,resized);
}

str
s3_put_orig(str bucket, str filename)
{
	str orig = Str("%s-orig",filename);
	if (copy_image(filename->data,orig->data)) {
		error("Failed to create original %s",orig);
		return NULL;
	}
	return s3_put_jpeg(bucket,orig);
}


