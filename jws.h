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

int jws_handler(Server srv, File fc, Response resp);
