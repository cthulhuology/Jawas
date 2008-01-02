// str.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "alloc.h"
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
seek(str s, int pos)
{
	if (! s) return NULL;
	if (pos >= s->pos && pos < s->pos + s->length) 
		return s;
	return seek(s->next,pos);
}


str
set(str s, int pos, char c)
{
	str x = seek(s,pos);
	if (x) x->data[pos - x->pos] = c;
	return s;
}

char
at(str s, int pos)
{
	str x = seek(s,pos);
	return x ? x->data[pos - x->pos] : '\0';
}

int
len(str s)
{
	str t;
	int retval = 0;
	for (t = s; t; t = t->next) 
		retval += t->length;
	return retval;
}

str
copy(const char* a, int al)
{
	str t = NULL, retval = NULL;
	int l;
	for (l = al ? al : strlen(a) ; l > Max_Buffer_Size; l -= Max_Buffer_Size) {
		retval = (str)salloc(Max_Buffer_Size + sizeof(struct str_struct));	
		if (a) memcpy(retval->data,a + l - Max_Buffer_Size, Max_Buffer_Size);
		retval->length = Max_Buffer_Size;
		retval->pos = l - Max_Buffer_Size;
		retval->next = t;
		t = retval;
	}
	retval = (str)salloc(l + sizeof(struct str_struct));
	if (a) memcpy(retval->data,a,l);
	retval->pos = 0;
	retval->length = l;
	retval->next = t;
	return retval;
}

str
blank(int l)
{
	if (l == 0) return NULL;
	return copy(NULL,l);
}

str
clone(str s)
{
	if (!s) return NULL;
	str retval = (str)salloc(s->length + sizeof(struct str_struct));
	memcpy(retval->data,s->data,s->length);
	retval->length = s->length;
	retval->pos = s->pos;
	retval->next = clone(s->next);
	return retval;
}

str
replace(str src, int pos, str rep)
{
	int i,j, sl = len(src), rl = len(rep);
	for (i = pos, j = 0; i < sl && j < rl; ++i, ++j) 
		set(src,i,at(rep,j));
	return src;
}

str
from(str s, int start, int l)
{
	int i;
	str retval = blank(l);
	for (i = 0; i < l; ++i) 
		set(retval,i,at(s,start+i));	
	return retval;
}

str
append(str s, str v)
{
	str t;
	int pos;
	if (! s) return clone(v);
	for (t = s; t->next; t = t->next);
	t->next = clone(v); 
	pos = t->pos + t->length;
	for (t = t->next; t; t = t->next) {
		t->pos = pos;
		pos += t->length;
	}
	return s;
}

str
int_str(int i)
{
	int t, l;
	t = i / 10;
	for(l = (i < 0 ? 2 : 1); t; t = t / 10) ++l;
	str retval = blank(l);
	for (t = (i < 0 ? -i : i); l; t = t / 10) 
		set(retval, --l, t % 10 + '0');
	if (i < 0)
		set(retval, 0, '-');
	return retval;
}

str
hex_str(int i)
{
	int t, l;
	t = i / 10;	
	for(l = 1; t; t = t / 16) ++l;
	str retval = blank(l);
	for (t = i; l; t = t / 16) 
		set(retval,--l, between(10,t % 16,15) ?
			t % 16 - 10 + 'a':
			t % 16 + '0');	
	return retval;
}

str
obj_str(void* p)
{
	return hex_str((int)p);
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
		if (between('0',at(a,i),'9'))
			retval = retval * 16 + at(a,i) - '0'; 
		else if (between('a',at(a,i),'f')) 
			retval = retval * 16 + at(a,i) - 'a' + 10; 
		else if (between('A',at(a,i),'F')) 
			retval = retval * 16 + at(a,i) - 'A' + 10; 
		else
			return retval;
	return retval;
}

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
	int i, o = 0, a = 0, ls = 0, la = 0;
	for (i = 0; fmt[i]; ++i)
		la += (fmt[i] == '%' ? 1 : 0);
	str retval = NULL, params[la];
	ls = strlen(fmt) - 2*la;
	for (i = 0; fmt[i]; ++i)
		if (fmt[i] == '%')
			switch (fmt[++i]) {
			case 'c':
				params[a] = copy(va_arg(args,char*),0);
				ls += len(params[a++]);
				break;
			case 's':
				params[a] = va_arg(args,str);
				if (params[a])
					ls += len(params[a]);
				++a;
				break;
			case 'i':
				params[a] = int_str(va_arg(args,int));
				ls += len(params[a++]);
				break;
			case 'p':
				params[a] = obj_str(va_arg(args,char*));
				ls += len(params[a++]);
				break;
			case 'h':
				params[a] = hex_str(va_arg(args,int));
				ls += len(params[a++]);
				break;
			default:
				++ls;
			}
	retval = blank(ls);
	a = 0; 
	o = 0;
	for (i = 0; fmt[i]; ++i) {
		if (fmt[i] == '%' 
		&& (fmt[i+1] == 'c'
		||  fmt[i+1] == 's'
		||  fmt[i+1] == 'i'
		||  fmt[i+1] == 'p'
		||  fmt[i+1] == 'h')) {
			if (params[a]) {
				replace(retval,o,params[a]);
				o += len(params[a++]);
			}
			++i;
		} else {
			if (fmt[i] != '%') 
				set(retval,o++,fmt[i]);
		}
	}
	va_end(args);
	return retval;	
}

str
name_field(str line)
{
	int i, l = len(line);
	if (!line) return NULL;
	for (i = 0; i < l && at(line,i) != ':'; ++i);
	return i < l ? from(line, i, l - i) : line;
}

str 
skip_fields(str line, int n)
{
	int i, l = len(line);
	if (! line) return NULL;
	if (n == 0) {
		for(i = 0; i < l && at(line,i) != ':'; ++i);
		debug("Skipped to field %s",from(line, i, l - i));
		for (++i; i< l && isspace(at(line,i)); ++i);
		if (i < l)
			return from(line, i, l - i);
		return line;
	}
	for (i = 0; i < l && n; ++i) 
		if (at(line,i) == ',') 
			--n;
	return from(line, i, l - i);
}

str
dequote(str line)
{
	int i, j, l = len(line);
	for (i = 0; i < l && at(line,i) != '"'; ++i);
	for (j = i+1; j < l && at(line,j) != '"'; ++j);
	return from(line, i + 1, j - i - 1);
} 

str
singlequote(str s)
{
	int i,o = 0, l = len(s);
	str retval = NULL;
	for (i = 0; i < l; ++i) {
		if (at(s,i) == '\'') {
			retval = retval ? append(retval,from(s,o,i)) : append(retval,Str("%s''",from(s,o,i)));
			debug("Retval: [%s]", retval);
			o = i + 1;
		}
	}
	retval = retval ? append(retval,from(s,o,i)) : s;
	debug("Singlequote %s is %s",s,retval);
	return retval;
}

int
fncmp(str a, str b, int n, test_t func)
{
	int i;
	for (i = 0; i < n; ++i) 
		if (func(at(a,i),at(b,i)))
			return 0;
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
	if (between('a',x,'z')) 
		x = x - 'a' + 'A';
	if (between('a',y,'z'))
		y = y - 'a' + 'A';
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
find(str src, int pos, char* delim)
{
	int i,j,d,l = len(src);
	str t = seek(src,pos);
	while (t) {
		d = pos - t->pos;
		for (i = d; i < t->length; ++i)
			for (j = 0; delim[j]; ++j ) 
				if (delim[j] == t->data[i]) 
					return i + t->pos;
		if (i + t->pos >= l) return l;
		pos = t->pos + t->length;
		t = seek(src,pos);
	}
	return l;
}

int
search(str src, int pos, str key)
{
	int i;
	int sl = len(src);
	int kl = len(key);
	for (i = pos; i < sl; ++i) {
		if (cmp(from(src,i,kl),key))
			return i;
	}
	return sl;
}

str
read_line(str src, int pos)
{
	int eol = find(src,pos,"\r\n");
	return from(src,pos,eol-pos);
}

int
skip_headers(str src, int pos)
{
	str t;
	for (t = seek(src,pos); t; t = seek(src,pos)) {
		str line = read_line(src,pos);
		int ll = len(line);
		pos += ll + 2;
		if (ll == 0) break;
	}
	return pos;
}

str
dechunk(str src)
{
	int d = 0;
	str t, retval = NULL;
	int pos = skip_headers(src,0);
	for (t = seek(src,pos); t; t = seek(src,pos)) {
		str line = read_line(src,pos);	
		debug("Line is %s",line);
		d = str_int(Str("0x%s",line));
		debug("Buffer Delta is %i",d);
		retval = append(retval,from(src,pos+len(line)+2,d));
		pos += d + len(line) + 4;
		if (d == 0) {
			debug("Done reading");
			return retval;
		}
	}
	return retval;
}

char*
dump(str s)
{
	str t;
	int l = len(s);
	char* retval = (char*)calloc(1,l + 1);
	for (t = s; t; t = t->next) 	
		memcpy(retval + t->pos, t->data, t->length);
	retval[l] = '\0';
	return retval;
}
