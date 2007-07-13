// pages.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

#include "defines.h"
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
		fprintf(stderr,"[JAWAS] Out of Memory!!!!\n");
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
			fprintf(stderr,"[JAWAS] Double free on page %p\n",p);
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
	fprintf(stderr,"[JAWAS] CacheInfo\n");
	fprintf(stderr,"\tBase Address: %p\n",gci.baseaddr);
	fprintf(stderr,"\tSize: %d bytes\n",gci.size);
	fprintf(stderr,"\tAllocated: %d\n",gci.allocated);
	fprintf(stderr,"\tFreed: %d\n",gci.freed);
	fprintf(stderr,"\tAllocations: %d\n",gci.allocations);
	fprintf(stderr,"\tFrees: %d\n",gci.frees);
}
