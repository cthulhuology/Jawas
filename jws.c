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
#include "str.h"
#include "headers.h"
#include "log.h"
#include "uri.h"
#include "requests.h"
#include "responses.h"
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

JSInstance ins;
jmp_buf jmp;

void ProcessFile(char* script);

str
js2str(JSContext* cx, jsval x)
{
	return copy(JS_GetStringBytes(JS_ValueToString(cx,x)),JS_GetStringLength(JS_ValueToString(cx,x)));
}

jsval
str2js(JSContext* cx, str x)
{
	if (! x) return EMPTY;
	char* t = dump(x);
 	return STRING_TO_JSVAL(JS_NewStringCopyN(cx,t,len(x)));
}

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
	str s;

	for (i = 0; i < argc; ++i) {
		s = jsval2str(argv[i]);
		if (!s) {
			error("print() failed, invalid string parameter at %i",i);
			*rval = FAILURE;
			return JS_TRUE;
		}
		//debug("Printing [%s]",s);
		ins.buffer = append(ins.buffer,s);
	}
	//debug("Current buffer [%s]",ins.buffer);
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
	str s;
	str filename;
	if (argc != 1) {
		error("Usage: include(filename);");
		*rval = FAILURE;
		return JS_TRUE;
	}
	s = jsval2str(argv[0]);
	filename = file_path(Req->host,Str("/%s",s));
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
	str s, filename;
	if (argc != 1) {
		error("Usage: use(filename);");
		*rval = FAILURE;
		return JS_TRUE;
	}
	s = jsval2str(argv[0]);
	filename = file_path(Req->host,Str("/%s",s));
	fc = load(filename);
	if (!fc) {
		error("Failed to open file %s",filename);
		*rval = FAILURE;
		return JS_TRUE;
	}
	if (JS_FALSE == JS_EvaluateScript(ins.cx, ins.glob, fc->data, fc->st.st_size, "jws.c", 1, rval)) {
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
	str s;
	if (argc != 1) {
		error("Usage: header(fieldname)");
		*rval = EMPTY;
		return JS_TRUE;	
	}
	s = jsval2str(argv[0]);
	str head  = find_header(ins.resp->req->headers,s);
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
	str s;
	if (argc != 1) {
		error("Usage: param(variable)");
		*rval = EMPTY;
		return JS_TRUE;
	}
	s = jsval2str(argv[0]);
	str pram = find_header(ins.resp->req->query_vars,s);
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
	*rval = str2jsval(Str("%i",time(NULL)));
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
		for (i = 1; i < len(qstr) && j < argc; ++i)
			if (at(qstr,i) == '?') {
				++j;
				str val = jsval2str(argv[j]);
				debug("Value [%i] is %s",j,val);
				val = singlequote(val);
				debug("Single Quoted Value is %s",val);
				qry = append(qry,append(from(qstr,o,i),val));
				debug("Qry is %s",qry);
				o = i + 1;
			}
	qry = append(qry,from(qstr,o,len(qstr)));
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
			char* param = dump(field(j));
			if (!JS_DefineProperty(cx,row,param,str2jsval(value),NULL,NULL, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT)) {
				error("Failed to apply column %c to row %i\n",field(j),i);
				free(param);
				continue;
			}
			free(param);
		}
	}
	reset();
	*rval = OBJECT_TO_JSVAL(arr);
	return JS_TRUE;
}

static JSBool
Encode(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	str s;
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
	str s;
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
	ins.buffer = append(ins.buffer,Str("<table>"));
	ins.buffer = append(ins.buffer,Str("<tr><td>Max Clients:</td><td> %i</td></tr>",gsci.max));
	ins.buffer = append(ins.buffer,Str("<tr><td>Current Clients:</td><td> %i</td></tr>",gsci.current));
	ins.buffer = append(ins.buffer,Str("</table><hr /><ol>"));
	for (sc = ins.srv->sc; sc; sc = sc->next) 
		ins.buffer = append(ins.buffer,Str("<li>%i.%i.%i.%i:%i</li>",
			(0xff & sc->peer),
			(0xff00 & sc->peer) >> 8,
			(0xff0000 & sc->peer) >> 16,
			(0xff000000 & sc->peer) >> 24,
			sc->port));
	ins.buffer = append(ins.buffer,Str("</ol>"));
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
		
	ins.buffer = append(ins.buffer,Str("<table>"));
	ins.buffer = append(ins.buffer,Str("<tr><td>Base Address:</td><td> %p</td></tr>",gpi.baseaddr));
	ins.buffer = append(ins.buffer,Str("<tr><td>Free Memory:</td><td> %p</td></tr>",free_memory()));
	ins.buffer = append(ins.buffer,Str("<tr><td>Allocated Memory:</td><td> %p</td></tr>",alloced_memory()));
	ins.buffer = append(ins.buffer,Str("<tr><td>Pages Allocated:</td><td> %i</td></tr>",gpi.allocated));
	ins.buffer = append(ins.buffer,Str("<tr><td>Scratch Pages Allocated:</td><td> %i</td></tr>",gsi.allocated));
	ins.buffer = append(ins.buffer,Str("<tr><td>Freed Pages:</td><td> %i</td></tr>",gpi.frees));
	ins.buffer = append(ins.buffer,Str("<tr><td>Freed Scratch Pages:</td><td> %i</td></tr>",gsi.frees));
	ins.buffer = append(ins.buffer,Str("</table>"));
	return JS_TRUE;
}

static JSBool
FileInfo(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	File fc;
	int total = 0;
	ins.buffer = append(ins.buffer,Str("<table><tr><th>Name</th><th>Hits</th><th>Size</th></tr>"));
	for (fc = ins.srv->fc; fc; fc = fc->next)  {
		ins.buffer = append(ins.buffer,Str("<tr><td>%c</td><td>%i</td><td>%i</td></tr>",&fc->name[len(cwd)],fc->count,fc->st.st_size));
		total += fc->st.st_size;
	}
	ins.buffer = append(ins.buffer,Str("<tr><td colspan=2>Total:</td><td>%i</td></tr></table>",total));
	return JS_TRUE;
}

static JSBool
RequestInfoTable(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	RequestInfo ri;
	ins.buffer = append(ins.buffer,Str("<table><tr><th>Request</th><th># Hits</th><th>Avg Time</th><th>PPM</th></tr>"));
	for (ri  = ins.srv->ri; ri; ri = ri->next) {
		ins.buffer = append(ins.buffer, Str("<tr><td>http://%s%s</td><td>%i</td><td>%i&mu;s</td><td>%i</td></tr>",ri->host,ri->path,ri->hits,ri->time, 60000000 / ri->time));
	}
	ins.buffer = append(ins.buffer,Str("</table>"));
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
	char* data = dump(jsn);
	if (JS_FALSE == JS_EvaluateScript(ins.cx, ins.glob, data, len(jsn), "jws.c", 1, &retval)) {
		error("Failed to load object %s",jsval2str(argv[0]));
		*rval = EMPTY;
		free(data);
		return JS_TRUE;
	} 
	free(data);
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
	debug("Run Script from now %i at %i ",when, when + time(NULL));
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
S3Put(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	if (argc != 4) {
		error("Usage: s3_put(file,bucket,filename,mime)");
		*rval = FAILURE;
		return JS_TRUE;
	}
	
	str file = jsval2str(argv[0]);
	str bucket = jsval2str(argv[1]); 
	str filename = jsval2str(argv[2]);
	str mime = jsval2str(argv[3]);
	s3_put(file,bucket,filename,mime);
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
	char* fname = dump(filename);
	char** props = get_image_properties(fname);
	free(fname);
	if (! props) {
		error("image_info failed to read image properties");
		*rval = EMPTY;
		return JS_TRUE;;
	}
	for (i = 0; props[i*2]; ++i) {
		if (props[i*2+1])
			value = copy(props[i*2+1],strlen(props[i*2+1]));
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
	if (argc != 5) {
		error("Usage: post(host,path,headers,data,callback)");
		*rval = EMPTY;
		return JS_TRUE;
	}
	int i;
	str host = jsval2str(argv[0]);
	str path = jsval2str(argv[1]);
	debug("Posting to %s/%s",host,path);
	Request req = new_request(Str("POST"),host,path);
	
	JSObject* o = JSVAL_TO_OBJECT(argv[2]);	
	JSIdArray* arr = JS_Enumerate(ins.cx, o);
	for (i = 0; i < arr->length; ++i ) {
		char* prop = JS_GetStringBytes(ATOM_TO_STRING(JSID_TO_ATOM(arr->vector[i])));
		jsval value;
		JS_GetProperty(ins.cx,o,prop,&value);
		request_headers(req,Str("%c",prop),jsval2str(value));
	}
	str body = jsval2str(argv[3]);
	req = request_data(req,body);
	str callback = jsval2str(argv[4]);
	request_callback(req,ins.resp,callback);
	send_request(req);
	ins.resp->status = 0;
	longjmp(jmp,1);
	return JS_TRUE; // never get here
}

static JSClass global_class = {
	"global", 0,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};

static JSFunctionSpec obj_functions[] = {
	{"json", JSON, 0 },
	{"store", StoreJSON, 0 },
	{"save", SaveJSON, 0 },
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
	{"s3_put",S3Put, 0},
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

int
InitParams(JSInstance* in, Headers headers)
{
	str x;
	JSContext* cx = in->cx;
	JSObject* arr = NULL;
	int i,j, n;
	if (! headers) return 0;
	over(headers,i) {
		skip_null(headers,i);
		x = Str("$%s",Key(headers,i));
		char* param = dump(x);
		debug("Setting %s = [%s]",x, Value(headers,i));
		n = 0;
		arr = NULL;
		if (JS_FALSE == JS_DefineProperty(in->cx,in->glob,param,str2jsval(Value(headers,i)),NULL, NULL,JSPROP_READONLY))
			debug("Failed to set property %s",x);
		overs(headers,j,i+1) {
			skip_null(headers,j);
			if (cmp(Key(headers,i),Key(headers,j))) {
				if (! arr) {
					arr =  JS_NewArrayObject(in->cx,0,NULL);
					JS_DefineElement(in->cx,arr,n,str2jsval(Value(headers,i)),NULL,NULL,JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
				}
				JS_DefineElement(in->cx,arr,++n,str2jsval(Value(headers,j)),NULL,NULL,JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
				headers->slots[j].key = NULL;
			}
		}
		if (arr && JS_FALSE == JS_DefineProperty(in->cx,in->glob,param,OBJECT_TO_JSVAL(arr),NULL, NULL,JSPROP_READONLY))
			debug("Failed to set property %s",x);
	}
        return 0;
}

int
InitScripts(JSInstance* in)
{
	int i;
	jsval retval;
	if (in && in->resp && in->resp->req && in->resp->req->host) {
		int rows = query(Str("SELECT func FROM scripts WHERE site = '%s' and active",in->resp->req->host));
		for (i = 0; i < rows; ++i) {
			str script = fetch(i,0);
			char* script_data = dump(script);
			if (JS_FALSE == JS_EvaluateScript(in->cx, in->glob, script_data, len(script), "jws.c", 1, &retval)) {
				error("Failed to compile script:");
				error("%s",script);
			}
			free(script_data);
		}	
		reset();
	}
	return 0;
}

int
InitRequest(JSInstance* in, Request req)
{
	int i;
	JSContext* cx = in->cx;
	JSObject* rq = JS_DefineObject(in->cx,in->glob,"request",NULL,NULL,JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
	over(req->headers,i) {
		char* k = dump(Key(req->headers,i));
		if (!JS_DefineProperty(in->cx,rq,k,str2jsval(Value(req->headers,i)),NULL,NULL, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT)) {
				free(k);
				error("Failed to assign property %s to request", Key(req->headers,i));
				continue;
		}
		free(k);
	}
	return 0;
}

int
InitResponse(JSInstance* in, Response resp)
{
	int i;
	JSContext* cx = in->cx;
	JSObject* rsp = JS_DefineObject(in->cx,in->glob,"response",NULL,NULL,JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
	over(resp->headers,i) {
		char* k = dump(Key(resp->headers,i));
		if (!JS_DefineProperty(in->cx,rsp,k,str2jsval(Value(resp->headers,i)),NULL,NULL, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT)) {
				free(k);
				error("Failed to assign property %s to response", Key(resp->headers,i));
				continue;
		}
		free(k);
	}
	return 0;
}

int
InitJS(JSInstance* i, Server srv, Headers data)
{
	i->srv = srv;
	i->resp = srv->resp;
	i->buffer = (srv->resp ? srv->resp->contents : NULL);
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
//	if (i->resp && i->resp->req) InitRequest(i,i->resp->req);
//	if (i->resp) InitResponse(i,i->resp);
	InitScripts(i);
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
	jsval retval;
	int o, l = 0;
	str script_data;
	for (o = 0; script[o]; ++o) {
		if (!strncmp(&script[o],"<?js",4)) {
			if (l < o)
				ins.buffer = append(ins.buffer,copy(&script[l],o-l));
			l = 0;
			if (script[o+4] == '=') {
				while (strncmp(&script[o+l+6],"=?>",3)) ++l;
				if (l > MAX_ALLOC_SIZE) {
					error("Expression to large!");
				
				} else {
					script_data = Str("print(%s);",copy(&script[o+6],l));	
				}
				o += l + 6;
				l = o+3;
			} else {
				while (strncmp(&script[o+l+5],"?>",2)) ++l;
				script_data = copy(&script[o+5],l-1);
				o += l + 5;
				l = o+2;
			}
			char* eval_script = dump(script_data);
			if (JS_FALSE == JS_EvaluateScript(ins.cx, ins.glob, eval_script, len(script_data), "jws.c", 1, &retval))
				error("Failed to evaluate [%s]", script_data);
		}
	}
	ins.buffer = append(ins.buffer,copy(&script[l],o-l));
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
			if (JS_FALSE == JS_EvaluateScript(ins.cx, ins.glob, fc->data, fc->st.st_size, "jws.c", 1, &rval)) 
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
	if (run_script(fc,NULL)) goto error;
	if (Resp) Resp->contents = ins.buffer;
	return Resp ? Resp->status : 0;
error:
	if (Resp) Resp->contents = NULL;
	return 500;
}

int
process_callback(str cb, Headers headers)
{
	jsval rval;
	if (InitJS(&ins,srv,headers)) {
		error("Failed to initialize Javascript");
		return 1;
	}
	Resp->status = 200;
	if (!setjmp(jmp)) {
		char* cb_data = dump(cb);
		debug("Evaluting callback %c",cb_data);
		if (JS_FALSE == JS_EvaluateScript(ins.cx, ins.glob, cb_data, len(cb), "jws.c", 1, &rval)) 
				debug("Failed to evaluate script %s",Req->cb);
		free(cb_data);
	}
	if (DestroyJS(&ins)) {
		error("Failed to destroy Javascript");
		return 1;
	}
	// debug("Response data [%s]", Resp->contents);
	// debug("Response headers [%s]", print_headers(NULL,Resp->headers));
	return 0;
}
