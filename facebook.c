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
	retval =  md5hex(retval->data,retval->length);	
	return retval;
}

str
facebook_method(str method, Headers kv)
{
	int i;
	str retval;
	kv = append_header(kv,Str("method"),method);
	kv = append_header(kv,Str("api_key"),facebook_key);
	kv = append_header(kv,Str("sig"),facebook_sig(kv));
	str args = Str("%s=%s",kv->slots[0].key,kv->slots[0].value);
	overs(kv,i,1) {
		skip_null(kv,i);
		args = Str("%s&%s=%s",args, kv->slots[i].key,kv->slots[i].value);
	}

	str headers = Str("Host: api.facebook.com\r\nContent-type: application/x-www-form-urlencoded\r\nUser-Agent: Jawas\r\nContent-Length: %i\r\n",len(args));
	str post = Str("POST http://api.facebook.com/restserver.php HTTP/1.1\r\n%s\r\n%s\r\n",headers,args);
	debug("posting to facebook\n%s",post);
	Socket sc = connect_socket("api.facebook.com",80);
	write_socket(sc,post);

	retval = read_socket(sc);
	retval = dechunk(retval);
	close_socket(sc);
	return retval;
}

