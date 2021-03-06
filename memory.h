// memory.h
//

#ifndef __HAVE_MEMORY_H__
#define __HAVE_MEMORY_H__

#include "config.h"

typedef struct region_struct* Region;
struct region_struct {
	size_t size;
	size_t allocated;
	size_t offset;
	char data[0];	
};

void new_region();
char* reserve(size_t length);
size_t advance(size_t length);

#endif
