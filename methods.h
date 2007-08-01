// methods.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_METHODS_H__
#define __HAVE_METHODS_H__

typedef struct method_dispatch_struct  MethodDispatch;
struct method_dispatch_struct {
	int len;
	char* name;
	int (*handler)(Server,Response);
};

int get_method(Server srv, Response resp);
int post_method(Server srv, Response resp);
int head_method(Server srv, Response resp);
int options_method(Server srv, Response resp);
int put_method(Server srv, Response resp);
int delete_method(Server srv, Response resp);
int trace_method(Server srv, Response resp);

#endif
