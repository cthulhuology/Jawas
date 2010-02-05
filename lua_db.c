// lua_db.c
//
// Lua postgresql stored procedure bindings
//
// © 2010 David J. Goehrig
//

#include "log.h"
#include "database.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

extern lua_State* lins;
extern str lins_buffer;
extern Database lins_database;

str lua2str(int index);
void str2lua(str s);

static int DBClosureLua(lua_State* l)
{
	int n = lua_gettop(l);
	str qry = lua2str(lua_upvalueindex(1));
	for (int i = 1; i <= n; ++i)
		qry = append(qry,Str("'%s'%s", lua2str(i), i < n ? Str(","): Str("")));
	lua_pop(l,n);
	append(qry,Str(")"));
	int res = query(qry);
	if (res < 0) {
		str2lua(db_error());
		return 1;
	}
	lua_newtable(l);
	n = lua_gettop(l);
	for (int i = 0; i < res; ++i) {
		lua_pushnumber(l,i+1);
		lua_newtable(l);
		for (int j = 0; j < fields(); ++j) {
			str2lua(field(j));
			str2lua(fetch(i,j));
			lua_settable(l,n+2);
		}
		lua_settable(l,n);
	}
	return 1;
}

int ProceduresLua(lua_State* l, char* schema)
{
	int n = lua_gettop(l);
	lua_pop(l,n);
	str sql = Str("SELECT functions_in('%c')", schema);
	int res = query(sql);
	if (res < 0) {
		str2lua(db_error());
		return 1;
	}
	for (int i = 0; i < res; ++i) {
		str proc = fetch(i,0);
		str2lua(proc); 				// once to bind it
		str2lua(Str("SELECT * FROM %s(", proc));	// once to store base query
		lua_pushcclosure(l,DBClosureLua,1);
		lua_settable(l,LUA_GLOBALSINDEX);
	}
	return 0;
}

