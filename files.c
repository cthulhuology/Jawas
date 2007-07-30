// Files.c
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "alloc.h"
#include "files.h"

File
open_file(File cache, char* filename)
{
	File fc = (File)salloc(sizeof(struct file_cache_struct) + strlen(filename) + 1);
	memcpy(fc->name,filename,strlen(filename)+1);
	fc->fd = open(filename,O_RDONLY,0400);
	if (fc->fd < 0 || fstat(fc->fd,&fc->st)) return NULL;
	if (!(fc->data = mmap(NULL,fc->st.st_size,PROT_READ,MAP_FILE|MAP_SHARED,fc->fd,0))) {
		close(fc->fd);
		return NULL;
	}
	fc->next = cache;
	fc->count = 0;
	return fc;
}

File
close_file(File fc, char* filename)
{
	File tmp, last = NULL;		
	for (tmp = fc; tmp; tmp = tmp->next) {
		if (!strcmp(filename,tmp->name)) {
			if (last) {
				last->next = tmp->next;
				last = fc;
			} else {
				last = tmp->next;	
			}
			break;
		}
		last = tmp;
	} 
	if (tmp) {
		munmap(tmp->data,tmp->st.st_size);
		close(tmp->fd);
	}
	return last;
}

File
query_cache(File* cache, char* filename)
{
	File tmp, last = NULL, prior = NULL;
	for (tmp = *cache; tmp; tmp = tmp->next) {
		if (!strcmp(tmp->name,filename)) {
			if (last && last->count < tmp->count + 1) {
				if (prior) { 
					prior->next = tmp;
				} else {
					*cache = tmp;
				}
				last->next = tmp->next;
				tmp->next = last;
			}
			tmp->count += 1;
			return tmp;
		}
		prior = last;
		last = tmp;
	}
	return NULL;
}

