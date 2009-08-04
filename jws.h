// jws.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "str.h"
#include "headers.h"
#include "requests.h"
#include "responses.h"
#include "files.h"
#include "server.h"
#include "jsatom.h"
#include "jsapi.h"
#include "database.h"

typedef struct js_instance_struct JSInstance;
struct js_instance_struct { 
	JSRuntime* rt;
	JSContext* cx;
	JSObject* glob;
	JSObject* obj_proto;
	JSBool builtins;
	Server srv;
	Response resp;
	str buffer;
	Headers query_string;
	Database database;
};

extern JSInstance ins;

int jws_handler(File fc);
int run_script(File fc, Headers data);

int InitJS(JSInstance* i, Server srv, Headers data);
int DestroyJS(JSInstance* i);

#define HeaderSetter(func) \
	func(ins.resp->headers,JS_GetStringBytes(JS_ValueToString(cx,argv[0])));\
	return JS_TRUE; 

#define EMPTY OBJECT_TO_JSVAL(NULL)

#define jsval2str(x) js2str(cx,x)
#define str2jsval(x) str2js(cx,x)

str js2str(JSContext* cx, jsval x);
jsval str2js(JSContext* cx, str x);

#define SUCCESS BOOLEAN_TO_JSVAL(JS_FALSE)
#define FAILURE BOOLEAN_TO_JSVAL(JS_TRUE)

int process_callback(str cb, Headers headers);

// Javascript Functions

