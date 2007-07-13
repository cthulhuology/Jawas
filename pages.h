// pages.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __PAGES_H__
#define __PAGES_H__

typedef struct page_struct* Page;
struct page_struct {
	Page next;
	char data[0];
};

typedef struct cache_info_struct CacheInfo;
struct cache_info_struct {
	char* baseaddr;
	size_t size;
	size_t allocated;
	size_t freed;			
	size_t allocations;
	size_t frees;
	Page free;
	int guard;
};

extern CacheInfo gci;

Page new_page();
void free_page(Page p);
void dump_cache_info();

#endif

