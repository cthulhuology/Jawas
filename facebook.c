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
	return $("http://www.facebook.com/login.php?api_key=%s&v=1.0",facebook_key);
}

str
facebook_sig(Headers kv)
{
	int i;
	Headers sorted = sort_headers(kv);
	debug("Headers %s", list_headers(sorted));
	str retval = $("%s=%s",Key(sorted,0),Value(sorted,0));
	overs(sorted,i,1) { 
		skip_null(sorted,i);
		retval = append(retval,$("%s=%s",Key(sorted,i), Value(sorted,i)));
	}
	retval = append(retval,facebook_secret);
	debug("FB SIG HEADERS is [%s]",retval);
	retval = md5hex(retval->data,len(retval));	
	return retval;
}

void
facebook_method(str method, Headers kv, str callback)
{
	
	kv = append_header(kv,$("method"),method);
	kv = append_header(kv,$("api_key"),facebook_key);
	kv = append_header(kv,$("sig"),facebook_sig(kv));
	str args = url_encode_headers(kv);

	Request req = new_request($("POST"),$("api.facebook.com"),$("/restserver.php"));
	request_headers(req,$("Content-Type"),$("application/x-www-form-urlencoded"));
	request_headers(req,$("User-Agent"),$("Jawas"));
	request_headers(req,$("Content-Length"),$("%i",len(args)));
	request_data(req,args);
	request_callback(req,server.response,callback);
	send_request(req);
}

