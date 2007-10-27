// Files.c
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "str.h"
#include "files.h"

str cwd = NULL;

File
open_file(File cache, str filename)
{
	File fc = (File)salloc(sizeof(struct file_cache_struct) + filename->len + 1);
	memcpy(fc->name,filename->data,filename->len+1);
	fc->fd = open(filename->data,O_RDONLY,0400);
	if (fc->fd < 0 || fstat(fc->fd,&fc->st)) {
		error("Failed to open file %s",filename);
		return NULL;
	}
	if (!(fc->data = mmap(NULL,fc->st.st_size,PROT_READ,MAP_FILE|MAP_SHARED,fc->fd,0))) {
		error("Failed to memory map file %s",filename);
		close(fc->fd);
		return NULL;
	}
	fc->next = cache;
	fc->count = 0;
	return fc;
}

File
reopen_file(File fc) 
{
	munmap(fc->data,fc->st.st_size);
	close(fc->fd);
	fc->fd = open(fc->name,O_RDONLY,0400);
	if (fc->fd < 0 || fstat(fc->fd,&fc->st)) {
		error("Failed to open file %c",fc->name);
		return NULL;
	}
	if (!(fc->data = mmap(NULL,fc->st.st_size,PROT_READ,MAP_FILE|MAP_SHARED,fc->fd,0))) {
		error("Failed to memory map file %c",fc->name);
		close(fc->fd);
		return NULL;
	}
	return fc;
}

File
close_file(File fc, str filename)
{
	File tmp, last = NULL;		
	for (tmp = fc; tmp; tmp = tmp->next) {
		if (!strcmp(filename->data,tmp->name)) {
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
query_cache(File* cache, str filename)
{
	File tmp, last = NULL, prior = NULL;
	for (tmp = *cache; tmp; tmp = tmp->next) {
		if (!strcmp(tmp->name,filename->data)) {
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

File
query_fd_cache(File cache, int fd)
{
	File tmp;
	for (tmp = cache; tmp; tmp = tmp->next)
		if (tmp->fd == fd)
			return tmp;
	return NULL;
}

void
set_cwd()
{
	cwd = char_str(getcwd(NULL,0),0);
}

str
temp_file()
{
	return Str("/tmp/%i%i",random(),time(NULL));
}

