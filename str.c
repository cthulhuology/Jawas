// str.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "memory.h"
#include "str.h"
#include "log.h"

int
min(int x, int y) 
{ 
	return x < y ? x : y;
}

int 
max(int x, int y)
{
	return x > y ? x : y;
}

int 
between(int a, int x, int b)
{
	return x >= a && x <= b;
}

str
set(str s, int pos, char c)
{
	if (!s) return NULL;
	s->data[pos] = c;
	return s;
}

char
at(str s, int pos)
{
	return s && pos < s->length ? s->data[pos] : '\0';
}

int
len(str s)
{
	return s ? s->length : 0;
}

str
ref(const char* a, int al)
{
	str retval = (str)reserve(sizeof(struct str_struct));
	retval->data = (char*)a;
	retval->length = al;
	return retval;
}

str
copy(const char* a, int al)
{
	if (!a) return NULL;
	str retval = blank(al);
	memcpy(retval->data,a,al);
	return retval;
}

str
blank(int l)
{
	str retval = (str)reserve(sizeof(struct str_struct) + l);
	retval->data = &retval->contents[0];
	retval->length = l;
	return retval;
}

str
clone(str s)
{
	return copy(s->data,s->length);
}

str
replace(str src, int pos, str rep)
{
	if (src->length > pos + rep->length) memcpy(&src->data[pos],rep->data,rep->length);
	return src;
}

str
from(str s, int start, int l)
{
	str retval = blank(l);
	memcpy(retval->data,s->data + start,l);
	return retval;
}

str
append(str s, str v)
{
	if (!s) return v;
	if ((str)&s->data[s->length] == v) {
		fprintf(stderr,"appending inline\n");
		s->length += v->length;
		memcpy((char*)v,v->data,v->length);
		return s;
	}
	str retval = blank(s->length + v->length);
	memcpy(retval->data,s->data,s->length);
	memcpy(retval->data + s->length,v->data,v->length);
	return retval;
}

size_t 
int_len(uint64_t i)
{
	uint64_t t, l;
	t = i / 10;
	for (l = (i < 0 ? 2 : 1); t; t = t / 10) ++l;
	return l;
}

void
int_print(char* data, int i, int l)
{
	for (int t = (i < 0 ? -i : i); l; t = t / 10) data[--l] =  t % 10 + '0';
}

str
int_str(uint64_t i)
{
	uint64_t l = int_len(i);
	str retval = blank(l);
	int_print(retval->data,i,l);
	return retval;
}

uint64_t
hex_len(uint64_t i)
{
	uint64_t t, l;
	t = i / 16;	
	for(l = 1; t; t = t / 16) ++l;
	return l;
}

void
hex_print(char* data, uint64_t i, uint64_t l)
{
	for (uint64_t t = i; l; t = t >> 4) 
		data[--l] =  between(10,t & 0x0f,15) ?
			(0x0f & t) + 'a' - 10:
			(t & 0x0f) + '0';	
}

str
hex_str(uint64_t i)
{
	uint64_t l = hex_len(i);
	str retval = blank(l);
	hex_print(retval->data,i,l);
	return retval;
}

str
obj_str(void* p)
{
	return hex_str((uint64_t)p);
}

int
str_int(str a)
{
	int i = 0,l = len(a);
	int retval = 0;
	if (at(a,0) == '-') i = 1;
	for (; i < l; ++i)  {
		if (! between('0',at(a,i),'9'))
			return retval;
		retval = retval * 10 + (at(a,i) - '0'); 
	}
	if (at(a,0) == '-') retval = -retval;
	return retval;
}

int
str_hex(str a)
{
	int i;
	int retval = 0;
	for (i = 0; i < len(a); ++i) 
		retval = (between('0',at(a,i),'9')) ?
			retval * 16 + at(a,i) - '0' :
		(between('a',at(a,i),'f')) ?
			retval * 16 + at(a,i) - 'a' + 10 :
		(between('A',at(a,i),'F')) ?
			retval * 16 + at(a,i) - 'A' + 10 :
			retval;
	return retval;
}

str
$(const char* fmt, ...)
{
	va_list args;
	va_start(args,fmt);
	return new_str(fmt,args);
}

str
new_str(const char* fmt, va_list args)
{
	char* c;
	str s;
	uint64_t x, xl, sl;
	uint64_t i, ls = 0, la = 0;
	for (i = 0; fmt[i]; ++i) la += (fmt[i] == '%' ? 1 : 0);
	str retval = blank(0);
	ls = 0;
	for (i = 0; fmt[i]; ++i) {
		if (fmt[i] == '%')
			switch (fmt[++i]) {
			case 'c':
				c = va_arg(args,char*);
				sl = strlen(c);
				memcpy(&retval->data[ls],c,sl);
				ls += sl;
				break;
			case 's':
				s = va_arg(args,str);
				memcpy(&retval->data[ls],s->data,s->length);
				ls += s->length;
				break;
			case 'd':
			case 'i':
				x = va_arg(args,uint64_t);
				xl = int_len(x);
				int_print(&retval->data[ls],x,xl);
				ls += xl;
				break;
			case 'p':
			case 'x':
				c = va_arg(args,char*);
				xl = hex_len((uint64_t)c);
				hex_print(&retval->data[ls],(uint64_t)c,xl);
				ls += xl;
				break;
			case 'h':
				x  = va_arg(args,uint64_t);
				xl = hex_len(x);
				hex_print(&retval->data[ls],x,xl);
				ls += xl;
				break;
			default:
				++ls;
			}
		else retval->data[ls++] = fmt[i];
	}
	retval->length = ls;
	advance(ls);
	va_end(args);
	return retval;	
}

str
name_field(str line)
{
	if (!line) return NULL;
	int i, l = len(line);
	for (i = 0; i < l && at(line,i) != ':'; ++i);
	return i < l ? ref(line->data,i) : line;
}

str 
skip_fields(str line, int n)
{
	int i, l = len(line);
	if (! line) return NULL;
	if (n == 0) {
		for(i = 0; i < l && at(line,i) != ':'; ++i);
		for (++i; i< l && isspace(at(line,i)); ++i);
		return (i < l) ? ref(&line->data[i], l - i) : line;
	}
	for (i = 0; i < l && n; ++i) if (at(line,i) == ',') --n;
	return ref(&line->data[i], l - i);
}

str
dequote(str line)
{
	int i, j, l = len(line);
	for (i = 0; i < l && at(line,i) != '"'; ++i);
	for (j = i+1; j < l && at(line,j) != '"'; ++j);
	return from(line, i + 1, j - i - 1);
}

int
fncmp(str a, str b, int n, test_t func)
{
	for (int i = 0; i < n; ++i) if (func(at(a,i),at(b,i))) return 0;
	return 1;
}
	
int 
not_equal(char x, char y) 
{
	return x != y;	
}	

int
ncmp(str a, str b, int n)
{
	return fncmp(a,b,n,(test_t)not_equal);
}

int
cmp(str a, str b)
{
	int l = len(a);
	if (l != len(b)) return 0;
	return fncmp(a,b,l,(test_t)not_equal);
}

int 
inot_equal(char x, char y) 
{
	if (between('a',x,'z')) x = x - 'a' + 'A';
	if (between('a',y,'z')) y = y - 'a' + 'A';
	return x != y;
}

int
icmp(str a, str b)
{
	int l = len(a);
	if (l != len(b)) return 0;
	return fncmp(a,b,l,(test_t)inot_equal);
}


int
is_less(char x, char y)
{
	return x < y;	
}

int
lesser(str a, str b)
{
	int i;
	int la = len(a);
	int lb = len(b);
	for (i = 0; i < min(la,lb); ++i) {
		if (at(a,i) - at(b,i) < 0) return 1;
		if (at(a,i) - at(b,i) > 0) return 0;
	}
	return la < lb;
}

int
find(str src, int pos, char* delim, size_t dl)
{
	int i;
	dl = dl ? dl : strlen(delim);
	for (i = pos; i < src->length; ++i)
		for (int j = 0; j < dl; ++j) 
			if (*(src->data+i) == delim[j])
				return i;
	return i;
}

int
search(str src, int pos, str key)
{
	return find(src,pos,key->data,key->length);
}

str
read_line(str src, int pos)
{
	int eol = find(src,pos,"\r\n",2);
	return ref(&src->data[pos],eol-pos);
}

int
skip_headers(str src, int pos)
{
	str line;
	int ll;
	int p = pos;
	do {
		line = read_line(src,pos);
		ll = len(line);
		p += ll + 2;
	} while (ll != 0 && pos < src->length);
	return pos;
}

str
dechunk(str src)
{
	int d = 0;
	int pos = skip_headers(src,0);
	str line = read_line(src,pos);	
	d = str_hex(line);
	return ref(&src->data[pos+len(line)+2],d);
}
