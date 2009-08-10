// Files.c
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "str.h"
#include "log.h"
#include "files.h"

str cwd = NULL;
int file_index = 0;

File
open_file(File cache, str filename)
{
	str t;
	int fl = len(filename);
	File fc = (File)salloc(sizeof(struct file_cache_struct) + fl + 1);
	for (t = filename; t; t = t->next)
		memcpy(fc->name + t->pos,t->data,t->length);
	fc->name[fl] = '\0';
	fc->fd = open(fc->name,O_RDONLY,0400);
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
	fc->parsed = NULL;
	fc->mime = NULL;
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
	int fl = len(filename);
	for (tmp = fc; tmp; tmp = tmp->next) {
		if (fl == strlen(tmp->name) && !strncmp(filename->data,tmp->name,fl)) {
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
	int fl = len(filename);
	File tmp, last = NULL, prior = NULL;
	for (tmp = *cache; tmp; tmp = tmp->next) {
		if (fl == strlen(tmp->name) && !strncmp(tmp->name,filename->data,fl)) {
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

int
mark_file(File fc, int i, int t, int o, int l)
{
	fc->parsed[i].kind = t;
	fc->parsed[i].offset = o;
	fc->parsed[i].length = l;
	return ++i;
}

File
parse_file(File fc)
{
	char* script = fc->data;
	int o = 0, l = 0, i = 0, e = 0;
	if (fc->parsed) return fc;
	fc->parsed = (Parsed)salloc(MAX_ALLOC_SIZE);
	memset(fc->parsed,0,MAX_ALLOC_SIZE);
	for (o = 0; script[o] && o < fc->st.st_size; ++o) {
		if (!strncmp(&script[o],"<?js",4)) {
			if (l < o) i = mark_file(fc,i,TEXT,l,o-l);
			l = 0;
			e = script[o+4] == '=' ? 1 : 0;
			while (strncmp(&script[o+l+5+e],e ? "=?>" : "?>",2+e)) ++l;
			i = mark_file(fc,i,SCRIPT+e,o+5+e,l);
			o += l + 5 + e;
			l = o+2+e;
		}
	}
	mark_file(fc,i,TEXT,l,o-l);
	return fc;
}

void
set_cwd()
{
	cwd = copy(getcwd(NULL,0),0);
}

str
temp_file()
{
	return Str("/tmp/%i%i",++file_index,time(NULL));
}

