// pages.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "pages.h"

PageInfo gpi = { NULL, 0, 0, 0, 0, 0, NULL };

Page
new_page()
{
	Page tmp;
	int i;	
	if (!gpi.free) {
		gpi.baseaddr = mmap(NULL,getpagesize() * CACHE_PAGES,PROT_READ|PROT_WRITE,MAP_ANON,-1,0);
		if (gpi.baseaddr == (char*)-1) return NULL;
		gpi.size = CACHE_PAGES * getpagesize();
		gpi.allocated = 0;
		gpi.freed = 0;
		gpi.allocations = 0;
		gpi.frees = 0;
		tmp = gpi.free = (Page)gpi.baseaddr;
		for (i = 0; i < CACHE_PAGES - 1; ++i) {
			tmp->next = (Page)(gpi.baseaddr+((1+i)*getpagesize()));
			tmp = (Page)(gpi.baseaddr + ((1 + i) * getpagesize()));
		}
		tmp->next = NULL;
	}
	if (gpi.free == NULL) {
		fprintf(stderr,"gpi.free is NULL!");
		exit(1);
	}
	++gpi.allocated;
	++gpi.allocations;
	--gpi.freed;
	tmp = gpi.free;
	gpi.free = gpi.free->next;
	return tmp;
}

void
free_page(Page p)
{
	Page tmp;
	for (tmp = gpi.free; PAGE_GUARD && tmp; tmp = tmp->next) {
		if (tmp == p) {
			debug("Double free on page %i\n",p);
			for(;;) {}
		}		
	}
	++gpi.frees;
	++gpi.freed;
	--gpi.allocated;
	p->next = gpi.free;
	gpi.free = p;	
}

void
dump_page_info()
{
	notice("****************************************");
	notice("PageInfo\n");
	notice("\tBase Address: %p\n",gpi.baseaddr);
	notice("\tSize: %i bytes\n",gpi.size);
	notice("\tAllocated: %i\n",gpi.allocated);
	notice("\tFreed: %i\n",gpi.freed);
	notice("\tAllocations: %i\n",gpi.allocations);
	notice("\tFrees: %i\n",gpi.frees);
	notice("****************************************");
}
