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
copy_jdata(JData data)
{
	return new_jdata(data->data,Length(data));
}

int* IntCache = NULL;
JData* JDataCache = NULL;

void
init_int_cache()
{
	IntCache = (int*)malloc(sizeof(int)*2 * MAX_INT_CACHE);
	for (int i = 0; i < MAX_INT_CACHE; ++i) 
		IntCache[i*2] = sprintf(&IntCache[i*2+1],"%d",i);
}

void
init_jdata_cache()
{
	JDataCache = (JData*)malloc(sizeof(JData) * MAX_JDATA_CACHE);
	for (int i = 0; i < MAX_JDATA_CACHE; ++i) JDataCache[i] = NULL;
}

JData
ident_jdata(char* data, int len)
{
	char* extra = NULL;
	int i = strtol(data,&extra,0);					
	if (!extra && i < MAX_INT_CACHE) return (JData)&IntCache[i*2];
	for (i = 0; i < MAX_JDATA_CACHE && JDataCache[i]; ++i)
		if (Length(JDataCache[i]) == len) 
			if(!memcmp(JDataCache[i]->data,data,len)) 
				return JDataCache[i];
	JDataCache[i] = new_jdata(data,len);
	return JDataCache[i];
}

JData
int_jdata(int data)
{
	JData retval;
	if (data < MAX_INT_CACHE) return (JData)&IntCache[i*2];
	retval = (JData)malloc(MAX_INT_SIZE + sizeof(struct jdata_struct));
	retval->len = sprintf(retval->data,"%d",data);
	return ident_jdata(retval->data,retval->len);
}

JData
num_jdata(double data)
{
	JData retval;
	retval = (JData)malloc(MAX_DOUBLE_SIZE + sizeof(struct jdata_struct));
	retval->len = sprintf(retval->data,"%g",data);
	return ident_jdata(retval->data,retval->len);
}

JObject
jdata_toObject(JData data)
{
	return (JObject)data;
}

JData
jdata_boolean(JData value) 
{
	if (!value) return False;
	if (value == Undefined) return False;
	if (value == Null) return False;
	if (value == False || value == True) return value;
	if (value == Nan) return False;
	if (value == Zero) return False;
	return True;
}

// JObject functions

JObject
new_object(int slots)
{
	JObject retval = (JObject)malloc(sizeof(struct jobject_struct) + sizeof(jslot_struct)* slots);
	retval->len = OBJECT_MASK | slots;
	return retval;
}

// Object Primitives

JData
obj_get(JObject obj, JData prop)
{
	ITERATE(obj) {
		if (Prop(i) == prop)
			return Value(i);
		CHASE_PROTOTYPE
	}
	return Undefined;
}

JData
obj_put(JObject obj, JData prop, JData value)
{
	if (False == obj_can_put(obj,prop)) return False;
	ITERATE(obj) {
		if (!Prop(i) && ! Value(i)) {
			Prop(i) = prop;
			Value(i) = value;
			return True;
		}
		CHASE_PROTOTYPE
	}
	return False;
}

JData
obj_prototype(JObject obj, JData prop)
{
	ITERATE(o) {
		if (!Prop(i)) 
			return Value(i);
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
		if (!Prop(i) && ! Value(i))
			return True;
		if (Prop(i) == prop)
			return ReadOnly(Perm(i)) ? False : True;
		CHASE_PROTOTYPE
	}
	return False;
}

JData
obj_has_prop(JObject obj, JData prop)
{
	ITERATE(obj) {
		if (Prop(i) == prop)
			return True;
		CHASE_PROTOTYPE
	}	
	return False;
}

JData
obj_delete(JObject obj, JData prop)
{
	ITERATE(obj) {
		if (Prop(obj) == prop 
		&& !DontDel(Perm(i))) {
			Prop(i) = NULL;
			Value(i) = NULL;
			return True;
		}
		CHASE_PROTOTYPE
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

JData
obj_scope(JObject obj)
{
	return (JObject)obj_get(obj,Scope);
}

JData
obj_match(JObject obj, JData str, JData index)
{
	return False;	
}

// Object Functions

Func(obj_call)
{
	JObject funj;
	JData value;
	ITERATE(obj) {
		if (Prop(i) == Args(0))) {
			value = Value(i);
			if (! value)
				return Undefined;
			if (isFUNC(value))
				return CALL(value)(obj,args);
			if (isPRIM(value)) {
				if (value == obj_get) 
					return obj_get(obj,Args(0));
				if (value == obj_put)
					return obj_put(obj,Args(0),Value(1));
				if (value == obj_prototype)
					return obj_prototype(obj,Args(0));
				if (value == obj_class)
					return obj_class(obj);
				if (value == obj_can_put)
					return obj_can_put(obj,Args(0));
				if (value == obj_has_prop)
					return obj_has_prop(obj,Args(0));
				if (value == obj_scope)
					return obj_scope(obj);
				if (value == obj_match)
					return obj_match(obj,Args(0),Args(1));
				return Undefined;
			}
			if (isVALUE(value)) {
				funj = func_construct(value,args);
				obj_put(args,Callee,funj);
				obj_put(args,This,obj);
				return func_apply(funj,args);
			}
		}
		CHASE_PROTOTYPE
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
		if (Prop(i) == Instance)
			Value(i) = True;
		if (isValue(Value(i)))
			++slots;
		CHASE_PROTOTYPE
	}
	ITERATE(args)
		if (isValue(Args(i)))
			++slots;
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
		if (isValue(Value(i)))
			obj_put(retval,Prop(i),Value(i));
		CHASE_PROTOTYPE
	}
	ITERATE(args)
		if (isValue(Args(i)))
			obj_put(retval,SlotProp(args,i),Args(i));
	return retval;
}

Func(obj_toString)
{
	return ObjValue;
}

Func(obj_toBoolean)
{
	return jdata_boolean(ObjValue);
}

Func(obj_toInteger)
{
	JData value = ObjValue;
	if (value == Undefined) return NaN;
	if (value == Null) return Zero;
	if (value == True) return One;
	if (value == False) return Zero;
	return jdata_int(int_jdata(value));
}

Func(obj_toNumber)
{
	JData value = ObjValue;
	if (value == Undefined) return NaN;
	if (value == Null) return Zero;
	if (value == True) return One;
	if (value == False) return Zero;
	return jdata_num(num_jdata(value));
}

Func(obj_value_of)
{
	return ObjValue;
}

Func(obj_has_own_prop)
{
	JData prop = Args(0);
	ITERATE(obj)
		if (Prop(i) == prop)
			return True;
	return False;
}

Func(obj_is_proto_of)
{
	JObject proto = obj_prototype(obj);
	if (proto == Args(0))
		return True;
	if (proto != object_obj)
		return obj_is_proto_of(proto,args);
	return False;
}

Func(obj_prop_is_enum)
{
	JData prop = Args(0);
	ITERATE(obj)  {
		if (Prop(i) == prop)
			return DontEnum(Perm(i)) ? False : True;
		CHASE_PROTOTYPE
	}
	return True;
}

Func(obj_enumerate)
{
	JObject funj,funa; 
	JObject retval;
	int count = 0;
	ITERATE(obj) {
		if (DontEnum(Perm(i))) continue;
		if (Prop(i)) ++count;
		CHASE_PROTOTYPE
	}
	retval = ARRAY(count);
	funa = ARGS(1);
	funj = Args(0);	
	count = 0;
	ITERATE(obj) {
		if (DontEnum(Perm(i))) continue;
		if (Prop(i))  {
			SlotValue(funa,0) = Value(i);
			obj_put(retval,INT_JDATA(count++),func_apply(funj,funa))
		}
		CHASE_PROTOTYPE
	}
	return retval;	
}

// String functions

Func(str_construct)
{
	JObject retval;
	const_args = ARGS(1);
	SlotProp(const_args,0) = Value;
	SlotValue(const_args,0) = Args(0);
	retval = obj_construct(obj_string,const_args);
	return retval;	
}

Func(str_substring)
{
	int start, end;
	JData value = ObjValue;
	start = JDATA_INT(Arg(0));
	end = (argc == 1 ? Length(obj) : JDATA_INT(Arg(1)));
	return  new_jdata(value->data + start,end-start);
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
	JData value = ObjValue; 
	JData search = Args(0);
	int slen = Length(search);
	int vlen = Length(value);
	int pos = (Argc == 1 ? 0 : JDATA_INT(Args(1)));
	ITERATE(value) 
		if ((pos + i + slen < vlen) && !strncmp(&value->data[i+pos],search->data,slen)) 
			return INT_JDATA(i);
	return NegOne;
}

Func(str_last_index_of)
{
	JData value = ObjValue;
	JData search = Args(0);
	int slen = Length(search);
	int vlen = Length(value);
	int pos = (Argc == 1 ? 0 : JDATA_INT(Args(1)));
	for (int i = vlen-slen; i >= pos; --i)
		if (!strncmp(&value->data[i],search->data,slen))
			return INT_JDATA(i);
	return NegOne;
}

Func(str_concat)
{
	JData retval, value = ObjValue;	
	int off, len = 0;
	ITERATE(args)
		len += Length(Args(i));
	retval = new_jdata(NULL,Length(value) + len);
	memcpy(retval->data,value->data,Length(value));
	off = Length(value);
	ITERATE(args) {
		memcpy(retval->data + off, Args(i)->data, Length(Args(i)));
		off += Length(Args(i));
	}
	return retval;
}

Func(str_char_at)
{
	JData value = ObjValue;	
	int pos = JDATA_INT(Args(0));
	return pos >= Length(value) ? Empty : new_jdata(&value->data[pos],1);
}

Func(str_char_code_at)
{
	JData value = ObjValue;	
	int pos = JDATA_INT(Args(0));
	return pos >= Length(value) ? NaN : INT_JDATA(value->data[pos]));
}

Func(str_encodeURI)
{

}

Func(str_decodeURI)
{

}

Func(str_tolower)
{
	JDate retval = copy_jdata(ObjValue);
	ITERATE(retval) 
		retval[i] = tolower(retval[i]);
	return retval;	
}

Func(str_toupper)
{
	JDate retval = copy_jdata(ObjValue);
	ITERATE(retval) 
		retval[i] = toupper(retval[i]);
	return retval;	
}

// Boolean functions

Func(bool_construct)
{
	JObject bool_args = ARGS(1);
	SlotProp(bool_args,0) = Value;
	SlotValue(bool_args,0) = jdata_boolean(Args(0));
	return obj_construct(boolean_obj,bool_args);
}

// Integer functions

Func(int_construct)
{
	JObject int_args = ARGS(1);
	SlotProp(int_args,0) = Value;
	SlotValue(int_args,0) = JDATA_INT(Args(0));
	return obj_construct(integer_obj,int_args);
}

// Number functions

Func(num_construct)
{
	JObject num_args = ARGS(1);
	SlotProp(num_args,0) = Value;
	SlotValue(num_args,0) = JDATA_NUM(Args(0));
	return obj_construct(integer_obj,num_args);
}

Func(num_isNan)
{
	return NaN == ObjValue ? True : False;
}

Func(num_isFinite)
{
	return Infinity == ObjValue ? False : True;
}

Func(num_to_fixed)
{
	return INT_JDATA(lround(JDATA_NUM(ObjValue)));
}

Func(num_to_exp)
{
	return Zero;
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

// Argument objects

JObject
new_arguments(int len)
{

}

// Array functions

JObject
new_array(int len)
{

}

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

Func(date_construct)
{
	JData tm = new_jdata(NULL,sizeof(struct tm));
	JObject tm_args = ARGS(2);
	SlotProp(tm_args,1) = Value;
	SlotValue(tm_args,1) = INT_JDATA(mktime(tm->data));
	return obj_construct(date_obj,tm_args);
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
	return ObjValue;
}

Func(date_get_full_year)
{
	LOCALTIME_TMS
	return INT_JDATA(tms.tm_year + 1900);
}

Func(date_get_month)
{
	LOCALTIME_TMS
	return INT_JDATA(tms.tm_mon);
}

Func(date_get_date)
{
	LOCALTIME_TMS
	return INT_JDATA(tms.tm_mday);
}

Func(date_get_day)
{
	LOCALTIME_TMS
	return INT_JDATA(tms.tm_wday);
}

Func(date_get_hours)
{
	LOCALTIME_TMS
	return INT_JDATA(tms.tm_hour);
}

Func(date_get_minutes)
{
	LOCALTIME_TMS
	return INT_JDATA(tms.tm_min);
}

Func(date_get_seconds)
{
	LOCALTIME_TMS
	return INT_JDATA(tms.tm_sec);
}

Func(date_get_milliseconds)
{
	return Zero;
}

Func(date_get_timezone)
{
	LOCALTIME_TMS
	return INT_JDATA(tms.tm_gmtoff);
}

Func(date_set_time)
{
	obj_put(obj,Value,Args(0));
	return ObjValue;
}

Func(date_set_milliseconds)
{
	return ObjValue;
}

Func(date_set_seconds)
{
	LOCALTIME_TMS
	tms->tm_sec = JDATA_INT(Args(0));
	SET_LOCALTIME_TMS
}

Func(date_set_minutes)
{
	LOCALTIME_TMS
	tms->tm_min = JDATA_INT(Args(0));
	SET_LOCALTIME_TMS
}

Func(date_set_hours)
{
	LOCALTIME_TMS
	tms->tm_hour = JDATA_INT(Args(0));
	SET_LOCALTIME_TMS
}

Func(date_set_date)
{
	LOCALTIME_TMS
	tms->tm_mday = JDATA_INT(Args(0));
	SET_LOCALTIME_TMS
}

Func(date_set_month)
{
	LOCALTIME_TMS
	tms->tm_mon = JDATA_INT(Args(0));
	SET_LOCALTIME_TMS
}

Func(date_set_full_year)
{
	LOCALTIME_TMS
	tms->tm_year = JDATA_INT(Args(0)) - 1900;
	SET_LOCALTIME_TMS
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

