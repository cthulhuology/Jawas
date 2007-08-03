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
	int (*handler)();
};

int get_method();
int post_method();
int head_method();
int options_method();
int put_method();
int delete_method();
int trace_method();
int dispatch_method(str method);

#define NOT_IMPLEMENTED(x) \
	int x() { return error_handler(405); }

#endif
