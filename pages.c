// pages.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "pages.h"

CacheInfo gci = { NULL, 0, 0, 0, 0, 0, NULL };

Page
new_page()
{
	Page tmp;
	int i;	
	if (!gci.free) {
		gci.baseaddr = mmap(NULL,getpagesize() * CACHE_PAGES,PROT_READ|PROT_WRITE,MAP_ANON,-1,0);
		if (gci.baseaddr == (char*)-1) return NULL;
		gci.size = CACHE_PAGES * getpagesize();
		gci.allocated = 0;
		gci.freed = 0;
		gci.allocations = 0;
		gci.frees = 0;
		tmp = gci.free = (Page)gci.baseaddr;
		for (i = 0; i < CACHE_PAGES - 1; ++i) {
			tmp->next = (Page)(gci.baseaddr+((1+i)*getpagesize()));
			tmp = (Page)(gci.baseaddr + ((1 + i) * getpagesize()));
		}
		tmp->next = NULL;
		dump_cache_info();
	}
	if (gci.free == NULL) {
		error("[JAWAS] Out of Memory!!!!\n");
		return NULL;
	}
	++gci.allocated;
	++gci.allocations;
	--gci.freed;
	tmp = gci.free;
	gci.free = gci.free->next;
	return tmp;
}

void
free_page(Page p)
{
	Page tmp;
	for (tmp = gci.free; PAGE_GUARD && tmp; tmp = tmp->next) {
		if (tmp == p) {
			debug("[JAWAS] Double free on page %i\n",p);
			for(;;) {}
		}		
	}
	++gci.frees;
	++gci.freed;
	--gci.allocated;
	p->next = gci.free;
	gci.free = p;	
}

void
dump_cache_info()
{
	notice("[JAWAS] CacheInfo\n");
	notice("\tBase Address: %i\n",gci.baseaddr);
	notice("\tSize: %i bytes\n",gci.size);
	notice("\tAllocated: %i\n",gci.allocated);
	notice("\tFreed: %i\n",gci.freed);
	notice("\tAllocations: %i\n",gci.allocations);
	notice("\tFrees: %i\n",gci.frees);
}
