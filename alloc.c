// alloc.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "pages.h"
#include "alloc.h"

ScratchInfo gsi = { 0, 0, 0, 0, 0, 0 };
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
	++gsi.scratches;
	gsi.max_scratches = max(gsi.scratches,gsi.max_scratches);
	return retval;		
}

char*
alloc_scratch(Scratch s, int size)
{
	char* retval = NULL;
	if (!size) return NULL;
	if (size > MAX_ALLOC_SIZE) {
		error("alloc_scratch %i exceeds MAX_ALLOC_SIZE",size);
		dump_scratch_info();
		for (;;) {}
		return NULL;
	}
	if (size > MAX_ALLOC_SIZE - s->len) {
		s->next = new_scratch(s->next);	
		return alloc_scratch(s->next,size);
	}
	retval = &s->data[s->len];
	memset(retval,0,size);
	s->len += size;
	++gsi.allocs;
	gsi.current += size;
	gsi.max_memory = max(gsi.current,gsi.max_memory);
	return retval;
}

void
free_scratch(Scratch s)
{
	Scratch tmp;
	if (!s) return;
	if (s->next) free_scratch(s->next);
	gsi.current -= s->len;
	free_page((Page)s);
	++gsi.frees;
	--gsi.scratches;
}

void
dump_scratch_info()
{
	notice("****************************************");
	notice("ScratchInfo");
	notice("Scratches %i",gsi.scratches);
	notice("Frees %i",gsi.frees);
	notice("Max Scratches: %i",gsi.max_scratches);
	notice("Allocs %i",gsi.allocs);
	notice("Max Memory %i",gsi.max_memory);
	notice("Current Memory %i",gsi.current);
	notice("****************************************");
}
