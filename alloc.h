// alloc.h
// 
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_ALLOC_H__
#define __HAVE_ALLOC_H__

#define MAX_ALLOC_SIZE (getpagesize() - sizeof(struct scratch_struct))

#define salloc(x)	alloc_scratch(gscratch,x)

typedef struct scratch_struct* Scratch;
struct scratch_struct  {
	Scratch next;
	int len;
	char data[0];
};

typedef struct scratch_info_struct ScratchInfo;
struct scratch_info_struct {
	size_t scratches;
	size_t frees;
	size_t max_scratches;	
	size_t allocs;
	size_t max_memory;
	size_t current;
};

extern ScratchInfo gsi;
extern Scratch gscratch;

Scratch new_scratch(Scratch n);
void free_scratch(Scratch s);
void set_scratch(Scratch s);

char* alloc_scratch(Scratch s, int size);
void dump_scratch_info();

#endif
