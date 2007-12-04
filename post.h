// post.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_POST__H__
#define __HAVE_POST__H__

#include "str.h"
#include "buffers.h"
#include "headers.h"

typedef struct post_struct* Post;
struct post_struct {
	str host;
	str path;
	Headers headers;
	Buffer body;
	Socket sc;
	str resp;
	int status;
};

Post new_post(str host, str path);
Post set_post_header(Post p, str key, str value);
Post set_post_data(Post p, str data);
int post(Post p);
str post_response(Post p);

#endif
