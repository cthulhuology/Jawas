// script.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __SCRIPT_H__
#define __SCRIPT_H__

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
