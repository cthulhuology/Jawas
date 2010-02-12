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
	char data[4096 - sizeof(Page)];
};

typedef struct page_info_struct PageInfo;
struct page_info_struct {
	Page baseaddr;
	Page free;
	size_t size;
	size_t allocated;
	size_t frees;
};

extern PageInfo gpi;

Page new_page();
Page free_page(Page p);
void dump_page_info();
int free_memory();
int total_memory();
int alloced_memory();

#endif

