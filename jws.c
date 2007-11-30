// jws.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "config.h"
#include "alloc.h"
#include "pages.h"
#include "headers.h"
#include "log.h"
#include "uri.h"
#include "requests.h"
#include "responses.h"
#include "buffers.h"
#include "files.h"
#include "server.h"
#include "jsapi.h"
#include "jsstr.h"
#include "jsobj.h"
#include "mail.h"
#include "amazon.h"
#include "wand.h"
#include "facebook.h"
#include "timers.h"
#include "database.h"
#include "jws.h"
#include "json.h"
#include "sms.h"
#include "post.h"

JSInstance ins;
jmp_buf jmp;

void ProcessFile(char* script);

static JSBool
ExitJS(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	longjmp(jmp,1);
	return JS_TRUE;	// never get here
}

static JSBool
Print(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	uintN i;
	JSString *s;
	char* c;

	for (i = 0; i < argc; ++i) {
		s = JS_ValueToString(cx, argv[i]);
		if (!s) {
			error("print() failed, invalid string parameter at %i",i);
			*rval = FAILURE;
			return JS_TRUE;
		}
		c = JS_GetStringBytes(s);
		ins.buffer = write_buffer(ins.buffer, c, JS_GetStringLength(s));
	}
	*rval = SUCCESS;
	return JS_TRUE;
}

static JSBool
Debug(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	uintN i;
	for (i = 0; i < argc; ++i) 
		debug("%s",jsval2str(argv[i]));
	*rval = SUCCESS;
	return JS_TRUE;
}

static JSBool
Error(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	uintN i;
	for (i = 0; i < argc; ++i) 
		error("%s",jsval2str(argv[i]));
	*rval = SUCCESS;
	return JS_TRUE;
}

static JSBool
CWD(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	*rval = str2jsval(Req->path);
	return JS_TRUE;
}

static JSBool
Include(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	File fc;
	JSString* s;
	str filename;
	if (argc != 1) {
		error("Usage: include(filename);");
		*rval = FAILURE;
		return JS_TRUE;
	}
	s = JS_ValueToString(cx,argv[0]);
	filename = file_path(Req->host,Str("/%s",char_str(JS_GetStringBytes(s),JS_GetStringLength(s))));
	fc = load(filename);
	if (!fc) {
		error("Failed to open file %s",filename);
		*rval = FAILURE;
		return JS_TRUE;
	}
	ProcessFile(fc->data);
	*rval = SUCCESS;
	return JS_TRUE;
}

static JSBool
Use(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	File fc;
	JSString* s;
	str filename;
	if (argc != 1) {
		error("Usage: use(filename);");
		*rval = FAILURE;
		return JS_TRUE;
	}
	s = JS_ValueToString(cx,argv[0]);
	filename = file_path(Req->host,Str("/%s",char_str(JS_GetStringBytes(s),JS_GetStringLength(s))));
	fc = load(filename);
	if (!fc) {
		error("Failed to open file %s",filename);
		*rval = FAILURE;
		return JS_TRUE;
	}
	if (JS_FALSE == JS_EvaluateScript(ins.cx, ins.glob, fc->data, fc->st.st_size, "js.c", 1, rval)) {
		debug("Failed to evaluate script %s",filename);
		*rval = FAILURE;
		return JS_TRUE;
	}
	*rval = SUCCESS;
	return JS_TRUE;
}

static JSBool
Header(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	JSString* s;
	if (argc != 1) {
		error("Usage: header(fieldname)");
		*rval = EMPTY;
		return JS_TRUE;	
	}
	s = JS_ValueToString(cx,argv[0]);
	str head  = find_header(ins.resp->req->headers,JS_GetStringBytes(s));
	if (!head) {
		*rval = EMPTY;
		return JS_TRUE;
	}
	*rval = str2jsval(head);
	return JS_TRUE;
}

static JSBool
Param(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	JSString* s;
	if (argc != 1) {
		error("Usage: param(variable)");
		*rval = EMPTY;
		return JS_TRUE;
	}
	s = JS_ValueToString(cx,argv[0]);
	str pram = find_header(ins.resp->req->query_vars,JS_GetStringBytes(s));
	if (! pram) {
		*rval = EMPTY;
		return JS_TRUE;
	}
	*rval = str2jsval(pram);
	return JS_TRUE;
}

static JSBool
Now(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	str n = int_str(time(NULL));
	*rval = str2jsval(n);
	return JS_TRUE;
}

static JSBool
Query(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	int i = 0,j = 0,o = 0;
	JSObject* arr;
	JSObject* row;
	if (argc < 1) {
		error("Usage: query(querystring,[...])");
		*rval = EMPTY;
		return JS_TRUE;
	}
	str qstr = jsval2str(argv[0]);
	str qry = NULL;
	if (argc > 1) 
		for (i = 1; i < qstr->len && j < argc; ++i)
			if (qstr->data[i] == '?') {
				++j;
				str val = jsval2str(argv[j]);
				debug("Value [%i] is %s",j,val);
				val = singlequote(val);
				debug("Single Quoted Value is %s",val);
				qry = qry ? Str("%s%s%s",qry,sub_str(qstr,o,i),val) : Str("%s%s",sub_str(qstr,o,i),val);
				debug("Qry is %s",qry);
				o = i + 1;
			}
	qry = qry ? Str("%s%s",qry,sub_str(qstr,o,qstr->len)) : qstr;
	debug("[Query] %s",qry);
	int res = query(qry);
	if (res < 0) {
		*rval = str2jsval(db_error());
		return JS_TRUE;
	}
	arr =  JS_NewArrayObject(cx,0,NULL);
	if (! arr) {
		error("Failed to initialize array for query : %s\n",qstr);
		*rval = EMPTY;
		return JS_TRUE;
	}
	for (i = 0; i < res; ++i) {
		row = JS_NewObject(cx,NULL,NULL,arr);
		if(!row || ! JS_DefineElement(cx,arr,i,OBJECT_TO_JSVAL(row),NULL,NULL,JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT)) {
			error("Failed to initialize object for row %i : %s\n",i,qstr);
			continue;
		}
		for (j = 0; j < fields(); ++j) {
			str value = fetch(i,j);
			if (!JS_DefineProperty(cx,row,field(j)->data,str2jsval(value),NULL,NULL, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT)) {
				error("Failed to apply column %c to row %i\n",field(j),i);
				continue;
			}
		}
	}
	reset();
	*rval = OBJECT_TO_JSVAL(arr);
	return JS_TRUE;
}

static JSBool
Encode(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	char* data;
	str s;
	Buffer tmp,buf;
	if (argc != 1) {
		error("Usage: encode(string)");	
		*rval = EMPTY;
		return JS_TRUE;
	}
	s = jsval2str(argv[0]);
	s = uri_encode(s);
	*rval = str2jsval(s);
	return JS_TRUE;	
}

static JSBool
Decode(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	char* data;
	str s;
	Buffer tmp,buf;
	if (argc != 1) {
		error("Usage: encode(string)");	
		*rval = EMPTY;
		return JS_TRUE;
	}
	s = jsval2str(argv[0]);
	s = uri_decode(s);
	*rval = str2jsval(s);
	return JS_TRUE;	
}

static JSBool
Status(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	if (argc != 1) {
		error("Usage: status(302)");
		*rval = FAILURE;
		return JS_TRUE;
	}
	Resp->status = str_int(jsval2str(argv[0]));
	longjmp(jmp,1);
	return JS_TRUE; // never get here
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
RequestInfoTable(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	RequestInfo ri;
	ins.buffer = print_buffer(ins.buffer,"<table><tr><th>Request</th><th># Hits</th><th>Avg Time</th><th>PPM</th></tr>");
	for (ri  = ins.srv->ri; ri; ri = ri->next) {
		ins.buffer = print_buffer(ins.buffer, "<tr><td>http://%s%s</td><td>%i</td><td>%i&mu;s</td><td>%i</td></tr>",ri->host,ri->path,ri->hits,ri->time, 60000000 / ri->time);
	}
	ins.buffer = print_buffer(ins.buffer,"</table>");
	return JS_TRUE;
}

static JSBool
JSON(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	str jsn = json(cx,obj);
	debug("JSON value %s",jsn);
	*rval = str2jsval(jsn);
	return JS_TRUE;
}

static JSBool
SaveJSON(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	if (argc > 0) {
		error("Usage: obj.save()");
		*rval = EMPTY;
		return JS_TRUE;
	}
	jsval id_val;
	str guid;
	if (JS_FALSE == JS_GetProperty(ins.cx,obj,"id",&id_val)) return JS_TRUE;
	return JS_TRUE;	
}

static JSBool
StoreJSON(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	str id,qstr;
	if (argc > 1) {
		error("Usage: obj.store([guid])");
		*rval = EMPTY;
		return JS_TRUE;
	}
	if (argc == 0) { 
		id = guid();
		qstr = Str("INSERT INTO object (id,data) VALUES (%s, '%s')",id,json(cx,obj));
	} else {
		id = jsval2str(argv[0]);
		qstr = Str("UPDATE object SET data = '%s' WHERE id = %s",json(cx,obj),guid);
	}
	debug("Executing query %s",qstr);
	int res = query(qstr);
	reset();
	if (res < 0) {
		*rval = EMPTY;
		return JS_TRUE;
	}
	*rval = str2jsval(id);
	return JS_TRUE;
}

static JSBool
LoadJSON(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	jsval retval;
	str qstr = Str("SELECT data FROM object WHERE id = %s",jsval2str(argv[0]));
	int res = query(qstr);
	if (res < 0) {
		reset();
		*rval = EMPTY;
		return JS_TRUE;
	}
	str jsn = Str("eval(%s)",fetch(0,0));
	reset();
	debug("Evaluating: %s",jsn);
	if (JS_FALSE == JS_EvaluateScript(ins.cx, ins.glob, jsn->data, jsn->len, "js.c", 1, &retval)) {
		error("Failed to load object %s",jsval2str(argv[0]));
		*rval = EMPTY;
		return JS_TRUE;
	} 
	*rval = retval;
	return JS_TRUE;
}

static JSBool
GetGuid(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	str id = guid();
	*rval = id ?  str2jsval(id) : EMPTY;
	return JS_TRUE;
}

static JSBool
Mail(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	if (argc != 4) {
		error("Usage: mail(to,from,subject,body)");
		*rval = FAILURE;
		return JS_TRUE;
	}
	str to = jsval2str(argv[0]);
	str from = jsval2str(argv[1]);
	str sub = jsval2str(argv[2]);
	str body = jsval2str(argv[3]);
	if (mail(to,from,sub,body)) {
		error("Failed to send message to %s",to);
		*rval = FAILURE;
		return JS_TRUE;
	}
	*rval = SUCCESS;
	return JS_TRUE;
}

static JSBool
RunScript(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	if (argc != 4) {
		error("Usage: run(host,script,when,data)");
		*rval = FAILURE;
		return JS_TRUE;
	}
	int i;
	str host = jsval2str(argv[0]);
	str script = file_path(host,Str("/%s",jsval2str(argv[1])));
	int when = str_int(jsval2str(argv[2]));
	JSObject* o = JSVAL_TO_OBJECT(argv[3]);	
	JSIdArray* arr = JS_Enumerate(ins.cx, o);
	Timers t = add_timer(script,when);
	for (i = 0; i < arr->length; ++i ) {
		char* prop = JS_GetStringBytes(ATOM_TO_STRING(JSID_TO_ATOM(arr->vector[i])));
		jsval value;
		JS_GetProperty(ins.cx,o,prop,&value);
		set_timer_value(t,Str("%c",prop),jsval2str(value));
	}
	*rval = SUCCESS;
	return JS_TRUE;
}
	
static JSBool
StopScript(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	if (argc != 1) {
		error("Usage: stop(timer_id)");
		*rval = FAILURE;
		return JS_TRUE;
	}
	str tid = jsval2str(argv[0]);
	debug("Cancelling timer recieved timer_id:  %s", tid);
	Timers t = str_obj(tid,Timers);
	debug("Cancelling timer %p",t);
	cancel_timer(t);
	*rval = SUCCESS;
	return JS_TRUE;
}
	
static JSBool
ListScripts(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	if (argc != 0) {
		error("Usage: running()");
		*rval = EMPTY;
		return JS_TRUE;
	}
	JSObject* arr =  JS_NewArrayObject(cx,0,NULL);
	int i = 0;
	Timers t;
	for (t = ins.srv->timers; t; t = t->next) 
		if (JS_DefineElement(cx,arr,i,str2jsval(Str("%p",t)),NULL,NULL,JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT))
			++i;
	*rval = OBJECT_TO_JSVAL(arr);
	return JS_TRUE;
}
	

static JSBool
S3Auth(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	if (argc != 2) {
		error("Usage: s3_auth(key,secret)");
		*rval = FAILURE;
		return JS_TRUE;
	}
	str key = jsval2str(argv[0]);
	str secret = jsval2str(argv[1]);
	s3_auth(key,secret);
	*rval = SUCCESS;
	return JS_TRUE;
}

static JSBool
S3PutJPEG(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	if (argc != 3) {
		error("Usage: s3_put_jpeg(file,bucket,filename)");
		*rval = FAILURE;
		return JS_TRUE;
	}
	
	str file = jsval2str(argv[0]);
	str bucket = jsval2str(argv[1]); 
	str filename = jsval2str(argv[2]);
	s3_put_jpeg(file,bucket,filename);
	*rval = SUCCESS;
	return JS_TRUE;
}

static JSBool
ResizeImage(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	if (argc != 3) {
		error("Usage: resize_image(filename,width,height)");
		*rval = EMPTY;
		return JS_TRUE;
	}
	str file = jsval2str(argv[0]);
	str width = jsval2str(argv[1]);
	str height = jsval2str(argv[2]);
	str retval = resize_image(file,width,height);
	*rval = str2jsval(retval);
	return JS_TRUE;
}

static JSBool
RotateImage(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	if (argc != 3) {
		error("Usage: resize_image(filename,width,height)");
		*rval = EMPTY;
		return JS_TRUE;
	}
	str file = jsval2str(argv[0]);
	str degrees = jsval2str(argv[1]);
	str color = jsval2str(argv[2]);
	str retval = rotate_image(file,degrees,color);
	*rval = str2jsval(retval);
	return JS_TRUE;
}

static JSBool
CropImage(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	if (argc != 3) {
		error("Usage: crop_image(filename,width,height)");
		*rval = EMPTY;
		return JS_TRUE;
	}
	str file = jsval2str(argv[0]);
	str width = jsval2str(argv[1]);
	str height = jsval2str(argv[2]);
	str retval = crop_image(file,width,height);
	*rval = str2jsval(retval);
	return JS_TRUE;
}

static JSBool
ImageInfo(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	int i;
	str value;
	JSObject* o = JS_NewObject(cx,NULL,NULL,NULL);
	if (argc != 1) {
		error("Usage: image_info(filename)");
		*rval = EMPTY;
		return JS_TRUE;	
	}
	str filename = jsval2str(argv[0]);
	char** props = get_image_properties(filename->data);
	if (! props) {
		error("image_info failed to read image properties");
		*rval = EMPTY;
		return JS_TRUE;;
	}
	for (i = 0; props[i*2]; ++i) {
		if (props[i*2+1])
			value = char_str(props[i*2+1],strlen(props[i*2+1]));
		else
			value = NULL;
		if (value) JS_DefineProperty(cx,o,props[i*2]+5,str2jsval(value),NULL,NULL, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
	}
	*rval = OBJECT_TO_JSVAL(o);
	return JS_TRUE;
}

static JSBool
FacebookAuth(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	if (argc != 2) {
		error("Usage: facebook_auth(key,secret)");
		*rval = FAILURE;
		return JS_TRUE;
	}
	str key = jsval2str(argv[0]);
	str secret = jsval2str(argv[1]);
	facebook_auth(key,secret);
	*rval = SUCCESS;
	return JS_TRUE;
}

static JSBool
FacebookLogin(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	if (argc != 0) {
		error("Usage: facebook_login()");
		*rval = EMPTY;
		return JS_TRUE;
	}
	*rval = str2jsval(facebook_login());
	return JS_TRUE;
}

static JSBool
FacebookMethod(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	int i;
	Headers kv = new_headers();	
	if (argc < 3) {
		*rval = FAILURE;
		error("Usage: facebook_method(method,...)");
		return JS_TRUE;
	}
	str method = jsval2str(argv[0]);
	for (i = 1; i < argc; i += 2)
		kv = append_header(kv,jsval2str(argv[i]),jsval2str(argv[i+1]));
	str res = facebook_method(method,kv);
	*rval = str2jsval(res);
	return JS_TRUE;
}

static JSBool
SMSInit(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	if (argc != 0) {
		error("Usage: sms_init()");
		*rval = FAILURE;
		return JS_TRUE;
	}
	
	*rval = sms_process_stack() ? FAILURE : SUCCESS;
	return JS_TRUE;
}

static JSBool
SMSReadACK(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	if (argc != 0) {
		error("Usage: sms_read_ack()");
		*rval = FAILURE;
		return JS_TRUE;
	}
	*rval = sms_process_line() ? FAILURE : SUCCESS;
	return JS_TRUE;
}

static JSBool
SendSMS(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	if (argc != 2) {
		error("Usage: sms_write_msg(number,txt)");
		*rval = FAILURE;
		return JS_TRUE;
	}
	str number = jsval2str(argv[0]);
	str text  = jsval2str(argv[1]);
	*rval = sms_write_msg(number,text) ? FAILURE : SUCCESS;
	return JS_TRUE;
}

static JSBool
PostHTTP(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	if (argc != 4) {
		error("Usage: post(host,path,headers,data)");
		*rval = EMPTY;
		return JS_TRUE;
	}
	int i;
	str host = jsval2str(argv[0]);
	str path = jsval2str(argv[1]);
	Post p = new_post(host,path);
	str body = jsval2str(argv[3]);
	p = set_post_data(p,body);
	JSObject* o = JSVAL_TO_OBJECT(argv[2]);	
	JSIdArray* arr = JS_Enumerate(ins.cx, o);
	for (i = 0; i < arr->length; ++i ) {
		char* prop = JS_GetStringBytes(ATOM_TO_STRING(JSID_TO_ATOM(arr->vector[i])));
		jsval value;
		JS_GetProperty(ins.cx,o,prop,&value);
		set_post_header(p,Str("%c",prop),jsval2str(value));
	}
	if (post(p)) {
		error("Failed to post to http://%s%s",host,path);
		*rval = EMPTY;
		return JS_TRUE;
	}
	str resp = post_response(p);
	*rval = str2jsval(resp);
	return JS_TRUE;	
}

static JSClass global_class = {
	"global", 0,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};

static JSFunctionSpec obj_functions[] = {
	{"json", JSON, 0 },
	{"store", StoreJSON, 0 },
	{"load", LoadJSON, 0 },
	{0},
};

static JSFunctionSpec glob_functions[] = {
	{"print", Print, 0},
	{"debug", Debug, 0},
	{"error", Error, 0},
	{"exit", ExitJS, 0},
	{"include", Include, 0},
	{"use", Use, 0},
	{"cwd", CWD, 0},
	{"header", Header, 0 },
	{"param", Param, 0 },
	{"now", Now, 0 },
	{"query", Query, 0 },
	{"encode", Encode, 0 },
	{"decode", Decode, 0 },
	{"status", Status, 0},
	{"location", Location, 0},
	{"expires", Expires, 0},
	{"content_type", ContentType, 0},
	{"cache_control", CacheControl, 0},
	{"connection", Connection, 0},
	{"hit_info", HitInfo, 0},
	{"client_info", ClientInfo, 0},
	{"mem_info", MemInfo, 0},
	{"file_info", FileInfo, 0},
	{"request_info", RequestInfoTable, 0},
	{"guid", GetGuid, 0 },
	{"mail", Mail, 0 },
	{"run", RunScript, 0 },
	{"stop", StopScript, 0 },
	{"running", ListScripts, 0 },
	{"s3_auth", S3Auth, 0 }, 
	{"s3_put_jpeg",S3PutJPEG, 0},
	{"image_info",ImageInfo, 0},
	{"resize_image",ResizeImage, 0},
	{"rotate_image",RotateImage, 0},
	{"crop_image",CropImage, 0},
	{"facebook_auth",FacebookAuth, 0},
	{"facebook_login",FacebookLogin, 0},
	{"facebook_method",FacebookMethod, 0},
	{"sms_init", SMSInit, 0},
	{"sms_read_ack", SMSReadACK, 0},
	{"send_sms", SendSMS, 0},
	{"post", PostHTTP, 0},
	{0},
};

static int
CreateDatabaseTableFunctions(JSInstance* in)
{
	int i;
	const char* args[] = { "id","obj",NULL };
	str q = Str("SELECT tablename FROM pg_tables WHERE schemaname = 'public'");
	int res = query(q);
	if (res < 0) {
		error("Failed to initialize database interface");
		return 1;
	}
	for (i = 0; i < res; ++i) {
		str table = fetch(i,0);
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
		if (NULL == JS_CompileFunction(in->cx,in->glob,table->data,2,args,s->data,s->len,"jws.c",0)) 
			debug("Failed to compile script %s",s);
	}
	reset();
	return 0;
}

int
InitParams(JSInstance* in, Headers headers)
{
	str x;
	JSContext* cx = in->cx;
	JSObject* o;
	JSObject* arr = NULL;
	int i,j, n;
	if (! headers) return 0;
	for (i = 0; i < MAX_HEADERS && headers[i].key; ++i) {
		if (headers[i].key == (str)-1) continue;
		x = Str("$%s",headers[i].key);
//		debug("Initializing %s = %s",x,headers[i].value);
		n = 0;
		arr = NULL;
		if (JS_FALSE == JS_DefineProperty(in->cx,in->glob,x->data,str2jsval(headers[i].value),NULL, NULL,JSPROP_READONLY))
			debug("Failed to set property %s",x);
		for (j = i+1; j < MAX_HEADERS && headers[j].key; ++j) {
			if (headers[j].key == (str)-1) continue;
			if (cmp_str(headers[i].key,headers[j].key)) {
				if (! arr) {
					arr =  JS_NewArrayObject(in->cx,0,NULL);
					JS_DefineElement(in->cx,arr,n,str2jsval(headers[i].value),NULL,NULL,JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
					debug("Adding %s to array %s",headers[i].value,x);
				}
				debug("Adding %s to array %s",headers[j].value,x);
				JS_DefineElement(in->cx,arr,++n,str2jsval(headers[j].value),NULL,NULL,JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
				headers[j].key = (str)-1;
			}
		}
		if (arr && JS_FALSE == JS_DefineProperty(in->cx,in->glob,x->data,OBJECT_TO_JSVAL(arr),NULL, NULL,JSPROP_READONLY))
			debug("Failed to set property %s",x);
	}
        return 0;
}

int
InitJS(JSInstance* i, Server srv, Headers data)
{
	i->srv = srv;
	i->resp = srv->resp;
	i->buffer = NULL;
	i->rt = JS_NewRuntime(RUNTIME_SIZE);
	if (!i->rt) return 1;
	i->cx = JS_NewContext(i->rt, CONTEXT_SIZE);
	if (!i->cx) return 1;
	i->glob = JS_NewObject(i->cx, &global_class, NULL, NULL);
	i->builtins = JS_InitStandardClasses(i->cx, i->glob);
	if (!JS_DefineFunctions(i->cx, i->glob, glob_functions)) return 1;
	i->obj_proto = OBJ_GET_PROTO(i->cx, i->glob);
	if (!JS_DefineFunctions(i->cx, i->obj_proto, obj_functions)) return 1; 
	i->database = new_database();
	if (!i->database) return 1;
	InitParams(i, data ? data : i->resp->req->query_vars);
//	return CreateDatabaseTableFunctions(i);
	return 0;
}

int 
DestroyJS(JSInstance* i)
{
	close_database(i->database);
	JS_DestroyContext(i->cx);
	JS_DestroyRuntime(i->rt);
	JS_ShutDown();
	return 0;
}

void
ProcessFile(char* script)
{
	cstr scratch;
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
				if (len > MAX_ALLOC_SIZE) {
					error("Expression to large!");
				
				} else {
					scratch = Cstr(&script[o+6],len);	
					str tmp = Str("print(%x);",scratch);
					scratch = Cstr(tmp->data,tmp->len);
				}
				o += len + 6;
				len = o+3;
			} else {
				while (strncmp(&script[o+len+5],"?>",2)) ++len;
				scratch = Cstr(&script[o+5],len-1);
				o += len + 5;
				len = o+2;
			}
			if (JS_FALSE == JS_EvaluateScript(ins.cx, ins.glob, scratch->data, scratch->len, "js.c", 1, &retval))
				error("Failed to evaluate [%x]", scratch);
		}
	}
	ins.buffer = write_buffer(ins.buffer,&script[len],o-len);
}

int
run_script(File fc, Headers data)
{
	jsval rval;
	if (InitJS(&ins,srv,data)) {
		error("Failed to initialize Javascript");
		return 1;
	}
	if (!setjmp(jmp)) {
		if (Resp)
			ProcessFile(fc->data);
		else 
			if (JS_FALSE == JS_EvaluateScript(ins.cx, ins.glob, fc->data, fc->st.st_size, "js.c", 1, &rval)) 
				debug("Failed to evaluate script %c",fc->name);
	}
	if (DestroyJS(&ins)) {
		error("Failed to destroy Javascript");
		return 1;
	}
	return 0;
}

int
jws_handler(File fc)
{
	Buffer tmp;
	if (run_script(fc,NULL)) goto error;
	if (Resp) Resp->contents = ins.buffer;
	return Resp ? Resp->status : 0;
error:
	for (;ins.buffer; ins.buffer = free_buffer(ins.buffer));
	if (Resp) Resp->contents = NULL;
	return 500;
}
