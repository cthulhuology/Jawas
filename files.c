// Files.c
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "memory.h"
#include "str.h"
#include "log.h"
#include "files.h"
#include "events.h"
#include "server.h"

str cwd = NULL;
int file_index = 0;

File files;

File
open_file(str filename)
{
	int fl = len(filename);
	File fc = (File)system_reserve(sizeof(struct file_cache_struct) + fl + 1);
	memcpy(fc->name.contents,filename->data,fl);	// Allocate a str struct by hand!
	fc->name.data = fc->name.contents;
	fc->name.length = fl;
	fc->name.data[fl] = '\0';
	fc->fd = open(fc->name.contents,O_RDONLY,0400);
	if (fc->fd < 0 || fstat(fc->fd,&fc->st)) {
		error("Failed to open file %s",filename);
		return NULL;
	}
	if (!(fc->data = mmap(NULL,fc->st.st_size,PROT_READ,MAP_FILE|MAP_SHARED,fc->fd,0))) {
		error("Failed to memory map file %s",filename);
		close(fc->fd);
		return NULL;
	}
	fc->parsed = NULL;
	fc->mime = NULL;
	return fc;
}

File
reload(File fc) 
{
	munmap(fc->data,fc->st.st_size);
	close(fc->fd);
	fc->fd = open(fc->name.data,O_RDONLY,0400);
	if (fc->fd < 0 || fstat(fc->fd,&fc->st)) {
		error("Failed to open file %c",fc->name.data);
		return NULL;
	}
	if (!(fc->data = mmap(NULL,fc->st.st_size,PROT_READ,MAP_FILE|MAP_SHARED,fc->fd,0))) {
		error("Failed to memory map file %c",fc->name.data);
		close(fc->fd);
		return NULL;
	}
	return fc;
}

void
close_files()
{
	for (int i = 0; i < server.file_index; ++i) {
		munmap(server.files[i]->data,server.files[i]->st.st_size);
		close(server.files[i]->fd);
	}
}

File
query_cache(str filename)
{
	for (int i = 0; i < server.file_index; ++i)
		if (cmp(filename,&server.files[i]->name))
			return server.files[i];
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
	fc->parsed = (Parsed)system_reserve(getpagesize());
	memset(fc->parsed,0,getpagesize());
	for (o = 0; script[o] && o < fc->st.st_size; ++o) {
		if (!strncmp(&script[o],"<?",2) && isspace(script[o+2])) {
			if (l < o) i = mark_file(fc,i,TEXT,l,o-l);
			l = 0;
			e = script[o+2] == '=' ? 1 : 0;
			while (o+l+3+e < fc->st.st_size && strncmp(&script[o+l+3+e],e ? "=?>" : "?>",2+e)) ++l;
			if (o+l+3+e < fc->st.st_size) {
				i = mark_file(fc,i,SCRIPT+e,o+3+e,l);
				o += l + 3 + e;
				l = o+2+e;
			}
		}
	}
	mark_file(fc,i,TEXT,l,o-l);
	return fc;
}

void
set_cwd()
{
	char* tmp = getcwd(NULL,0);
	cwd = ref(tmp,strlen(tmp));
}

str
temp_file()
{
	return $("/tmp/%i%i",++file_index,time(NULL));
}

File
load(str filename)
{
	if (!filename) return NULL;
	File retval = query_cache(filename);
	if (retval) return retval;
	retval = open_file(filename);
	if (!retval) {
		error("Failed to open %s\n",filename);
		return NULL;
	}
	add_file_monitor(retval->fd,retval);
	return retval;
}

