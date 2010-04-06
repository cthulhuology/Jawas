// lua_db.c
//
// Lua postgresql stored procedure bindings
//
// © 2010 David J. Goehrig
//

#include "include.h"
#include "defines.h"
#include "memory.h"
#include "log.h"
#include "headers.h"
#include "database.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

extern lua_State* lins;
extern str lins_buffer;
extern Database lins_database;
extern Database db;

str lua2str(int index);
void str2lua(str s);

Headers stored_procedures;

static int DBClosureLua(lua_State* l)
{
	if (! lins_database) lins_database = new_database();
	int n = lua_gettop(l);
	str qry = lua2str(lua_upvalueindex(1));
	for (int i = 1; i <= n; ++i)
		qry = append(qry,_("'%s'%s", lua2str(i), i < n ? _(","): _("")));
	lua_pop(l,n);
	qry = append(qry,_(")"));
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

int PrepareProcedures(char* schema) 
{
	new_database();
	str sql = _("SELECT * from functions_in('%c');", schema);
	int res = query(sql);
	if (res < 0) {
		dblog("%s",db_error());
		close_database(db);
		return 1;
	}
	stored_procedures = new_headers();
	for (int i = 0; i < res && i < MAX_HEADERS; ++i) {
		str proc = fetch(i,0);
		append_header(stored_procedures,proc,_("SELECT * FROM %s(",proc));
	}
	close_database(db);
	return 0;
}

int ProceduresLua(lua_State* l)
{
	int i;
	int n = lua_gettop(l);
	lua_pop(l,n);
	over (stored_procedures,i) {
		str2lua(Key(stored_procedures,i)); 				// once to bind it
		str2lua(Value(stored_procedures,i));		// once to store base query
		lua_pushcclosure(l,DBClosureLua,1);
		lua_settable(l,LUA_GLOBALSINDEX);
	}
	return 0;
}

