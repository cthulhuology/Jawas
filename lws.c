// lws.c
//
// LUA web bindings for JAWAS
//
// © 2009 David J. Goehrig
//

#include "include.h"
#include "defines.h"
#include "config.h"
#include "memory.h"
#include "str.h"
#include "headers.h"
#include "log.h"
#include "uri.h"
#include "requests.h"
#include "responses.h"
#include "files.h"
#include "server.h"
#include "timers.h"
#include "database.h"
#include "usage.h"
#include "auth.h"
#include "lua_json.h"
#include "dates.h"
#include "status.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "lua_db.h"

lua_State* lins;
Database lins_database;
jmp_buf l_jump;

str
lua2str(int index)
{
	str retval = NULL;
	if (lua_isstring(lins,index))
		return ref(lua_tostring(lins,index),lua_strlen(lins,index));
	if (lua_isnumber(lins,index)) {
		lua_pushvalue(lins,index);
		retval = ref(lua_tostring(lins,-1),lua_strlen(lins,-1));
		lua_pop(lins,1);
		return retval;
	}
	if (lua_isboolean(lins,index)) {
		return lua_toboolean(lins,index) ? $("true") : $("false");
	}
	return $("%p",lua_topointer(lins,index));
}

void
str2lua(str s)
{
	s ? lua_pushlstring(lins,s->data,len(s)) : lua_pushnil(lins);
}

void
EvalLuaFile(File fc)
{
	if (!fc->parsed) parse_file(fc);
	for (int i = 0; fc->parsed[i].kind; ++i) {
		switch(fc->parsed[i].kind) {
		case TEXT:
			write_chunk(Req->sc,&fc->data[fc->parsed[i].offset],fc->parsed[i].length);
			break;
		case SCRIPT:
		case EMIT:
			if (luaL_loadbuffer(lins,&fc->data[fc->parsed[i].offset], fc->parsed[i].length, fc->name) || lua_pcall(lins,0,LUA_MULTRET,0)) error("Failed to evaluate [%s]",copy(&fc->data[fc->parsed[i].offset],fc->parsed[i].length));
		}
	}
}

static int ExitLua(lua_State* L)
{
	longjmp(l_jump,1);
	return 0;
}

static int PrintLua(lua_State* l)
{
	int n = lua_gettop(l);
	for (int i = 1; i <= n; ++i)
		write_chunked_socket(Req->sc,lua2str(i));
	lua_pop(l,n);
	return 0;
}

static int DebugLua(lua_State* l)
{
	int n = lua_gettop(l);
	for (int i = 1; i <= n; ++i)
		debug("%s",lua2str(i));
	lua_pop(l,n);
	return 0;
}

static int ErrorLua(lua_State* l)
{
	int n = lua_gettop(l);
	for (int i = 1; i <= n; ++i)
		error("%s",lua2str(i));
	lua_pop(l,n);
	return 0;
}

static int UseLua(lua_State* l)
{
	str filename = file_path(Req ? Req->host : $("localhost"), $("/%s",lua2str(1)));
	if (luaL_dofile(l,filename->data)) error("Failed to evaluate script %s",filename);
	lua_pop(l,1);
	return 0;
}

static int IncludeLua(lua_State* l)
{
	str filename = file_path(Req ? Req->host : $("localhost"), $("/%s",lua2str(1)));
	lua_pop(l,1);
	File fc = load(filename);
	if (!fc) {
		error("Failed to open file %s",filename);
		return 0;
	}
	EvalLuaFile(fc);
	return 0;
}

static int CWDLua(lua_State* l)
{
	int n = lua_gettop(l);
	lua_pop(l,n);
	str2lua(Req->path);
	return 1;
}

static int HeaderLua(lua_State* l)
{
	int n = lua_gettop(l);
	str s = lua2str(1);
	lua_pop(l,n);
	str2lua(find_header(Req->headers,s));
	return 1;
}

static int ParamLua(lua_State* l)
{
	int n = lua_gettop(l);
	str s = lua2str(1);
	lua_pop(l,n);
	str2lua(find_header(Req->query_vars,s));
	return 1;
}

static int NowLua(lua_State* l)
{
	int n = lua_gettop(l);
	lua_pop(l,n);
	str2lua($("%i",time(NULL)));
	return 1;
}

static int QueryLua(lua_State* l)
{
	int n = lua_gettop(l);
	str qstr = lua2str(1);
	str qry = NULL;
	int i = 0, j = 1, o = 0;
	for (i = 1; i < len(qstr) && j <= n; ++i)
		if (at(qstr,i) == '?') {
			str val = lua2str(++j);
			val = singlequote(val);
			qry = append(qry,append(from(qstr,o,i-o),val));
			o = i+1;
		}
	lua_pop(l,n);
	qry = append(qry,from(qstr,o,len(qstr)-o));
	int res = query(qry);
	if (res < 0) {
		str2lua(db_error());
		return 1;
	}
	lua_newtable(l);
	n = lua_gettop(l);
	for (i = 0; i < res; ++i) {
		lua_pushnumber(l,i+1);
		lua_newtable(l);
		for (j = 0; j < fields(); ++j) {
			str2lua(field(j));
			str2lua(fetch(i,j));
			lua_settable(l,n+2);
		}
		lua_settable(l,n);
	}
	return 1;
}

static int EncodeLua(lua_State* l)
{
	int n = lua_gettop(l);
	str s = lua2str(1);
	lua_pop(l,n);
	str2lua(uri_encode(s));
	return 1;
}

static int DecodeLua(lua_State* l)
{
	int n = lua_gettop(l);
	str s = lua2str(1);
	lua_pop(l,n);
	str2lua(uri_decode(s));
	return 1;
}

static int MD5SumLua(lua_State* l)
{
	int n = lua_gettop(l);
	str s = lua2str(1);
	lua_pop(l,n);
	str2lua(md5sum(s->data,s->length));
	return 1;
}

static int Hmac1Lua(lua_State* l)
{
	int n = lua_gettop(l);
	str secret = lua2str(1);
	str data = lua2str(2);
	lua_pop(l,n);
	str retval = hmac1(secret,data);
	fprintf(stderr,"Luahmac len %d\n",len(retval));
	str2lua(retval);
	return 1;
}

static int Base64Lua(lua_State* l)
{

	int n = lua_gettop(l);
	str s = lua2str(1);
	lua_pop(l,n);
	str2lua(base64(s));
	return 1;
}

static int HexLua(lua_State* l)
{

	int n = lua_gettop(l);
	str s = lua2str(1);
	lua_pop(l,n);
	str2lua(hex(s));
	return 1;
}

int luarandfd = 0;
static int RandomLua(lua_State* l)
{
	int n = lua_gettop(l);
	lua_pop(l,n);
	if (!luarandfd) luarandfd = open("/dev/random",O_RDONLY);
	str data = blank(12);
	read(luarandfd,data->data,12);
	str o = hex(data);
	debug("ranom number is [%s] length is %i",o,o->length);
	str2lua(o);
	return 1;
}

static int PostHTTPLua(lua_State* l)
{
	int n = lua_gettop(l);
	str proto = lua2str(1);
	str host = lua2str(2);
	str path = lua2str(3);
	// n-3 is the headers table
	str data = lua2str(5);
	str callback = lua2str(6);
	Request req = new_request($("POST"),host,path);
	if (icmp($("https"),proto)) use_ssl(req);
	lua_pushnil(l);
	while(lua_next(l,4)) {
		request_headers(req,lua2str(-2),lua2str(-1));
		lua_pop(l,1);
	}
	request_data(req,data);
	request_callback(req,Resp,callback);
	send_request(req);
	if (Resp) Resp->status = 0;
	lua_pop(l,n+1);
	longjmp(l_jump,1);
	return 0;
}

static int SocketLua(lua_State* l)
{
	int n = lua_gettop(l);
	str host = lua2str(1);
	str port = lua2str(2);
	debug("looking up %s:%i",host,str_int(port));
	IPAddress* ipaddrs = lookup(host);
	debug("Socket for ipaddr %s", ipaddress(*ipaddrs,str_int(port)));
	Socket s = create_socket(new_socket(0),NULL,NULL);
	s->host = host;
	socket_attach(s,ipaddrs[0],str_int(port));
	debug("Created socket for %s",socket_peer(s));
	lua_pop(l,n);
	lua_pushlightuserdata(l,s);
	return 1;
}

static int SendLua(lua_State* l)
{
	int n = lua_gettop(l);
	Socket s = (Socket)lua_topointer(l,1);
	str buf = lua2str(2);
	debug("Sending [%s] to %s",buf, socket_peer(s));
	int len = socket_send(s,buf);
	debug("Sent %i bytes",len);
	lua_pop(l,n);
	lua_pushnumber(l,len);
	return 1;
}

static int ReceiveLua(lua_State* l)
{
	int n = lua_gettop(l);
	Socket s = (Socket)lua_topointer(l,1);
	lua_pop(l,n);
	str buf = socket_recv(s);
	str2lua(buf);
	return 1;
}

static int JSONLua(lua_State* l)
{
	int n = lua_gettop(l);
	str json = lua2str(1);
	lua_pop(l,n);
	debug("Parsed json %s", parse_json_string(json));
	return 1;
}

typedef struct {
	const char* name;
	void* func;
} LuaFunctions;

static LuaFunctions lua_glob_functions[] = {
	{ "exit", ExitLua },
	{ "print", PrintLua },
	{ "debug", DebugLua },
	{ "error", ErrorLua },
	{ "use", UseLua },
	{ "include", IncludeLua },
	{ "cwd", CWDLua },
	{ "header", HeaderLua },
	{ "param", ParamLua },
	{ "now", NowLua },
	{ "query", QueryLua },
	{ "encode", EncodeLua },
	{ "decode", DecodeLua },
	{ "md5sum", MD5SumLua },
	{ "hmac1", Hmac1Lua },
	{ "base64", Base64Lua },
	{ "hex", HexLua },
	{ "rnd", RandomLua },
	{ "post", PostHTTPLua },
	{ "socket", SocketLua },
	{ "send", SendLua },
	{ "receive", ReceiveLua },
	{ "parse", JSONLua },
	{ NULL, NULL }
};

void
init_lua()
{
	int i = 0;
	lins_database = new_database();
	lins = lua_open();
	luaL_openlibs(lins);
	luaopen_base(lins);
	luaopen_string(lins);
	luaopen_table(lins);
	luaopen_math(lins);
	for (i = 0; lua_glob_functions[i].name; ++i)
		lua_register(lins,lua_glob_functions[i].name,lua_glob_functions[i].func);
	ProceduresLua(lins,"public");
	str filename = file_path(Req ? Req->host : $("localhost"), $("/common.lua"));
	luaL_dofile(lins,filename->data);
	Headers data = Resp->req->query_vars;
	if(data) over(data,i) {
		skip_null(data,i);
		str2lua(Key(data,i));
		str2lua(Value(data,i));
		lua_settable(lins,LUA_GLOBALSINDEX);
	}
}

void
end_lua()
{
	lua_close(lins);
	close_database(lins_database);
}

int
run_lua_script(File fc, Headers data)
{
	init_lua();
	if (!setjmp(l_jump)) {
		if (Resp) EvalLuaFile(fc);
		else if (luaL_loadbuffer(lins,fc->data, fc->st.st_size, fc->name) || lua_pcall(lins,0,LUA_MULTRET,0)) error("Failed to evaluate script %c",fc->name);
	}
	end_lua();
	return 0;
}

int
lws_handler(File fc)
{
	cache_control(Resp->headers,"max-age=0, no-cache");
	date_field(Resp->headers,Date(time(NULL))->data);
	send_status(Resp->sc,200);
	send_headers(Resp->sc,Resp->headers);
	run_lua_script(fc,NULL);
	return 200;
}

