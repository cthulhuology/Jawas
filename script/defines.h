// defines.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

// Constants

#define DEFAULT_SLOTS	8
#define MAX_INT_SIZE	12
#define MAX_DOUBLE_SIZE	32
#define MAX_INT_CACHE	1024
#define MAX_JDATA_CACHE	4096

// Permissions

#define ReadOnly(x)	x && ((int)x & 1)
#define DontEnum(x)	x && ((int)x & 2)
#define DontDel(x)	x && ((int)x & 4)

#define R(x)	(int)x | 1
#define E(x)	(int)x | 2
#define D(x)	(int)x | 4 

#define RE(x) R(E(x))
#define RD(x) R(D(x))
#define ED(x) E(D(x))
#define RED(x) R(E(D(x)))

// Functions

#define FUNCTION_MASK	0xfffffffc
#define PRIM(x) (JData)((int)x | 1)
#define FUNC(x) (JData)((int)x | 2)
#define CALL(x) ((func_t)((int)x & FUNCTION_MASK))

#define isVALUE(x)	(int)x & 3 == 0
#define isPRIM(x)	(int)x & 3 = 1
#define isFUNC(x)	(int)x & 3 = 2

#define Func(x) JData x (JObject obj, JObject args)

// Constructors

#define JDATA(x,l)	new_jdata(x,l)
#define OBJECT(x)	new_object(DEFAULT_SLOTS + x)
#define OBJECT 		OBJECT(0)
#define ARRAY(x)	new_array(x)
#define ARGS(x)		new_arguments(x)

// Objects & Slots

#define PERM_MASK 0x7
#define PERM(x)		(int)x & PERM_MASK

#define PROPERTY_MASK 0xfffffff8
#define PROP(x) 	((JData)((int)x & PROPERTY_MASK))

#define Slot(o,i)	o->slots[i]
#define SlotPerm(o,i)	PERM(Slot(o,i).prop)
#define SlotProp(o,i)	PROP(Slot(o,i).prop)
#define SlotValue(o,i)	Slot(o,i).value
#define Perm(x)		SlotPerm(obj,x)
#define Prop(x)		SlotProp(obj,x)
#define Value(x)	SlotValue(obj,x)

#define Args(i)		SlotValue(args,i)
#define Argc		Length(args)

#define OBJECT_MASK	0x80000000
#define OBJECT_LEN_MASK	0x7fffffff
#define isOBJ(x)	x->len & OBJECT_MASK
#define isDATA(x)	x->len & OBJECT_MASK == 0

#define Length(o)	(isOBJ(o) ? o->len & OBJECT_LEN_MASK  : o->len)
#define ObjValue	obj_default_value(obj)

//  Iterators

#define ITERATE(o) \
	for (int i = 0; o && i < Length(o); ++i)

#define CHASE_PROTOTYPE(o,i) \
	if (!SlotProp(o,i) && SlotValue(o,i)) { \
		obj = SlotValue(o,i); \
		i = 0;\
	}

#define LOCALTIME_TMS \
	struct tm tms; \
	localtime_r(JDATA_INT(ObjValue),&tms);
	

#define SET_LOCALTIME_TMS \
	obj_put(obj,Value,INT_JDATA(mktime(&tms)));	
	return ObjValue;

// Conversions

#define JDATA_INT(x)	strtol(x->data,NULL,0)
#define JDATA_NUM(x)	strtod(x->data,NULL)
#define JDATA_OBJ(x)	jdata_toObject(x)
#define INT_JDATA(x)	int_jdata(x)
#define NUM_JDATA(x)	num_jdata(x)
#define OBJ_JDATA(x)	((JData)x)

// Special Atoms

#define Zero		&IntCache[0]
#define One		&IntCache[2]
#define Two		&IntCache[4]
