// memory.h
//

#ifndef __HAVE_MEMORY_H__
#define __HAVE_MEMORY_H__

#include "config.h"

typedef struct event_struct* Event;

typedef struct region_struct* Region;
struct region_struct {
	Event events[MAX_EVENTS];
	char data[0];	
}; 

struct region_list_struct {
	Region address;
	size_t size;
	size_t allocated;
	size_t offset;
	struct timeval ttl;
};

void new_region();
void init_regions();
char* reserve(size_t length);
char* system_reserve(size_t length);
size_t advance(size_t length);

#endif
