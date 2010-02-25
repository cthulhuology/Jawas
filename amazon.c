// amazon.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "memory.h"
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
	str sts = _("%s\n\n\%s\n%s\nx-amz-acl:public-read\n/%s/%s",verb,mime,date,bucket,filename);
	debug("STS is %s",sts);
	return _("AWS %s:%s",s3_key,base64(hmac1(s3_secret,sts)));
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
	str auth = s3_put_auth_string(_("PUT"),mime,date,bucket,filename);
	debug("AUTH is %s", auth);
	str size = _("%i",fc->st.st_size);
	debug("Size is %s",size);

	Request req = new_request(_("PUT"), _("%s.s3.amazonaws.com",bucket),_("/%s",filename));	
	request_headers(req,_("x-amz-acl"),_("public-read"));
	request_headers(req,_("Content-Type"),mime);
	request_headers(req,_("Content-Length"),size);
	request_headers(req,_("Date"),date);
	request_headers(req,_("Authorization"),auth);
	request_file(req,fc);
	request_callback(req,server.response,callback);
	send_request(req);

	debug("S3 PUT DONE");
}
