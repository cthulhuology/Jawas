// alloc.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
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
new_scratch(Scratch n)
{
	Scratch retval = (Scratch)new_page();	
	retval->next = n;
	retval->len = 0;
	++gsi.allocated;
	return retval;		
}

char*
alloc_scratch(Scratch s, int size)
{
	char* retval = NULL;
	if (!size) return NULL;
	if (size > MAX_ALLOC_SIZE) {
		error("alloc_scratch %i exceeds MAX_ALLOC_SIZE",size);
		halt;
		return NULL;
	}
	if (size > MAX_ALLOC_SIZE - s->len) {
		s->next = new_scratch(s->next);	
		return alloc_scratch(s->next,size);
	}
	retval = &s->data[s->len];
	memset(retval,0,size);
	s->len += size;
	return retval;
}

void
free_scratch(Scratch s)
{
	if (!s) return;
	if (s->next) free_scratch(s->next);	
	if (free_page(s)) {
		++gsi.frees;
	}
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
