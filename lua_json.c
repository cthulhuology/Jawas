// lua_json.c
//
// © 2009 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "config.h"
#include "memory.h"
#include "str.h"
#include "headers.h"
#include "log.h"
#include "lua_json.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

extern lua_State* lins;
str lua2str(int index);
void str2lua(str s);

int colon(str s, int o) { return at(s,o) == ':'; }
int comma(str s, int o) { return at(s,o) == ','; }
int quote(str s, int o) { return at(s,o) == '"'; }
int backslash(str s, int o) { return at(s,o) == '\\'; }

int
digit(str s, int o)
{
	char c = at(s,o);
	return c >= '0' && c <= '9';
}

int
bracket(str s, int o)
{
	char c = at(s,o);
	return c == '{' || c == '(' || c == '[' || c == '}' || c == ']' || c == ')';
}

int
space(str s, int o)
{
	char c = at(s,o);
	return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}

int
extension(str s, int o)
{
	char c = at(s,o);
	return c == '-' || c == '+' || c == 'e' || c == 'E' || c == '.';
}

str
quoted_string(str s, int* o)
{
	str retval = blank(len(s) - *o);
	int i;
	if (quote(s,*o)) ++*o;
	for (i = 0; i+*o < len(s); ++i)  {
		if (quote(s,*o+i)) {
			*o = i > 0 ? *o+i : *o;
			str2lua(i > 0 ? retval : NULL);
			return i > 0 ? retval : NULL;
		}
		if (backslash(s,i+*o)) ++i;
		if (i+*o < len(s)) {
			retval->data[i] = at(s,i+*o);
			retval->length = i+1;
		}
	}
	*o = i > 0 ? *o+i : *o;
	str2lua(i > 0 ? retval : NULL);
	return retval;
}

str
token(str s, int* o)
{
	str retval = blank(len(s) - *o);
	int i;
	for (i = 0; i+*o < len(s); ++i) {
		if (quote(s,i+*o) || colon(s,i+*o) || space(s,i+*o) || bracket(s,i+*o) || comma(s,i+*o))  {
			*o = i > 0 ? *o+i : *o;
			str2lua(i > 0 ? retval : NULL);
			return i > 0 ? retval : NULL;
		}
		retval->data[i] = at(s,i+*o);
		retval->length = i+1;
	}
	*o = i > 0 ? *o+i : *o;
	str2lua(i > 0 ? retval : NULL);
	return retval;
}

str
number(str s, int* o)
{
	str retval = blank(len(s) - *o);
	int i;
	for (i = 0; i+*o < len(s); ++i) {
		if (!digit(s,i+*o) && !extension(s,i+*o)) {
			*o = i > 0 ? *o+i : *o;
			str2lua(i > 0 ? retval : NULL);
			return i>0 ? retval : NULL;
		}
		retval->data[i] = at(s,i+*o);
		retval->length = i+1;
	}
	*o = i > 0 ? *o+i : *o;
	str2lua(i > 0 ? retval : NULL);
	return retval;
}

int array(str s, int o) { return at(s,o) == '['; }
int array_end(str s, int o) { return at(s,o) == ']'; }

int object(str s, int o) { return at(s,o) == '{'; }
int object_end(str s, int o)  { return at(s,o) == '}'; }

str
parse_array(str s, int* o)
{
	int off, i, j = 0;
	lua_newtable(lins);
	int n = lua_gettop(lins);
	for (i = 1; i+*o < len(s); ++i) {
		if (space(s,i+*o)) continue;
		if (array_end(s,i+*o)) {
			++i;
			break;
		}
		off = i + *o;
		lua_pushnumber(lins,++j);
		bracket(s,off) ? parse_json_object(s, &off) :
			quote(s,off) ? quoted_string(s,&off) : token(s,&off);
		i = off - *o;
		while(!array_end(s,i+*o) && !comma(s,i+*o) && i+*o < len(s)) ++i;
		lua_settable(lins,n);
		if (array_end(s,i+*o)) {
			++i;
			break;
		}
	}
	off = *o;
	*o = *o + i;
	return from(s,off,*o-off);
}

str
parse_json_object(str s, int* o)
{
	int off,i;
	if (array(s,*o)) return parse_array(s,o);
	lua_newtable(lins);
	int n = lua_gettop(lins);
	for (i = 1; i+*o < len(s); ++i) {
		if (space(s,i+*o)) continue;
		if (object_end(s,i+*o)) {
			++i;
			break;
		}
		off = i+*o;
		quote(s,off) ? quoted_string(s,&off) : token(s,&off);
		i = off - *o;
		while (!colon(s,*o+i) && *o+i < len(s)) ++i;
		++i;
		while (space(s,*o+i) && *o+i < len(s)) ++i;
		off = i+*o;
		bracket(s,off) ? parse_json_object(s,&off) :
			quote(s,off) ? quoted_string(s,&off) : token(s,&off);
		lua_settable(lins,n);
		i = off - *o;
		while (!object_end(s,*o+i) && !comma(s,*o+i) && *o+i < len(s) ) ++i;
		if (object_end(s,i+*o)) {
			++i;
			break;
		}
	}
	off = *o;
	*o = *o + i;
	return from(s,off,*o-off);
}

str
parse_json_string(str s)
{
	for (int o = 0; o < len(s); ++o) {
		if (space(s,o)) continue;
		if (bracket(s,o)) return parse_json_object(s,&o);
	}
	return NULL;
}
