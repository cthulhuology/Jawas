// memory.c
//
// Memory access routines
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "events.h"
#include "memory.h"

Region memory;

void
new_region()
{
	memory = (Region)mmap(NULL,getpagesize()*CACHE_PAGES,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANON,-1,0);
	memory->size = getpagesize() * CACHE_PAGES;
	memory->allocated = memory->size - sizeof(struct region_struct);
	memory->offset = 0;
}

size_t
advance(size_t length)
{
	memory->offset += length;
	return length;
}

char* 
reserve(size_t length)
{
	char* retval = &memory->data[memory->offset];
	advance(length);
	return retval;
}
