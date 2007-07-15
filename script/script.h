// script.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#define DEFAULT_SLOTS	8
#define MAX_INT_SIZE	12
#define MAX_DOUBLE_SIZE	32

#define ReadOnlyProp(x)	x & 1
#define DontEnumProp(x)	x & 2
#define DontDelProp(x)	x & 4 

#define R(x)	x | 1
#define E(x)	x | 2
#define D(x)	x | 4 

#define RE(x) R(E(x))
#define RD(x) R(D(x))
#define ED(x) E(D(x))
#define RED(x) R(E(D(x)))

#define FUNCTION_MASK  0xfffffffc
#define PRIM(x) (JData)((int)x | 1)
#define FUNC(x) (JData)((int)x | 2)
#define CALL(x) (func_t)((int)x & FUNCTION_MASK)

#define PROPERTY_MASK 0xfffffff8
#define Prop(x) 	((JData)((int)x & PROPERTY_MASK))

#define Slot(o,i)	o->slots[i]
#define SlotProp(o,i)	Slot(o,i).prop
#define SlotValue(o,i)	Slot(o,i).value

#define Length(o)	o->len

#define NewObject new_object(DEFAULT_SLOTS)

typedef struct jdata_struct JData;
struct jdata_struct {
	int len;
	char data[0];
};

typedef struct jslot_struct JSlot;
struct jslot_struct {
	JData prop;
	JData value;
};

typedef struct jobject_struct JObject;
struct jobject_struct {
	int len;
	JSlot slots[0];
};

JData new_jdata(char* data, int len);
JData int_jdata(int data);
JData num_jdata(double data);

int jdata_int(JData data);
double jdata_num(JData data);

JObject new_object();
JObject init_object(JObject obj, JObject proto);
JObject set_property(JObject obj, JData prop, JData value);
JObject get_property(JObject obj, JData prop);

#endif
