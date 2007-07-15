// js.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>

#include "defines.h"
#include "uri.h"
#include "responses.h"
#include "buffers.h"
#include "files.h"
#include "server.h"
#include "jsapi.h"
#include "js.h"

// Javascript Functions

JSInstance ins;

void ProcessFile(char* script);

static JSBool
Print(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	uintN i;
	JSString *str;
	char* cstr;

	for (i = 0; i < argc; ++i) {
		str = JS_ValueToString(cx, argv[i]);
		if (!str) return JS_FALSE;
		if (i) ins.buffer = write_buffer(ins.buffer," ",1);
		cstr = JS_GetStringBytes(str);
		ins.buffer = write_buffer(ins.buffer, cstr, strlen(cstr));
		//fprintf(stderr,"[JS %d] %s\n",ins.buffer->length,ins.buffer->data);
	}
	ins.buffer = write_buffer(ins.buffer, "\n",1);
	return JS_TRUE;
}

static JSBool
Include(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	File fc;
	JSString* str;
	char* filename;
	if (argc != 1) return JS_FALSE;
	str = JS_ValueToString(cx,argv[0]);
	filename = file_path(ins.resp->req,JS_GetStringBytes(str),JS_GetStringLength(str));
	fc = load(ins.srv,filename);
	if (!fc) return JS_FALSE;
	ProcessFile(fc->data);
	return JS_TRUE;
}

static JSBool
Header(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	JSString* str;
	if (argc != 1) return JS_FALSE;	
	str = JS_ValueToString(cx,argv[0]);
	Buffer head  = find_header(ins.resp->req->headers,JS_GetStringBytes(str));
	str = JS_NewString(cx,head->data,head->length);
	*rval = STRING_TO_JSVAL(str);
	return JS_TRUE;
}

static JSBool
Param(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	JSString* str;
	if (argc != 1) return JS_FALSE;
	str = JS_ValueToString(cx,argv[0]);
	Buffer pram = find_header(ins.resp->req->query_vars,JS_GetStringBytes(str));
	if (! pram) return JS_FALSE;
	str = JS_NewString(cx,pram->data,pram->length);
	*rval = STRING_TO_JSVAL(str);
	return JS_TRUE;
}

static JSBool
Query(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	int i,j;
	JSObject* arr;
	JSObject* row;
	char* value_cstr;
	JSString* value;
	JSString* query;
	PGresult* res;
	if (argc != 1) return JS_FALSE;
	query  = JS_ValueToString(cx,argv[0]);
	res  = PQexec(ins.database,JS_GetStringBytes(query));
	switch(PQresultStatus(res)) {
		case PGRES_EMPTY_QUERY:
		case PGRES_COMMAND_OK:
			PQclear(res);
			return JS_TRUE;
		case PGRES_TUPLES_OK:
			arr =  JS_NewArrayObject(cx,0,NULL);
			if (! arr) {
				fprintf(stderr,"Failed to initialize array for query : %s\n",JS_GetStringBytes(query));
				return JS_FALSE;
			}
			for (i = 0; i < PQntuples(res); ++i) {
				row = JS_NewObject(cx,NULL,NULL,arr);
				if(!row || ! JS_DefineElement(cx,arr,i,OBJECT_TO_JSVAL(row),NULL,NULL,JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT)) {
					fprintf(stderr,"Failed to initialize object for row %d : %s\n",i,JS_GetStringBytes(query));
					continue;
				}
				for (j = 0; j < PQnfields(res); ++j) {
					value_cstr = PQgetvalue(res,i,j);
					value = JS_NewString(cx,value_cstr,strlen(value_cstr));
					if (!JS_DefineProperty(cx,row,PQfname(res,j),STRING_TO_JSVAL(value),NULL,NULL, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT)) {
						fprintf(stderr,"Failed to apply column %s to row %d\n",PQfname(res,j),i);
						continue;
					}
				}
			}
			*rval = OBJECT_TO_JSVAL(arr);
			// PQclear(res);
			return JS_TRUE;
		case PGRES_BAD_RESPONSE:
		case PGRES_NONFATAL_ERROR:
		case PGRES_FATAL_ERROR:
		default:
			PQclear(res);
			return JS_FALSE;
	}
}

static JSBool
Encode(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	char* data;
	JSString* str;
	Buffer tmp,buf;
	if (argc != 1) return JS_FALSE;
	str = JS_ValueToString(cx,argv[0]);
	tmp = write_buffer(NULL,JS_GetStringBytes(str),JS_GetStringLength(str));
	buf = uri_encode(tmp);	
	data = print_buffer(buf);	
	str = JS_NewString(cx,data,strlen(data));
	for (; buf; buf = free_buffer(buf));
	for (; tmp; tmp = free_buffer(tmp));
	free(data);
	*rval = STRING_TO_JSVAL(str);
	return JS_TRUE;	
}

static JSBool
Decode(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	char* data;
	JSString* str;
	Buffer tmp,buf;
	if (argc != 1) return JS_FALSE;
	str = JS_ValueToString(cx,argv[0]);
	tmp = write_buffer(NULL,JS_GetStringBytes(str),JS_GetStringLength(str));
	buf = uri_decode(tmp);	
	data = print_buffer(buf);	
	fprintf(stderr,"[Decode] (%s)\n",data);
	str = JS_NewString(cx,data,buf->length);
	for (; buf; buf = free_buffer(buf));
	for (; tmp; tmp = free_buffer(tmp));
	*rval = STRING_TO_JSVAL(str);
	return JS_TRUE;	
}

static JSBool
Location(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
}

static JSBool
Expires(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
}

static JSBool
ContentType(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
}

static JSBool
CacheControl(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
}

static JSBool
Connection(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
}

static JSClass global_class = {
	"global", 0,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};

static JSFunctionSpec my_functions[] = {
	{"print", Print, 0},
	{"include", Include, 0},
	{"header", Header, 0 },
	{"param", Param, 0 },
	{"query", Query, 0 },
	{"encode", Encode, 0 },
	{"decode", Decode, 0 },
	{"location", Location, 0},
	{"expires", Expires, 0},
	{"content_type", ContentType, 0},
	{"cache_control", CacheControl, 0},
	{"connection", Connection, 0},
	{0},
};

int
InitJS(JSInstance* i, Server srv, Response resp)
{
	i->srv = srv;
	i->resp = resp;
	i->buffer = NULL;
	i->rt = JS_NewRuntime(RUNTIME_SIZE);
	if (!i->rt) return 1;
	i->cx = JS_NewContext(i->rt, CONTEXT_SIZE);
	if (!i->cx) return 1;
	i->glob = JS_NewObject(i->cx, &global_class, NULL, NULL);
	i->builtins = JS_InitStandardClasses(i->cx, i->glob);
	if (!JS_DefineFunctions(i->cx, i->glob, my_functions)) return 1;
	i->database = PQconnectdb(DB_CONNECT_STRING);
	if (!i->database) return 1;
	return 0;
}

int 
DestroyJS(JSInstance* i)
{
	PQfinish(i->database);	
	JS_DestroyContext(i->cx);
	JS_DestroyRuntime(i->rt);
	JS_ShutDown();
	return 0;
}

void
ProcessFile(char* script)
{
	Buffer tmpbuffer;
	JSBool ok;
	jsval retval;
	int tmp;
	int len = 0;
	for (tmp = 0; script[tmp]; ++tmp) {
		if (!strncmp(&script[tmp],"<?js=",5)) {
			if (len < tmp) ins.buffer = write_buffer(ins.buffer,&script[len],tmp-len);
			len = 0;
			while (strncmp(&script[tmp+len+6],"=?>",3)) ++len;
			tmpbuffer = write_buffer(NULL,"print(",6);
			tmpbuffer = write_buffer(tmpbuffer,&script[tmp+6],len);
			tmpbuffer = write_buffer(tmpbuffer,");",2);
			// fprintf(stderr,"Evaluating %d [%s]\n",tmpbuffer->length,tmpbuffer->data);
			ok = JS_EvaluateScript(ins.cx, ins.glob, tmpbuffer->data, tmpbuffer->length , "js.c", 1, &retval);
			// fprintf(stderr,"ok? %s\n", (ok == JS_FALSE ? "nope" : "yep"));
			free_buffer(tmpbuffer);
			tmp += len + 6;
			len = tmp+3;
		} else if (!strncmp(&script[tmp],"<?js",4)) {
			if (len < tmp) ins.buffer = write_buffer(ins.buffer,&script[len],tmp-len);
			len = 0;
			while (strncmp(&script[tmp+len+5],"?>",2)) ++len;
			ok = JS_EvaluateScript(ins.cx, ins.glob, &script[tmp + 5], len-1 , "js.c", 1, &retval);
			tmp += len + 5;
			len = tmp+2;
		}
	}
	ins.buffer = write_buffer(ins.buffer,&script[len],tmp-len);
}

Response
jws_handler(Server srv, File fc, Response resp)
{
	Buffer tmp;
	if (InitJS(&ins,srv,resp)) goto error;
	ProcessFile(fc->data);
	if (DestroyJS(&ins)) goto error;
	resp->contents = ins.buffer;
	print_buffer(ins.buffer);
	return resp;
error:
	for (;ins.buffer; ins.buffer = free_buffer(ins.buffer));
	resp->status = 500;
	resp->contents = NULL;
	return resp;
}

