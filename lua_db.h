// lua_db.h
//
// © 2010 David J. Goehrig
//

#ifndef __HAVE_LUA_DB_H__
#define __HAVE_LUA_DB_H__

#include <lua.h>
int PrepareProcedures(char* schema);
int ProceduresLua(lua_State* l);

#endif
