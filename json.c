// json.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "json.h"
#include "jws.h"
#include "jsstr.h"
#include "jsatom.h"
#include "headers.h"
#include "server.h"
#include "defines.h"

str
json(JSContext* cx, JSObject* obj)
{
	int i;
	Headers kv = new_headers();
	JSIdArray* arr = JS_Enumerate(cx,obj);
	for (i = 0; i < arr->length; ++i) {
		char* prop = JS_GetStringBytes(ATOM_TO_STRING(JSID_TO_ATOM(arr->vector[i])));
		jsval value;
		str key = Str("%c",prop);
		JS_GetProperty(cx,obj,prop,&value);
		switch (JS_TypeOfValue(cx,value)) {
			case JSTYPE_OBJECT:
				append_header(kv,key, json(cx,JSVAL_TO_OBJECT(value)));
				break;
			case JSTYPE_STRING:
				append_header(kv,key, Str("\"%s\"",jsval2str(value)));
				break;
			case JSTYPE_NUMBER:
			case JSTYPE_BOOLEAN:
				append_header(kv,key,jsval2str(value));
				break;
			case JSTYPE_FUNCTION:
			case JSTYPE_VOID:
			default:
				break;
		}
	}
	return Str("{ %s }", list_headers(kv));	
}

Headers
parse_json(Headers hd, str buf, int pos)
{
	debug("Found json [%s]",from(buf,pos,len(buf)-pos));
	return append_header(hd,Str("json"),from(buf,pos,len(buf)-pos));
}
