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

#include "buffers.h"

#include "jsapi.h"

// Javascript Functions

Buffer gbuffer;

static JSBool
Print(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
	uintN i;
	int pos;
	Buffer buffer = new_buffer(gbuffer);
	JSString *str;

	pos = 0;
	for (i = 0; pos < (buffer->length - 2) && i < argc; ++i) {
		str = JS_ValueToString(cx, argv[i]);
		if (!str) return JS_FALSE;
		pos += sprintf(&buffer->data[pos],"%s%s", (i ? " " : ""), JS_GetStringBytes(str));
	}
	pos += sprintf(&buffer->data[pos],"\n");
	buffer->length = pos;
	gbuffer = buffer;
	return JS_TRUE;
}

#define RUNTIME_SIZE 8L * 1024L * 1024L
#define CONTEXT_SIZE 8192

typedef struct js_instance_struct JSInstance;
struct js_instance_struct { 
	JSRuntime      *rt;
	JSContext      *cx;
	JSObject       *glob;
	JSBool          builtins;
	int		fd;	
	char*		script;
};

JSClass global_class = {
	"global", 0,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};

static JSFunctionSpec my_functions[] = {
	{"print", Print, 0},
	{0},
};

int
InitJS(JSInstance* ins)
{
	ins->rt = JS_NewRuntime(RUNTIME_SIZE);
	if (!ins->rt) return 1;
	ins->cx = JS_NewContext(ins->rt, CONTEXT_SIZE);
	if (!ins->cx) return 1;
	ins->glob = JS_NewObject(ins->cx, &global_class, NULL, NULL);
	ins->builtins = JS_InitStandardClasses(ins->cx, ins->glob);
	if (!JS_DefineFunctions(ins->cx, ins->glob, my_functions)) return 1;
	return 0;
}

int 
DestroyJS(JSInstance* ins)
{
	JS_DestroyContext(ins->cx);
	JS_DestroyRuntime(ins->rt);
	JS_ShutDown();
	return 0;
}

int
OpenFile(char* filename, int* fd, char** script) 
{
	struct stat	st;
	if (!filename || !*filename) return 1;
	if (0 > (*fd = open(filename,O_RDONLY,0400))) return 2;
	if (fstat(*fd,&st)) return 3;
	if (! (*script = mmap(NULL,st.st_size,PROT_READ,MAP_SHARED|MAP_FILE,*fd,0))) return 3;
	return 0;
}

int
CloseFile(int fd, char* script) 
{
	struct stat	st;
	if (fstat(fd,&st)) return 1;
	if (munmap(script,st.st_size)) return 2;
	if (close(fd)) return 3;
	return 0;
}

int
ProcessFile(JSInstance* ins, char* script,request_rec* req)
{
	JSBool ok;
	jsval retval;
	int tmp;
	int len = 0;
	gtotal = 0;
	for (tmp = 0; script[tmp]; ++tmp) {
		if (!strncmp(&script[tmp],"<?js",4)) {
			if (len < tmp) gtotal += ap_rwrite(1,&script[len],tmp-len,req);
			len = 0;
			while (strncmp(&script[tmp+len+5],"?>",2)) ++len;
			ok = JS_EvaluateScript(ins->cx, ins->glob, &script[tmp + 5], len-1 , "test.c", 1, &retval);
			tmp += len + 5;
			len = tmp+2;
		}
	}
	gtotal += ap_rwrite(1,&script[len],tmp-len,req);
	return 0;
}

static int
jawas_handler(request_rec *req)
{
	JSInstance	ins;
	if (InitJS(&ins)) return 1;
	if (OpenFile(req->filename,&ins.fd,&ins.script)) return 1;
	if (ProcessFile(&ins,ins.script,req)) return 1;
	if (CloseFile(ins.fd,ins.script)) return 1;
	if (DestroyJS(&ins)) return 1;
	return OK;
}

