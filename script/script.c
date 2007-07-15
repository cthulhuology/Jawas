// script.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "script.h"
#include "atoms.h"
#include "objects.h"

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

int
jdata_int(JData data)
{
	return strtol(data->data,NULL,0);
}

double
jdata_num(JData data)
{
	return strtod(data->data,NULL);
}

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
	int i;
	for (i = 0; obj && i < Length(obj); ++i) {
		if (Prop(SlotProp(obj,i)) == prop)
			return SlotValue(obj,i);
		if (!SlotProp(obj,i))
			obj = (JObject)SlotValue(obj,i);
	}
	return Undefined;
}

JData
obj_put(JObject obj, JData prop, JData value)
{
	int i;
	if (False == obj_can_put(obj,prop)) return False;
	for (i = 0; obj && i < Length(obj); ++i) {
		if (! SlotProp(obj,i) && ! SlotValue(obj,i)) {
			SlotProp(obj,i) = prop;
			SlotValue(obj,i) = value;
			return True;
		}
		if (! SlotProp(obj,i)) 
			obj = (JObject)SlotValue(obj,i);
	}
	return False;
}

JData
obj_class(JObject obj)
{
	return obj_get(obj,Class);
}

JData
obj_can_put(JObject obj, JData prop)
{
	int i;
	if (True == obj_has_prop(obj,prop)) {
		for (i = 0; obj && i < Length(obj); ++i) {
			if (Prop(SlotProp(obj,i)) == prop) 
				return ReadOnlyProp(SlotProp(obj,i)) ? False : True;
			if (!SlotProp(obj,i))
				obj = SlotValue(obj,i);
		}
	}
	return True;	
}

JData
obj_has_prop(JObject obj, JData prop)
{
	int i;
	for (i = 0; obj && i < Length(obj); ++i) {
		if (Prop(SlotProp(obj,i) == prop))
			return True;
		if (!SlotProp(obj,i))
			obj = SlotValue(obj,i);
	}	
	return False;
}

JData
obj_delete(JObject obj, JData prop)
{
	int i;
	for (i = 0; obj && i < Length(obj); ++i) {
		if (Prop(SlotProp(obj,i) == prop)) {
			SlotProp(obj,i) = NULL;
			SlotValue(obj,i) = NULL;
			return True;
		}
		if (!SlotProp(obj,i))
			obj = SlotValue(obj,i);
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
	return False;	
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

JData
obj_call(JObject obj, JObject args)
{
	return False;
}

JObject
obj_construct(JObject obj, JObject args)
{
	return obj;
}

JData
obj_toBoolean(JObject obj, JSObject args)
{
	JData value = obj_default_value(obj);
	if (value == Undefined) return False;
	if (value == Null) return False;
	if (value == False || value == True) return value;
	if (value == Nan) return False;
	if (value == Zero) return False;
	return True;
}

JData
obj_toNumber(JObject obj, JSObject args)
{
	JData value = obj_default_value(obj);
	if (value == Undefined) return NaN;
	if (value == Null) return Zero;
	if (value == True) return One;
	if (value == False) return Zero;
	return jdata_num(num_jdata(value));
}

JData
obj_toInteger(JObject obj, JSObject args)
{
	JData value = obj_default_value(obj);
	if (value == Undefined) return NaN;
	if (value == Null) return Zero;
	if (value == True) return One;
	if (value == False) return Zero;
	return jdata_int(int_jdata(value));
}

JData
obj_toString(JObject obj, JSObject args)
{
	return obj_default_value(obj);
}

JObject
data_toObject(JData data)
{

}



