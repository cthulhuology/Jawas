// jws.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include <sys/un.h>

#include "include.h"
#include "defines.h"
#include "log.h"
#include "uri.h"
#include "responses.h"
#include "buffers.h"
#include "files.h"
#include "server.h"
#include "jsapi.h"
#include "jws.h"

// Javascript Functions

JSInstance ins;

void ProcessFile(char* script);

int
sms_connect() 
{
	struct sockaddr_un unsc;
	unsc.sun_len = sizeof(unsc);
	unsc.sun_family = AF_UNIX;
	memcpy(unsc.sun_path,"/tmp/sms",9);

	int sock = socket(AF_UNIX,SOCK_STREAM,0);
	if (connect(sock,(struct sockaddr*)&unsc,unsc.sun_len)) {
		error("Failed to connect to /tmp/sms\n");
		close(sock);
		return 0;
	}
	return sock;
}

static JSBool
SendMessage(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	int i;
	JSString* str;
	int sc = sms_connect();
	write(sc,"SEND ",5);
	for (i = 0; i < argc; ++i ) {
		str = JS_ValueToString(cx,argv[i]);
		write(sc,JS_GetStringBytes(str),JS_GetStringLength(str));
		write(sc," ",1);
	}
	close(sc);
	return JS_TRUE;	
}

static JSBool
AddUser(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	int i;
	JSString* str;
	int sc = sms_connect();
	write(sc,"ADD USER ",9);
	for (i = 0; i < 3; ++i ) {
		str = JS_ValueToString(cx,argv[i]);
		write(sc,JS_GetStringBytes(str),JS_GetStringLength(str));
		write(sc," ",1);
	}
	close(sc);
	return JS_TRUE;	
}

static JSBool
AddChannel(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	JSString* str;
	int sc = sms_connect();
	write(sc,"ADD CHANNEL ",12);
	str = JS_ValueToString(cx,argv[0]);
	write(sc,JS_GetStringBytes(str),JS_GetStringLength(str));
	close(sc);
	return JS_TRUE;	
}

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
	filename = file_path(ins.resp->req->host->data,ins.resp->req->host->length,JS_GetStringBytes(str),JS_GetStringLength(str));
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
				error("Failed to initialize array for query : %s\n",JS_GetStringBytes(query));
				return JS_FALSE;
			}
			for (i = 0; i < PQntuples(res); ++i) {
				row = JS_NewObject(cx,NULL,NULL,arr);
				if(!row || ! JS_DefineElement(cx,arr,i,OBJECT_TO_JSVAL(row),NULL,NULL,JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT)) {
					error("Failed to initialize object for row %i : %s\n",i,JS_GetStringBytes(query));
					continue;
				}
				for (j = 0; j < PQnfields(res); ++j) {
					value_cstr = PQgetvalue(res,i,j);
					value = JS_NewString(cx,value_cstr,strlen(value_cstr));
					if (!JS_DefineProperty(cx,row,PQfname(res,j),STRING_TO_JSVAL(value),NULL,NULL, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT)) {
						error("Failed to apply column %s to row %i\n",PQfname(res,j),i);
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
	str = JS_NewString(cx,data,buf->length);
	for (; buf; buf = free_buffer(buf));
	for (; tmp; tmp = free_buffer(tmp));
	*rval = STRING_TO_JSVAL(str);
	return JS_TRUE;	
}

static JSBool
Location(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	JSString* str;	
	if (argc != 1) return JS_FALSE;
	str = JS_ValueToString(cx,argv[0]);
	location(ins.resp->headers,JS_GetStringBytes(str));
	debug("Loaction: %s",JS_GetStringBytes(str));
	return JS_TRUE;
}

static JSBool
Expires(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	JSString* str;	
	if (argc != 1) return JS_FALSE;
	str = JS_ValueToString(cx,argv[0]);
	expires(ins.resp->headers,JS_GetStringBytes(str));
	debug("Expires: %s",JS_GetStringBytes(str));
	return JS_TRUE;
}

static JSBool
ContentType(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	JSString* str;	
	if (argc != 1) return JS_FALSE;
	str = JS_ValueToString(cx,argv[0]);
	content_type(ins.resp->headers,JS_GetStringBytes(str));
	debug("ContentType: %s",JS_GetStringBytes(str));
	return JS_TRUE;
}

static JSBool
CacheControl(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	JSString* str;	
	if (argc != 1) return JS_FALSE;
	str = JS_ValueToString(cx,argv[0]);
	cache_control(ins.resp->headers,JS_GetStringBytes(str));
	debug("CacheControl: %s",JS_GetStringBytes(str));
	return JS_TRUE;
}

static JSBool
Connection(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	JSString* str;	
	if (argc != 1) return JS_FALSE;
	str = JS_ValueToString(cx,argv[0]);
	connection(ins.resp->headers,JS_GetStringBytes(str));
	debug("Connection: %s",JS_GetStringBytes(str));
	return JS_TRUE;
}

static JSClass global_class = {
	"global", 0,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};

static JSFunctionSpec my_functions[] = {
	{"send_message",SendMessage,0},
	{"add_user",AddUser,0},
	{"add_channel",AddChannel,0},
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
			ok = JS_EvaluateScript(ins.cx, ins.glob, tmpbuffer->data, tmpbuffer->length , "js.c", 1, &retval);
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

int
jws_handler(Server srv, File fc, Response resp)
{
	Buffer tmp;
	if (InitJS(&ins,srv,resp)) goto error;
	ProcessFile(fc->data);
	if (DestroyJS(&ins)) goto error;
	resp->contents = ins.buffer;
	print_buffer(ins.buffer);
	return resp->status;
error:
	for (;ins.buffer; ins.buffer = free_buffer(ins.buffer));
	resp->contents = NULL;
	return 500;
}
