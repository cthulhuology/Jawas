// memory.c
//
// © 2007,2013 David J. Goehrig
//
// Memory access routines
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "events.h"
#include "memory.h"
#include <sys/mman.h>

Region memory = NULL;

void
new_region()
{
	#ifdef _SC_PAGESIZE
	size_t page_size = sysconf(_SC_PAGESIZE);
	#else
	size_t page_size = 4096;
	#endif
	if (memory) {
		munmap(memory,memory->size);
		memory = NULL;
	}
	memory = (Region)mmap(NULL,page_size*CACHE_PAGES,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANON,-1,0);
	if (memory == MAP_FAILED) {
		perror("mmap");
		fprintf(stderr, "Failed to allocate memory\n");
		exit(0);
	}
	memory->size = page_size * CACHE_PAGES;
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
