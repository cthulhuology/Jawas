// defines.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#define DEFAULT_SLOTS	8
#define MAX_INT_SIZE	12
#define MAX_DOUBLE_SIZE	32

#define ReadOnly(x)	x & 1
#define DontEnum(x)	x & 2
#define DontDel(x)	x & 4 

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
#define CALL(x) ((func_t)((int)x & FUNCTION_MASK))

#define isVALUE(x) (int)x & 3 == 0
#define isPRIM(x) (int)x & 3 = 1
#define isFUNC(x) (int)x & 3 = 2

#define Func(x) JData x (JObject obj, JObject args)

#define PERM_MASK 0x7
#define Perm(x)		(int)x & PERM_MASK

#define PROPERTY_MASK 0xfffffff8
#define Prop(x) 	((JData)((int)x & PROPERTY_MASK))

#define Slot(o,i)	o->slots[i]
#define SlotPerm(o,i)	Perm(Slot(o,i).prop)
#define SlotProp(o,i)	Prop(Slot(o,i).prop)
#define SlotValue(o,i)	Slot(o,i).value

#define Length(o)	o->len

#define OBJECT(x)	new_object(DEFAULT_SLOTS + x)
#define OBJECT 		OBJECT(0)

#define ITERATE(o) \
	for (int i = 0; o && i < Length(o); ++i)

#define CHASE_TAIL(o,i) \
	if (!SlotProp(o,i)) { \
		obj = SlotValue(o,i); \
		i = 0;\
	}

#define JDATA(x,l)	new_jdata(x,l)
#define JDATA_INT(x)	strtol(x->data,NULL,0)
#define JDATA_NUM(x)	strtod(x->data,NULL)
#define JDATA_OBJ(x)	jdata_toObject(x)
#define INT_JDATA(x)	int_jdata(x)
#define NUM_JDATA(x)	num_jdata(x)
#define OBJ_JDATA(x)	((JData)x)

