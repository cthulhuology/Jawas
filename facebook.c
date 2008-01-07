// facebook.c
// 
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "str.h"
#include "log.h"
#include "headers.h"
#include "sockets.h"
#include "server.h"
#include "auth.h"
#include "dates.h"
#include "facebook.h"

str facebook_key;
str facebook_secret;

void
facebook_auth(str key, str secret)
{
	facebook_key = key;
	facebook_secret = secret;
}

str
facebook_login()
{
	return Str("http://www.facebook.com/login.php?api_key=%s&v=1.0",facebook_key);
}

str
facebook_sig(Headers kv)
{
	int i;
	Headers sorted = sort_headers(kv);
	debug("Headers %s", list_headers(sorted));
	str retval = Str("%s=%s",Key(sorted,0),Value(sorted,0));
	overs(sorted,i,1) { 
		skip_null(sorted,i);
		retval = append(retval,Str("%s=%s",Key(sorted,i), Value(sorted,i)));
	}
	retval = append(retval,facebook_secret);
	char* data = dump(retval);
	debug("Producing sig from %c",data);
	retval = md5hex(data,retval->length);	
	free(data);
	return retval;
}

void
facebook_method(str method, Headers kv, str callback)
{
	
	kv = append_header(kv,Str("method"),method);
	kv = append_header(kv,Str("api_key"),facebook_key);
	kv = append_header(kv,Str("sig"),facebook_sig(kv));
	str args = url_encode_headers(kv);

	Request req = new_request(Str("POST"),Str("api.facebook.com"),Str("/restserver.php?"));

	request_headers(req,Str("Content-type"),Str("application/x-www-form-urlencded"));
	request_headers(req,Str("User-Agent"),Str("Jawas"));
	//request_headers(req,Str("Content-Length"),Str("%i",len(args)));
	
	req = request_data(req,args);

	request_callback(req,Resp,callback);

	send_request(req);
}

