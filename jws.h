// jws.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "headers.h"
#include "buffers.h"
#include "responses.h"
#include "files.h"
#include "libpq-fe.h"
#include "server.h"
#include "jsatom.h"
#include "jsapi.h"

typedef struct js_instance_struct JSInstance;

struct js_instance_struct { 
	JSRuntime* rt;
	JSContext* cx;
	JSObject* glob;
	JSBool builtins;
	Server srv;
	Response resp;
	Buffer buffer;
	Headers query_string;
	PGconn* database;
};

extern JSInstance ins;

int jws_handler(File fc);
int run_script(File fc, Headers data);

int InitJS(JSInstance* i, Server srv, Headers data);
int DestroyJS(JSInstance* i);

#define HeaderSetter(func) \
	func(ins.resp->headers,JS_GetStringBytes(JS_ValueToString(cx,argv[0])));\
	return JS_TRUE; 

