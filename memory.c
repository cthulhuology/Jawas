// memory.c
//
// Memory access routines
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "memory.h"


struct region_list_struct {
	char* address;
	size_t size;
} region_list[MAX_REGIONS];
int region_index = 0;

char*
new_region(size_t length)
{
	if (region_index >= MAX_REGIONS) {
		error("Out of regions");
		halt;
	}
	length = getpagesize() * (1 + (length / getpagesize()));
	char* buffer = (char*)mmap(NULL,length,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANON,-1,0);
	if (buffer == (char*)-1) return NULL;
	region_list[region_index].address = buffer;
	region_list[region_index++].size = length;
	return buffer;
}

void
free_region(char* region)
{
	for (int i = 0; i < region_index; ++i)
		if (region >= region_list[i].address
		&& region < region_list[i].size + region_list[i].address) {
			munmap(region_list[i].address,region_list[i].size);
			if ( region_index-i > 1)
				memcpy(&region_list[i],&region_list[i+1],sizeof(struct region_list_struct)* region_index-i);
			--region_index;
			return;
		}
}
