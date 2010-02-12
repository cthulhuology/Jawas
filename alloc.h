// alloc.h
// 
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_ALLOC_H__
#define __HAVE_ALLOC_H__

#include "include.h"

#define MAX_ALLOC_SIZE (getpagesize() - sizeof(struct scratch_struct))

typedef struct scratch_struct* Scratch;
struct scratch_struct  {
	Scratch next;
	int len;
	char data[0];
};

typedef struct scratch_info_struct ScratchInfo;
struct scratch_info_struct {
	size_t allocated;
	size_t frees;
};

extern ScratchInfo gsi;
extern Scratch gscratch;

Scratch new_scratch();
void free_scratch(Scratch s);
void set_scratch(Scratch s);
void adopt_scratch(Scratch dst, Scratch src);

char* alloc_scratch(Scratch s, int size);
char* salloc(int size);
void dump_scratch_info();

#endif
