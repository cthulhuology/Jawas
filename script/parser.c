// parser.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "log.h"
#include "parser.h"

int lineno = 0;
str* reserved_words = NULL;
int* delim_frequency = NULL;

CONSTRUCTOR(exp,Exp);
CONSTRUCTOR(blk,Block);
CONSTRUCTOR(try,Try);
CONSTRUCTOR(ctch,Catch);
CONSTRUCTOR(ife,IfElse);
CONSTRUCTOR(swtch,Switch);
CONSTRUCTOR(cse,Case);
CONSTRUCTOR(wth,With);
CONSTRUCTOR(lbl,Label);
CONSTRUCTOR(itr,Iterator);
CONSTRUCTOR(obj,Object);
CONSTRUCTOR(func,Func);
CONSTRUCTOR(stmt,Stmt);

void
init_reserved()
{
	int i;
	for (i = 1; reserved_word_list[i]; ++i);
	reserved_words = (str*)salloc(sizeof(str)*(i+1));
	reserved_words[i] = NULL;
	for (i = 1; reserved_word_list[i]; ++i)
		reserved_words[i] = char_str(reserved_word_list[i],0);
}

void
init_delim_frequency()
{
	delim_frequency = (int*)salloc(sizeof(int) * strlen(delim_chars));
}

int
reserved(cstr buf)
{
	int i;
	for (i = 1; reserved_words[i]; ++i)
		if (!cmp_str(buf,reserved_words[i]))
			return i;
	return 0;
}

int
delim(char buf)
{
	int i;
	for (i = 1; delim_chars[i]; ++i)
		if (buf == delim_chars[i]) {
			++delim_frequency[i];
			return i;
		}
	return 0;		
}

int
eol(char buf)
{
	int i;
	for (i = 1; eol_chars[i]; ++i)
		if (eol_chars[i] == buf)
			return i;
	return 0;
}

int
start_block(cstr buf, char c)
{
	int i;
	for (i = 0; i < buf->len && buf->data[i] != c; ++i);
	return i;
}

cstr
skip(cstr buf, int off)
{
	int i;
	if (!off || !buf) return NULL;
	for (i = 0; i + off < buf->len && isspace(buf->data[i+off]); ++i);
	if (off + i >= buf->len) return NULL;
	return Cstr(buf->data + off + i, buf->len - (off+i));	
}

cstr 
parse_ident(cstr buf, cstr* ident)
{
	int l;
	for (l = 0; l < buf->len && !delim(buf->data[l]); ++l);
	*ident = Cstr(buf->data,l);	
	return skip(buf,l);
}

cstr 
parse_exp(cstr buf, exp* e)
{
	int l;
	for (l = 0; !delim(buf->data[l]); ++l);
	(*e)->object = NULL;
	if (l > 0) 
		(*e)->object = Cstr(buf->data,l);	
	cstr rem = skip(buf,l);
	(*e)->msg = reserved(rem);
	(*e)->args = NULL;
	rem = skip(rem,reserved_words[(*e)->msg]->len);	
	if (rem) {
		(*e)->args = Exp();
		return parse_exp(rem,&(*e)->args);
	}
	return NULL; 
}

cstr
parse_stmt(cstr buf, stmt* s)
{
	int l;
	for (l = 0; l < buf->len && !eol(buf->data[l]); ++l);
	(*s)->rep = Cstr(buf->data,l);
	(*s)->flag = UNKNOWN;
	return skip(buf,l);
}

cstr
parse_match(cstr buf, char a, char b)
{
	int l, count = 0;
	for (l = 0; buf->data[l]; ++l) {
		if (buf->data[l] == a) ++count;
		if (buf->data[l] == b) {
			--count;
			if (!count) 
				return Cstr(buf->data+1,l-1);
		}
	}
	debug("No matching ) at %i", lineno);
	return NULL;
}

cstr
parse_block(cstr buf, blk* b)
{
	blk tmp;
	cstr x,retval = NULL;
	if (!buf) return NULL;
	if (buf->data[0] == '{') {
		x = parse_match(buf,'{','}');
		debug("Block contents are %x",x);
		retval = skip(buf,x->len+2);
	} else x = buf;
	for (tmp = *b; x; tmp = tmp->tail) { 	
		tmp->tail = NULL;
		tmp->state = Stmt();
		x = parse_stmt(x,&tmp->state);	
		if (x) tmp->tail = Block();
	}
	return retval;
}

cstr
parse_parens(cstr buf, blk* b)
{
	cstr x = parse_match(buf,'(',')');
	parse_block(x,b);
	return skip(buf,x->len+2);
}

cstr
parse_index(cstr buf, exp* e)
{
	cstr x = parse_match(buf,'[',']');
	parse_exp(x,e);
	return skip(buf,x->len+2);
}

cstr
parse_catch(cstr buf, ctch* c)
{
	cstr ct,x;
	cstr rem = parse_ident(buf,&ct);
	if (!ct) return buf;
	int res = reserved(ct);
	if (! res || (res != 1 && res != 2))
		return buf;
	if (res == 2 && rem->data[0] != '{') 
		rem = parse_ident(rem,&(*c)->id);
	x = parse_match(rem,'{','}');
	parse_block(x, &(*c)->blk);	
	rem = skip(rem,x->len+2);
	(*c)->tail = NULL;
	if (rem)
		return parse_catch(rem,&(*c)->tail);
	return NULL;
}

cstr 
parse_try(cstr buf, try* t)
{
	cstr rem = NULL;
	(*t)->try = NULL;
	cstr x = skip(buf,start_block(buf,'{'));
	if (x) {
		(*t)->try = Block();
		rem = parse_block(x,&(*t)->try);
	}
	if (rem)
		return parse_catch(rem,&(*t)->catch);
	debug("try without catch at %i",lineno);
	return NULL;
}


cstr
parse_ife(cstr buf, ife* e)
{

}

cstr 
parse_switch(cstr buf, swtch* s)
{
}

cstr
parse_case(cstr buf, cse* c)
{
}

cstr parse_with(cstr buf, wth* w)
{
}

cstr parse_label(cstr buf, lbl* l)
{
}

cstr parse_do(cstr buf, itr* r)
{
}

cstr parse_while(cstr buf, itr* r)
{
}

cstr parse_for(cstr buf, itr* r)
{
}

cstr parse_object(cstr buf, obj* o)
{
}

cstr parse_list(cstr buf, obj* o)
{
}

cstr parse_function(cstr buf, func* f)
{
}

