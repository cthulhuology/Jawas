// script.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "script.h"
#include "atoms.h"
#include "objects.h"

// JData functions

JData
new_jdata(char* data, int len)
{
	JData retval;
	retval = (JData)malloc(len + sizeof(struct jdata_struct));
	if (data) memcpy(retval->data,data,len);
	retval->len = len;
	return retval;
}

JData
int_jdata(int data)
{
	JData retval;
	retval = (JData)malloc(MAX_INT_SIZE + sizeof(struct jdata_struct));
	retval->len = sprintf(retval->data,"%d",data);
	return retval;
}

JData
num_jdata(double data)
{
	JData retval;
	retval = (JData)malloc(MAX_DOUBLE_SIZE + sizeof(struct jdata_struct));
	retval->len = sprintf(retval->data,"%g",data);
	return retval;
}

JObject
jdata_toObject(JData data)
{

}

// Object functions

JObject
new_object(int slots)
{
	JObject retval = (JObject)malloc(sizeof(struct jobject_struct) + sizeof(jslot_struct)* slots);
	retval->len = slots;
	return retval;
}

JData
obj_get(JObject obj, JData prop)
{
	ITERATE(obj) {
		if (SlotProp(obj,i) == prop)
			return SlotValue(obj,i);
		CHASE_TAIL
	}
	return Undefined;
}

JData
obj_put(JObject obj, JData prop, JData value)
{
	if (False == obj_can_put(obj,prop)) return False;
	ITERATE(obj) {
		if (! SlotProp(obj,i) && ! SlotValue(obj,i)) {
			SlotProp(obj,i) = prop;
			SlotValue(obj,i) = value;
			return True;
		}
		CHASE_TAIL
	}
	return False;
}

JObject
obj_prototype(JObject obj, JData prop)
{
	ITERATE(o) {
		if (! SlotProp(obj,i)) 
			return SlotValue(obj,i);
	}
	return Undefined;
}

JData
obj_class(JObject obj)
{
	return obj_get(obj,Class);
}

JData
obj_can_put(JObject obj, JData prop)
{
	ITERATE(obj) {
		if (!SlotProp(obj,i) && ! SlotValue(obj,i))
			return True;
		if (SlotProp(obj,i) == prop)
			return ReadOnly(SlotPerm(obj,i)) ? False : True;
		CHASE_TAIL
	}
	return False;
}

JData
obj_has_prop(JObject obj, JData prop)
{
	ITERATE(obj) {
		if (SlotProp(obj,i) == prop)
			return True;
		CHASE_TAIL
	}	
	return False;
}

JData
obj_delete(JObject obj, JData prop)
{
	ITERATE(obj) {
		if (SlotProp(obj,i) == prop 
		&& !DontDel(SlotPerm(obj,i))) {
			SlotProp(obj,i) = NULL;
			SlotValue(obj,i) = NULL;
			return True;
		}
		CHASE_TAIL
	}
	return False
}

JData
obj_default_value(JObject obj)
{
	return obj_get(obj,Value);
}

JData
obj_has_instance(JObject obj, JData value)
{
	return obj_get(obj,Instance);
}

JObject
obj_scope(JObject obj)
{
	return (JObject)obj_get(obj,Scope);
}

JData
obj_match(JObject obj, JData str, JData index)
{
	return False;	
}

Func(obj_call)
{
	JObject funj;
	JData value;
	ITERATE(obj) {
		if (SlotProp(obj,i) == SlotValue(args,0)) {
			value = SlotValue(obj,i);
			if (! value)
				return Undefined;
			if (isFUNC(value))
				return CALL(value)(obj,args);
			if (isPRIM(value)) {
				if (value == obj_get) 
					return obj_get(obj,SlotValue(args,0));
				if (value == obj_put)
					return obj_put(obj,SotValue(args,0),SlotValue(args,1));
				if (value == obj_prototype)
					return obj_prototype(obj,SlotValue(args,0));
				if (value == obj_class)
					return obj_class(obj);
				if (value == obj_can_put)
					return obj_can_put(obj,SlotValue(args,0));
				if (value == obj_has_prop)
					return obj_has_prop(obj,SlotValue(args,0));
				if (value == obj_scope)
					return obj_scope(obj);
				if (value == obj_match)
					return obj_match(obj,SlotValue(args,0), SlotValue(args,1));
				return Undefined;
			}
			if (isVALUE(value)) {
				funj = func_construct(value,args);
				obj_put(args,Callee,funj);
				obj_put(args,This,obj);
				return func_apply(funj,args);
			}
		}
		CHASE_TAIL
	}
	return Undefined;
}

Func(obj_eval)
{

}

Func(obj_construct)
{
	JObject retval;
	int slots = 0;
	ITERATE(obj) {
		if (SlotProp(obj,i) == Instance)
			SlotValue(obj,i) = True;
		if (isValue(SlotValue(obj,i)))
			++slots;
		CHASE_TAIL
	}
	ITERATE(args) {
		if (isValue(SlotValue(obj,i)))
			++slots;
	}
	retval = OBJECT(slots);	
	ITERATE(retval) {
		SlotProp(retval,i) = NULL;
		SlotValue(retval,i) = NULL;
		if (i == Length(retval) - 3) {
			SlotValue(retval,i) = obj;	
			break;
		}
	}
	ITERATE(obj) {
		if (isValue(SlotValue(obj,i)))
			obj_put(retval,SlotProp(obj,i),SlotValue(obj,i));
		CHASE_TAIL
	}
	ITERATE(args) {
		if (isValue(SlotValue(args,i)))
			obj_put(retval,SlotProp(args,i),SlotValue(obj,i));
	}
	return retval;
}

Func(obj_toString)
{
	return obj_default_value(obj);
}

Func(obj_toBoolean)
{
	JData value = obj_default_value(obj);
	if (value == Undefined) return False;
	if (value == Null) return False;
	if (value == False || value == True) return value;
	if (value == Nan) return False;
	if (value == Zero) return False;
	return True;
}

Func(obj_toInteger)
{
	JData value = obj_default_value(obj);
	if (value == Undefined) return NaN;
	if (value == Null) return Zero;
	if (value == True) return One;
	if (value == False) return Zero;
	return jdata_int(int_jdata(value));
}

Func(obj_toNumber)
{
	JData value = obj_default_value(obj);
	if (value == Undefined) return NaN;
	if (value == Null) return Zero;
	if (value == True) return One;
	if (value == False) return Zero;
	return jdata_num(num_jdata(value));
}

Func(obj_value_of)
{
	return JDATA_OBJ(obj);
}

Func(obj_has_own_prop)
{
	JProp prop = obj_toString(args);
	ITERATE(obj) {
		if (SlotProp(obj,i) == prop)
			return True;
	}
	return False;
}

Func(obj_is_proto_of)
{
	JObject proto = obj_prototype(obj);
	if (proto == SlotValue(args,0))
		return True;
	if (proto != object_obj)
		return obj_is_proto_of(proto,args);
	return False;
}

Func(obj_prop_is_enum)
{

}

Func(obj_enumerate)
{

}

// String functions

Func(str_construct)
{

}

Func(str_substring)
{

}

Func(str_split)
{

}

Func(str_slice)
{

}

Func(str_search)
{

}

Func(str_replace)
{

}

Func(str_index_of)
{

}

Func(str_last_index_of)
{

}

Func(str_concat)
{

}

Func(str_char_at)
{

}

Func(str_char_code_at)
{

}

Func(str_encodeURI)
{

}

Func(str_decodeURI)
{

}

Func(str_tolower)
{

}

Func(str_toupper)
{

}

// Boolean functions

Func(bool_construct)
{

}

// Integer functions

Func(int_construct)
{

}

// Number functions

Func(num_construct)
{

}

Func(num_isNan)
{
	return NaN == obj_defalt_value(obj) ? True : False;
}

Func(num_isFinite)
{
	return Infinity == obj_default_value(obj) ? False : True;
}

Func(num_to_fixed)
{

}

Func(num_to_exp)
{

}

Func(num_to_precision)
{

}

// Function functions

Func(func_construct)
{

}

Func(func_apply)
{

}

// Array functions

Func(array_concat)
{

}

Func(array_join)
{

}

Func(array_push)
{

}

Func(array_pop)
{

}

Func(array_reverse)
{

}

Func(array_shift)
{

}

Func(array_unshift)
{

}

Func(array_slice)
{

}

Func(array_sort)
{

}

Func(array_splice)
{

}

// Math functions

Func(math_e)
{
}

Func(math_ln10)
{
}

Func(math_ln2)
{
}

Func(math_log2e)
{
}

Func(math_log10e)
{
}

Func(math_pi)
{
}

Func(math_sqrt12)
{
}

Func(math_sqrt2)
{
}

Func(math_abs)
{
}

Func(math_acos)
{
}

Func(math_asin)
{
}

Func(math_atan)
{
}

Func(math_atan2)
{
}

Func(math_ceil)
{
}

Func(math_cos)
{
}

Func(math_exp)
{
}

Func(math_floor)
{
}

Func(math_log)
{
}

Func(math_max)
{
}

Func(math_min)
{
}

Func(math_pow)
{
}

Func(math_random)
{
}

Func(math_round)
{
}

Func(math_sin)
{
}

Func(math_sqrt)
{
}

Func(math_tan)
{
}

// Date functions

Func(date_make_day)
{

}

Func(date_make_time)
{
}

Func(date_make_date)
{
}

Func(date_time_clip)
{
}

Func(date_parse)
{
}

Func(date_utc)
{
}

Func(date_to_date_string)
{
}

Func(date_to_time_string)
{
}

Func(date_get_time)
{
}

Func(date_get_full_year)
{
}

Func(date_get_month)
{
}

Func(date_get_date)
{
}

Func(date_get_day)
{
}

Func(date_get_hours)
{
}

Func(date_get_minutes)
{
}

Func(date_get_seconds)
{
}

Func(date_get_milliseconds)
{
}

Func(date_get_timezone)
{
}

Func(date_set_time)
{
}

Func(date_set_milliseconds)
{
}

Func(date_set_seconds)
{
}

Func(date_set_minutes)
{
}

Func(date_set_hours)
{
}

Func(date_set_date)
{
}

Func(date_set_month)
{
}

Func(date_set_full_year)
{
}

// File functions
Func(file_open)
{
}

Func(file_close)
{
}

Func(file_read)
{
}

Func(file_write)
{
}

Func(file_seek)
{
}

// Socket functions
Func(socket_connect)
{
}

Func(socket_listen)
{
}

Func(socket_accept)
{
}

Func(socket_bind)
{
}

Func(socket_shutdown)
{
}

Func(socket_read)
{
}

Func(socket_write)
{
}

// RegExp functions

Func(regexp_exec)
{
}

Func(regexp_test)
{
}

