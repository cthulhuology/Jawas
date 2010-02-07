// memory.h
//

#ifndef __HAVE_MEMORY_H__
#define __HAVE_MEMORY_H__

char* new_region(unsigned long length);
void free_region(char* region);

#endif
