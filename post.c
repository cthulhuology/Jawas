// post.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "str.h"
#include "buffers.h"
#include "sockets.h"
#include "post.h"

Post
new_post(str host, str path)
{
	Post retval = (Post)salloc(sizeof(struct post_struct));
	retval->host = host;
	retval->path = path;
	retval->headers = new_headers();
	retval->body = NULL;
	retval->sc = NULL;
	retval->resp = NULL;
	retval->status = 0;
	return retval;
}

Post
set_post_header(Post p, str key, str value)
{
	append_header(p->headers,key,value);	
	return p;
}

Post
set_post_data(Post p, str data)
{
	p->body = write_str(p->body,data);
	return p;
}

int
post(Post p)
{
	int len,total = 0;
	if (!p) return 1;	
	connection(p->headers,"close");
	transfer_encoding(p->headers,"chunked");
	Buffer msg = print_buffer(NULL,"POST %s HTTP/1.1\r\nHost: %s\r\n",p->path,p->host);
	msg = print_headers(msg,p->headers);	
	msg = print_buffer(msg,"\r\n");
	str host = name_field(p->host);
	str port = skip_fields(p->host,0);
	int port_num = (port == p->host) ? 80 : str_int(port);
	debug("Host: %s port: %i",host,port_num);
	p->sc = connect_socket(host->data,port_num);
	if (!p->sc ) return 1;
	len = length_buffer(msg);
	for (total = 0; total < len; total += send_contents(p->sc,msg,0));
	len = length_buffer(p->body);
	for (total = 0; total < len; total += send_contents(p->sc,p->body,1));
	write_chunked_socket(p->sc,NULL,0);
	return 0;
}

str
post_response(Post p)
{
	p->resp = readstr_socket(p->sc);	
	debug("Response:\n%s",p->resp);
	return p->resp;
}
