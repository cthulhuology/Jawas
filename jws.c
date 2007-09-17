// jws.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "pages.h"
#include "log.h"
#include "uri.h"
#include "responses.h"
#include "buffers.h"
#include "files.h"
#include "server.h"
#include "jsapi.h"
#include "jsstr.h"
#include "jws.h"

// Javascript Functions

JSInstance ins;
jmp_buf jmp;

void ProcessFile(char* script);

int
sms_connect() 
{
	struct sockaddr_un unsc;
#ifndef LINUX
	unsc.sun_len = sizeof(unsc);
#endif
	unsc.sun_family = AF_UNIX;
	memcpy(unsc.sun_path,"/tmp/sms",9);

	int sock = socket(AF_UNIX,SOCK_STREAM,0);
	if (connect(sock,(struct sockaddr*)&unsc,SUN_LEN(&unsc))) {
		error("Failed to connect to /tmp/sms\n");
		close(sock);
		return 0;
	}
	return sock;
}

static JSBool
ExitJS(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	longjmp(jmp,1);
	return JS_TRUE;	// never get here
}

static JSBool
SendMessage(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	int i;
	JSString* s;
	int sc = sms_connect();
	write(sc,"SEND ",5);
	for (i = 0; i < argc; ++i ) {
		s = JS_ValueToString(cx,argv[i]);
		write(sc,JS_GetStringBytes(s),JS_GetStringLength(s));
		write(sc," ",1);
	}
	close(sc);
	return JS_TRUE;	
}

static JSBool
AddUser(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	int i;
	JSString* s;
	int sc = sms_connect();
	write(sc,"ADD USER ",9);
	for (i = 0; i < 3; ++i ) {
		s = JS_ValueToString(cx,argv[i]);
		write(sc,JS_GetStringBytes(s),JS_GetStringLength(s));
		write(sc," ",1);
	}
	close(sc);
	return JS_TRUE;	
}

static JSBool
AddChannel(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	JSString* s;
	int sc = sms_connect();
	write(sc,"ADD CHANNEL ",12);
	s = JS_ValueToString(cx,argv[0]);
	write(sc,JS_GetStringBytes(s),JS_GetStringLength(s));
	close(sc);
	return JS_TRUE;	
}

static JSBool
Print(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	uintN i;
	JSString *s;
	char* c;

	for (i = 0; i < argc; ++i) {
		s = JS_ValueToString(cx, argv[i]);
		if (!s) return JS_FALSE;
		if (i) ins.buffer = write_buffer(ins.buffer," ",1);
		c = JS_GetStringBytes(s);
		ins.buffer = write_buffer(ins.buffer, c, JS_GetStringLength(s));
	}
	ins.buffer = write_buffer(ins.buffer, "\n",1);
	return JS_TRUE;
}

static JSBool
CWD(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	JSString* s = JS_NewString(cx,Req->path->data,Req->path->len);
	*rval = STRING_TO_JSVAL(s);
}

static JSBool
Include(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	File fc;
	JSString* s;
	str filename;
	if (argc != 1) return JS_FALSE;
	s = JS_ValueToString(cx,argv[0]);
	filename = file_path(Req->host,Str("/%s",char_str(JS_GetStringBytes(s),JS_GetStringLength(s))));
	fc = load(filename);
	if (!fc) return JS_FALSE;
	ProcessFile(fc->data);
	return JS_TRUE;
}

static JSBool
Use(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	File fc;
	JSString* s;
	str filename;
	if (argc != 1) return JS_FALSE;
	s = JS_ValueToString(cx,argv[0]);
	filename = file_path(Req->host,Str("/%s",char_str(JS_GetStringBytes(s),JS_GetStringLength(s))));
	fc = load(filename);
	if (!fc) return JS_FALSE;
	if (JS_FALSE == JS_EvaluateScript(ins.cx, ins.glob, fc->data, fc->st.st_size, "js.c", 1, rval)) {
		debug("Failed to evaluate script %s",filename);
		return JS_FALSE;
	}
	return JS_TRUE;
}

static JSBool
Header(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	JSString* s;
	if (argc != 1) return JS_FALSE;	
	s = JS_ValueToString(cx,argv[0]);
	str head  = find_header(ins.resp->req->headers,JS_GetStringBytes(s));
	s = JS_NewString(cx,head->data,head->len);
	*rval = STRING_TO_JSVAL(s);
	return JS_TRUE;
}

static JSBool
Param(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	JSString* s;
	if (argc != 1) return JS_FALSE;
	s = JS_ValueToString(cx,argv[0]);
	str pram = find_header(ins.resp->req->query_vars,JS_GetStringBytes(s));
	if (! pram) return JS_FALSE;
	s = JS_NewString(cx,pram->data,pram->len);
	*rval = STRING_TO_JSVAL(s);
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
				error("Failed to initialize array for query : %c\n",JS_GetStringBytes(query));
				return JS_FALSE;
			}
			for (i = 0; i < PQntuples(res); ++i) {
				row = JS_NewObject(cx,NULL,NULL,arr);
				if(!row || ! JS_DefineElement(cx,arr,i,OBJECT_TO_JSVAL(row),NULL,NULL,JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT)) {
					error("Failed to initialize object for row %i : %c\n",i,JS_GetStringBytes(query));
					continue;
				}
				for (j = 0; j < PQnfields(res); ++j) {
					value_cstr = PQgetvalue(res,i,j);
					value = JS_NewString(cx,value_cstr,strlen(value_cstr));
					if (!JS_DefineProperty(cx,row,PQfname(res,j),STRING_TO_JSVAL(value),NULL,NULL, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT)) {
						error("Failed to apply column %c to row %i\n",PQfname(res,j),i);
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
	JSString* s;
	Buffer tmp,buf;
	if (argc != 1) return JS_FALSE;
	s = JS_ValueToString(cx,argv[0]);
	data = uri_encode(JS_GetStringBytes(s));
	s = JS_NewString(cx,data,strlen(data));
	*rval = STRING_TO_JSVAL(s);
	return JS_TRUE;	
}

static JSBool
Decode(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	char* data;
	JSString* s;
	Buffer tmp,buf;
	if (argc != 1) return JS_FALSE;
	s = JS_ValueToString(cx,argv[0]);
	data = uri_decode(JS_GetStringBytes(s));
	s = JS_NewString(cx,data,buf->length);
	*rval = STRING_TO_JSVAL(s);
	return JS_TRUE;	
}

static JSBool
Location(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	HeaderSetter(location)
}

static JSBool
Expires(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	HeaderSetter(expires)
}

static JSBool
ContentType(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	HeaderSetter(content_type)
}

static JSBool
CacheControl(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	HeaderSetter(cache_control)
}

static JSBool
Connection(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	HeaderSetter(connection)
}

static JSBool
ClientInfo(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	Socket sc;
	ins.buffer = print_buffer(ins.buffer,"<table>");
	ins.buffer = print_buffer(ins.buffer,"<tr><td>Max Clients:</td><td> %i</td></tr>",gsci.max);
	ins.buffer = print_buffer(ins.buffer,"<tr><td>Current Clients:</td><td> %i</td></tr>",gsci.current);
	ins.buffer = print_buffer(ins.buffer,"</table><hr /><ol>");
	for (sc = ins.srv->sc; sc; sc = sc->next) 
		ins.buffer = print_buffer(ins.buffer,"<li>%i.%i.%i.%i:%i</li>",
			(0xff & sc->peer),
			(0xff00 & sc->peer) >> 8,
			(0xff0000 & sc->peer) >> 16,
			(0xff000000 & sc->peer) >> 24,
			sc->port);
	ins.buffer = print_buffer(ins.buffer,"</ol>");
	return JS_TRUE;
}

static JSBool
HitInfo(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	return JS_TRUE;
}

static JSBool
MemInfo(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
		
	ins.buffer = print_buffer(ins.buffer,"<table>");
	ins.buffer = print_buffer(ins.buffer,"<tr><td>Base Address:</td><td> %p</td></tr>",gpi.baseaddr);
	ins.buffer = print_buffer(ins.buffer,"<tr><td>Free Memory Size:</td><td> %i</td></tr>",gpi.size - (gpi.allocated * getpagesize()));
	ins.buffer = print_buffer(ins.buffer,"<tr><td>Allocated Memory:</td><td> %i</td></tr>",gpi.allocated * getpagesize());
	ins.buffer = print_buffer(ins.buffer,"<tr><td>Scratch Memory:</td><td> %i</td></tr>",gsi.current);
	ins.buffer = print_buffer(ins.buffer,"<tr><td>Max Scratch Memory:</td><td> %i</td></tr>",gsi.max_memory);
	ins.buffer = print_buffer(ins.buffer,"<tr><td>Scratch Memory Allocations:</td><td> %i</td></tr>",gsi.allocs);
	ins.buffer = print_buffer(ins.buffer,"<tr><td>Pages Allocated:</td><td> %i</td></tr>",gpi.allocated);
	ins.buffer = print_buffer(ins.buffer,"<tr><td>Scratch Pages Allocated:</td><td> %i</td></tr>",gsi.scratches);
	ins.buffer = print_buffer(ins.buffer,"<tr><td>Page Allocations:</td><td> %i</td></tr>",gpi.allocations);
	ins.buffer = print_buffer(ins.buffer,"<tr><td>Freed Pages:</td><td> %i</td></tr>",gpi.frees);
	ins.buffer = print_buffer(ins.buffer,"<tr><td>Freed Scratch Pages:</td><td> %i</td></tr>",gsi.frees);
	ins.buffer = print_buffer(ins.buffer,"</table>");
	return JS_TRUE;
}

static JSBool
FileInfo(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	File fc;
	int total = 0;
	ins.buffer = print_buffer(ins.buffer,"<table><tr><th>Name</th><th>Hits</th><th>Size</th></tr>");
	for (fc = ins.srv->fc; fc; fc = fc->next)  {
		ins.buffer = print_buffer(ins.buffer,"<tr><td>%c</td><td>%i</td><td>%i</td></tr>",&fc->name[cwd->len],fc->count,fc->st.st_size);
		total += fc->st.st_size;
	}
	ins.buffer = print_buffer(ins.buffer,"<tr><td colspan=2>Total:</td><td>%i</td></tr></table>",total);
	return JS_TRUE;
}

static JSBool
GetGuid(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	PGresult* res = PQexec(ins.database,"SELECT nextval('guid_seq')");
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		error("Result status not ok");	
		return JS_FALSE;
	}
	char* guid = PQgetvalue(res,0,0);
	debug("Guid is %c",guid);
        *rval = STRING_TO_JSVAL(JS_NewString(cx,guid,strlen(guid)));
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
	{"use", Use, 0},
	{"cwd", CWD, 0},
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
	{"hit_info", HitInfo, 0},
	{"client_info", ClientInfo, 0},
	{"mem_info", MemInfo, 0},
	{"file_info", FileInfo, 0},
	{"guid", GetGuid, 0 },
	{0},
};

static int
CreateDatabaseTableFunctions(JSInstance* in)
{
	int i;
	PGresult* res;
	const char* args[] = { "id","obj",NULL };
	char query[] = "SELECT tablename FROM pg_tables WHERE schemaname = 'public' AND tableowner = 'jawas'";
	res = PQexec(in->database,query);	
	if (PQresultStatus(res) != PGRES_TUPLES_OK) return 1;
	for (i = 0; i < PQntuples(res); ++i) {
		char* table = PQgetvalue(res,i,0);
		str s = Str(
"	if (id) {													" 
"		if (obj) {  												"
"			var kv = [];											"
"			for (var k in obj) {										"
"				if (k != 'id') kv.push(k + \"= '\" + obj[k] + \"'\");					"
"			}												"
"			query(\"UPDATE %c SET \" + kv.join(\", \") + \" WHERE id = \" + id);				"
"			return id;											"
"		} else {												" 
"			var res = query(\"SELECT * FROM %c WHERE id = '\" + id + \"'\"); 				"
"			return res[0]; 											"
"		} 													"
"	} else {													"
"		var gid = guid();											"
"		var keys = [];												"
"		var values = [];											"
"		for (var k in obj) {											"
"			if (k != 'id') {										"
"				keys.push(k);										"
"				values.push(obj[k]);									"
"			}												"
"		}													"
"		keys.push('id');											"
"		values.push(gid);											"
"		query(\"INSERT INTO %c (\" + keys.join(\", \") + \") VALUES ('\" + values.join(\"', '\") + \"')\");	"
"		return gid; 												"
"	}														",
		table,table,table,table,table,table);
		if (NULL == JS_CompileFunction(in->cx,in->glob,table,2,args,s->data,s->len,"jws.c",0)) 
			debug("Failed to compile script %s",s);
	}
	return 0;
}

int
InitParams(JSInstance* in)
{
	str x;
	Headers headers = in->resp->req->query_vars;
	JSString* s;
	JSObject* o;
	JSObject* arr = NULL;
	int i,j, n;
	if (! headers) return 0;
	for (i = 0; i < MAX_HEADERS && headers[i].key; ++i) {
		if (headers[i].key == (str)-1) continue;
		x = Str("$%s",headers[i].key);
		debug("Initializing %s = %s",x,headers[i].value);
		n = 0;
		arr = NULL;
		s = JS_NewString(in->cx,headers[i].value->data,headers[i].value->len);
		if (JS_FALSE == JS_DefineProperty(in->cx,in->glob,x->data,STRING_TO_JSVAL(s),NULL, NULL,JSPROP_READONLY))
			debug("Failed to set property %s",x);
		for (j = i+1; j < MAX_HEADERS && headers[j].key; ++j) {
			if (headers[j].key == (str)-1) continue;
			if (cmp_str(headers[i].key,headers[j].key)) {
				if (! arr) {
					arr =  JS_NewArrayObject(in->cx,0,NULL);
					JS_DefineElement(in->cx,arr,n,STRING_TO_JSVAL(s),NULL,NULL,JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
					debug("Adding %s to array %s",headers[i].value,x);
				}
				s = JS_NewString(in->cx,headers[j].value->data,headers[j].value->len);
				debug("Adding %s to array %s",headers[j].value,x);
				JS_DefineElement(in->cx,arr,++n,STRING_TO_JSVAL(s),NULL,NULL,JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
				headers[j].key = (str)-1;
			}
		}
		if (arr && JS_FALSE == JS_DefineProperty(in->cx,in->glob,x->data,OBJECT_TO_JSVAL(arr),NULL, NULL,JSPROP_READONLY))
			debug("Failed to set property %s",x);
	}
        return 0;
}


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
	InitParams(i);
	return CreateDatabaseTableFunctions(i);
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
	str scratch;
	JSBool ok;
	jsval retval;
	int o;
	int len = 0;
	for (o = 0; script[o]; ++o) {
		if (!strncmp(&script[o],"<?js",4)) {
			if (len < o)
				ins.buffer = write_buffer(ins.buffer,&script[len],o-len);
			len = 0;
			if (script[o+4] == '=') {
				while (strncmp(&script[o+len+6],"=?>",3)) ++len;
				scratch = char_str(&script[o+6],len);	
				scratch = Str("print(%s);",scratch);
				o += len + 6;
				len = o+3;
			} else {
				while (strncmp(&script[o+len+5],"?>",2)) ++len;
				scratch = char_str(&script[o+5],len-1);
				o += len + 5;
				len = o+2;
			}
			if (JS_FALSE == JS_EvaluateScript(ins.cx, ins.glob, scratch->data, scratch->len, "js.c", 1, &retval))
				notice("Failed to evaluate [%s]", scratch);
		}
	}
	ins.buffer = write_buffer(ins.buffer,&script[len],o-len);
}

int
jws_handler(File fc)
{
	Buffer tmp;
	if (InitJS(&ins,srv,Resp)) goto error;
	if (!setjmp(jmp)) {
		ProcessFile(fc->data);
	}
	if (DestroyJS(&ins)) goto error;
	Resp->contents = ins.buffer;
	return Resp->status;
error:
	for (;ins.buffer; ins.buffer = free_buffer(ins.buffer));
	Resp->contents = NULL;
	return 500;
}

