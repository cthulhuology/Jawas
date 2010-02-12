// alloc.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "pages.h"
#include "alloc.h"

ScratchInfo gsi = { 0, 0 };
Scratch gscratch;

void
set_scratch(Scratch s)
{
	gscratch = s;
}

Scratch
new_scratch()
{
	Scratch retval = (Scratch)new_page();	
	retval->next = NULL;
	retval->len = 0;
	++gsi.allocated;
	return retval;		
}

char*
alloc_scratch(Scratch s, int size)
{
	Scratch t;
	char* retval = NULL;
	if (!size) return NULL;
	if (size > MAX_ALLOC_SIZE) {
		error("alloc_scratch %i exceeds MAX_ALLOC_SIZE",size);
		halt;
		return NULL;
	}
	for (t = s; size > MAX_ALLOC_SIZE - t->len && t->next; t = t->next); 
	if ( size > MAX_ALLOC_SIZE - t->len) t->next = new_scratch();
	retval = &t->data[t->len];
	memset(retval,0,size);
	t->len += size;
	return retval;
}

char*
salloc(int size)
{
	return alloc_scratch(gscratch,size);
}

void
adopt_scratch(Scratch dst, Scratch src)
{
	Scratch c;
	for (c = dst; c && c->next; c = c->next);
	c->next = src;
}

void
free_scratch(Scratch s)
{
	Scratch t;
	for (t = s; t; t = (Scratch)free_page((Page)t));
	++gsi.frees;
}

void
dump_scratch_info()
{
	notice("****************************************");
	notice("ScratchInfo");
	notice("Allocs %i",gsi.allocated);
	notice("Frees %i",gsi.frees);
	notice("****************************************");
}
