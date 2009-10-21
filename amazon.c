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
	str sts = Str("%s\n\n\%s\n%s\nx-amz-acl:public-read\n/%s/%s",verb,mime,date,bucket,filename);
	debug("STS is %s",sts);
	return Str("AWS %s:%s",s3_key,base64(hmac1(s3_secret,sts)));
}

void
s3_put(str file, str bucket, str filename, str mime, str callback)
{
	debug("S3 PUT");
	File fc = load(file);
	debug("Loaded file %s", file);
	debug("MIME is %s",mime);
	str date = Date();
	debug("Date is %s",date);
	str md5 = base64(md5sum(fc->data,fc->st.st_size));
	debug("MD5 is %s",md5);
	str auth = s3_put_auth_string(Str("PUT"),mime,date,bucket,filename);
	debug("AUTH is %s", auth);
	str size = Str("%i",fc->st.st_size);
	debug("Size is %s",size);

	Request req = new_request(Str("PUT"), Str("%s.s3.amazonaws.com",bucket),Str("/%s",filename));	

	request_headers(req,Str("x-amz-acl"),Str("public-read"));
	request_headers(req,Str("Content-Type"),mime);
	request_headers(req,Str("Content-Length"),size);
	request_headers(req,Str("Date"),date);
	request_headers(req,Str("Authorization"),auth);

	request_file(req,fc);
	
	request_callback(req,Resp,callback);

	send_request(req);

	debug("S3 PUT DONE");
}
