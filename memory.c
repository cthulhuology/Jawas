// memory.c
//
// Memory access routines
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "events.h"
#include "memory.h"

int region_index = 0;
struct region_list_struct region_list[MAX_REGIONS], system_region;

void
new_region()
{
	region_index = (region_index+1) % MAX_REGIONS;
	region_list[region_index].size = getpagesize() * CACHE_PAGES;
	region_list[region_index].offset = 0;
	gettimeofday(&region_list[region_index].ttl,NULL);
}

void
init_regions()
{
	for (int i = 0; i < MAX_REGIONS; ++i) {
		new_region();
		region_list[region_index].address = (Region)mmap(NULL,region_list[region_index].size,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANON,-1,0);
		if (region_list[region_index].address == (Region)-1) exit(1);
	}
	system_region.size = SYSTEM_REGION_SIZE;
	system_region.offset = 0;
	gettimeofday(&system_region.ttl,NULL);
	system_region.address = (Region)mmap(NULL, system_region.size,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANON,-1,0);
	if (system_region.address == (Region)-1) exit(2);
}

char* 
reserve(size_t length)
{
	if ((char*)region_list[region_index].size < &region_list[region_index].address->data[region_list[region_index].offset + length])
		new_region();
	if (&region_list[region_index].address->data[region_list[region_index].offset + length] > 
		&region_list[region_index].address->data[region_list[region_index].size]) 
		halt;
	char* retval = &region_list[region_index].address->data[region_list[region_index].offset];
	region_list[region_index].offset += length;
	return retval;
}

char*
system_reserve(size_t length)
{
	if (&system_region.address->data[system_region.offset + length] > 
		&system_region.address->data[system_region.size]) 
		halt;
	char* retval = &system_region.address->data[system_region.offset];
	system_region.offset += length;
	return retval;
}

