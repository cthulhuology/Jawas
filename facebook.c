// facebook.c
// 
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "str.h"
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
	str retval = Str("%s=%s",sorted[0].key,sorted[0].value);
	for (i = 1; sorted[i].key; ++i) 
		retval = Str("%s%s=%s", retval, sorted[i].key, sorted[i].value);
	retval = Str("%s%s",retval,facebook_secret);
	retval =  md5hex(retval->data,retval->len);	
	return retval;
}

str
facebook_method(str method, Headers kv)
{
	int i;
	kv = append_header(kv,Str("method"),method);
	kv = append_header(kv,Str("api_key"),facebook_key);
	kv = append_header(kv,Str("sig"),facebook_sig(kv));
	str args = Str("%s=%s",kv[0].key,kv[0].value);
	for (i = 1; kv[i].key; ++i) 
		args = Str("%s&%s=%s",args, kv[i].key,kv[i].value);
	str headers = Str("Host: api.facebook.com\r\nContent-type: application/x-www-form-urlencoded\r\nUser-Agent: Jawas\r\nContent-Length: %i\r\n",args->len);
	str post = Str("POST http://api.facebook.com/restserver.php HTTP/1.1\r\n%s\r\n%s\r\n",headers,args);
	debug("posting to facebook\n%s",post);
	Socket sc = connect_socket("api.facebook.com",80);
	write_socket(sc,post->data,post->len);
	Buffer output = write_str(NULL,readstr_socket(sc));
	output = dechunk_buffer(output);
	str retval = read_str(output,0,length_buffer(output));
	close_socket(sc);
	return retval;
}

