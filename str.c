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

char*
str_char(str s)
{
	int i, j;
	char* retval = salloc(s->len);
	for (i = j = 0; i < s->len; ++i) 
		if (s->data[i])
			retval[j++] = s->data[i];
	retval[j] = '\0';
	return retval;
}
	
str
char_str(const char* a, int l)
{
	int len = l ? l : strlen(a);
	str retval = (str)salloc(1+len+sizeof(struct str_struct));
	retval->len = len;
	if (a) memcpy(retval->data,a,len);
	retval->data[len] = '\0';
	return retval;
}

type_str_func(int_str,int,"%d");
type_str_func(obj_str,void*,"%p");
type_str_func(num_str,double,"%g");
type_str_func(hex_str,int,"%x");

str
Str(const char* fmt, ...)
{
	va_list args;
	va_start(args,fmt);
	return new_str(fmt,args);
}

str
new_str(const char* fmt, va_list args)
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
				if (params[a])
					ls += params[a]->len;
				++a;
				break;
			case 'i':
				params[a] = int_str(va_arg(args,int));
				ls += params[a++]->len;
				break;
			case 'p':
				params[a] = obj_str(va_arg(args,char*));
				ls += params[a++]->len;
				break;
			case 'h':
				params[a] = hex_str(va_arg(args,int));
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
		||  fmt[i+1] == 'x'
		||  fmt[i+1] == 'i'
		||  fmt[i+1] == 'p'
		||  fmt[i+1] == 'h'
		||  fmt[i+1] == 'n')) {
			if (params[a]) {
				memcpy(&retval->data[o],params[a]->data,params[a]->len);
				o += params[a++]->len;
			}
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

int
lesser_str(str a, str b)
{
	int retval = strncmp(a->data,b->data,min(a->len,b->len));
	return  retval < 0 ? 1 : retval > 0 ? 0 : a->len < b->len;
}

str
name_field(str line)
{
	int i;
	for (i = 0; i < line->len && line->data[i] != ':'; ++i);
	return char_str(line->data, i);
}

str 
skip_fields(str line, int n)
{
	int i;
	if (n == 0) {
		for(i = 0; i < line->len && line->data[i] != ':'; ++i);
		debug("Skipped to field %s",char_str(line->data +i, line->len - i));
		for (++i;i< line->len && isspace(line->data[i]); ++i);
		if (i < line->len)
			return char_str(line->data + i, line->len - i);
		return line;
	}
	for (i = 0; i < line->len && n; ++i) 
		if (line->data[i] == ',') 
			--n;
	return char_str(line->data + i, line->len - i);
}

str
dequote(str line)
{
	int i, j;
	for (i = 0; i < line->len && line->data[i] != '"'; ++i);
	for (j = i+1; j < line->len && line->data[j] != '"'; ++j);
	return char_str(line->data + i + 1, j - i - 1);
} 

str
sub_str(str s, int start, int end)
{
	str retval =  char_str(s->data + start, end - start);
	debug("sub_str \"%s\"[%i,%i]",retval,start,end);
	return retval;
}

str
singlequote(str s)
{
	int i,o = 0;
	str retval = NULL;
	for (i = 0; i < s->len; ++i) {
		if (s->data[i] == '\'') {
			retval = retval ? Str("%s%s''",retval,sub_str(s,o,i)) : Str("%s''",sub_str(s,o,i));
			debug("Retval: [%s]", retval);
			o = i + 1;
		}
	}
	retval = retval ? Str("%s%s",retval,sub_str(s,o,i)) : s;
	debug("Singlequote %s is %s",s,retval);
	return retval;
}
