// pages.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "pages.h"

PageInfo gpi = { NULL, NULL, 0, 0, 0 };

Page
new_page()
{
	Page tmp;
	int i;	
	if (!gpi.free) {
		gpi.baseaddr = (Page)mmap(NULL,getpagesize() * CACHE_PAGES,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANON,-1,0);
		if (gpi.baseaddr == (Page)-1) return NULL;
		gpi.size = CACHE_PAGES * getpagesize();
		gpi.allocated = 0;
		gpi.frees = 0;
		tmp = gpi.free = (Page)gpi.baseaddr;
		for (i = 0; i < CACHE_PAGES - 1; ++i) {
			tmp->next = &gpi.baseaddr[1+i];
			tmp = tmp->next;
		}
		tmp->next = NULL;
	}
	if (gpi.free == NULL) {
		fprintf(stderr,"gpi.free is NULL!");
		exit(1);
	}
	++gpi.allocated;
	tmp = gpi.free;
	gpi.free = gpi.free->next;
	return tmp;
}

int
free_page(Page p)
{
	Page tmp;
	for (tmp = gpi.free; PAGE_GUARD && tmp; tmp = tmp->next) {
		if (tmp == p) {
			fprintf(stderr,"Double free on page %p\n",p);
			return 0;
		}
	}
	++gpi.frees;
	p->next = gpi.free;
	gpi.free = p;	
	return 1;
}

int
free_memory() 
{
	int total = 0;
	Page tmp;
	for (tmp = gpi.free; tmp; tmp = tmp->next) ++total;
	return total * getpagesize();
}

int
total_memory()
{
	return gpi.size;
}

int
alloced_memory()
{
	return total_memory() - free_memory();
}

void
dump_page_info()
{
	notice("****************************************");
	notice("PageInfo\n");
	notice("\tBase Address: %p\n",gpi.baseaddr);
	notice("\tSize: %i bytes\n",gpi.size);
	notice("\tAllocated: %i\n",gpi.allocated);
	notice("\tFrees: %i\n",gpi.frees);
	notice("****************************************");
}
