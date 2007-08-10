// str.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "alloc.h"
#include "str.h"

cstr
Cstr(const char* a, int l)
{
	cstr retval = (cstr)salloc(sizeof(struct cstr_struct));
	retval->len = l;
	retval->data = a;		
	return retval;
}

str
char_str(const char* a, int l)
{
	int len = l ? l :  strlen(a);
	str retval = (str)salloc(1+len+sizeof(struct str_struct));
	retval->len = len;
	memcpy(retval->data,a,len);
	retval->data[len] = '\0';
	return retval;
}

type_str_func(int_str,int,"%d");
type_str_func(obj_str,void*,"%p");
type_str_func(num_str,double,"%g");

str
Str(char* fmt, ...)
{
	va_list args;
	va_start(args,fmt);
	return new_str(fmt,args);
}

str
new_str(char* fmt, va_list args)
{
	cstr x;
	int i, o = 0, a = 0, ls = 0, la = 0;
	for (i = 0; fmt[i]; ++i)
		la += (fmt[i] == '%' ? 1 : 0);
	str tmp, retval = NULL, params[la];
	ls = strlen(fmt) - 2*la;
	for (i = 0; fmt[i]; ++i)
		if (fmt[i] == '%')
			switch (fmt[++i]) {
			case 'x':
				x = va_arg(args,cstr);
				params[a] = char_str(x->data,x->len);
				ls += params[a++]->len;
				break;
			case 'c':
				params[a] = char_str(va_arg(args,char*),0);
				ls += params[a++]->len;
				break;
			case 's':
				params[a] = va_arg(args,str);
				ls += params[a++]->len;
				break;
			case 'i':
				params[a] = int_str(va_arg(args,int));
				ls += params[a++]->len;
				break;
			case 'p':
				params[a] = obj_str(va_arg(args,char*));
				ls += params[a++]->len;
				break;
			case 'n':
				params[a] = num_str(va_arg(args,int));
				ls += params[a++]->len;
				break;
			default:
				++ls;
			}
	retval = (str)salloc(sizeof(struct str_struct) + ls+1);	
	retval->len = ls;
	a = 0; 
	o = 0;
	for (i = 0; fmt[i]; ++i) {
		if (fmt[i] == '%' 
		&& (fmt[i+1] == 'c'
		||  fmt[i+1] == 's'
		||  fmt[i+1] == 'i'
		||  fmt[i+1] == 'p'
		||  fmt[i+1] == 'n')) {
			memcpy(&retval->data[o],params[a]->data,params[a]->len);
			o += params[a++]->len;
			++i;
		} else {
			if (fmt[i] != '%') 
				retval->data[o++] = fmt[i];
		}
	}
	va_end(args);
	retval->data[retval->len] = '\0';
	return retval;	
}
