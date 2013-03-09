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
#include "mime.h"

#include <sys/mman.h>

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
	size_t page_size = sysconf(_SC_PAGESIZE);
	if (fc->parsed) return fc;
	fc->parsed = (Parsed)reserve(page_size);
	memset(fc->parsed,0,page_size);
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

File
open_file(str filename)
{
	int fl = len(filename);
	File fc = (File)reserve(sizeof(struct file_cache_struct) + fl + 1);
	memcpy(&fc->name.contents[0],filename->data,fl);	// Allocate a str struct by hand!
	fc->name.data = fc->name.contents;
	fc->name.length = fl;
	fc->name.data[fl] = '\0';
	fc->fd = open(fc->name.data,O_RDONLY,0400);
	if (fc->fd < 0 || fstat(fc->fd,&fc->st)) {
		error("Failed to open file %s",filename);
		return NULL;
	}
	if (!(fc->data = mmap(NULL,fc->st.st_size,PROT_READ,MAP_SHARED,fc->fd,0))) {
		error("Failed to memory map file %s",filename);
		close(fc->fd);
		return NULL;
	}
	add_file_monitor(fc->fd, filename);
	fc->parsed = NULL;
	fc->mime = NULL;
	if (parseable_file(fc)) parse_file(fc);
	return fc;
}

File
reload(int fd) 
{
	File fc = NULL;
	for (int i = 0; i < server.file_index; ++i)
		if (server.files[i]->fd == fd) 
			fc = server.files[i];
	if (! fc) return NULL;
	munmap(fc->data,fc->st.st_size);
	close(fc->fd);
	fc->fd = open(fc->name.data,O_RDONLY,0400);
	if (fc->fd < 0 || fstat(fc->fd,&fc->st)) {
		error("Failed to open file %c",fc->name.data);
		return NULL;
	}
	if (!(fc->data = mmap(NULL,fc->st.st_size,PROT_READ,MAP_SHARED,fc->fd,0))) {
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

void
set_cwd()
{
	char* tmp = getcwd(NULL,0);
	server.cwd = ref(tmp,strlen(tmp));
}

str
temp_file()
{
	return _("/tmp/%i%i",server.file_index,time(NULL));
}

File
load(str filename)
{
	if (!filename) return NULL;
	return query_cache(filename);
}

void
load_directory(str directory)
{
	char *path[2] = { directory->data, NULL };
	FTS* f = fts_open(path,FTS_LOGICAL,NULL);	// NULL is the comparator
	FTSENT* fe;
	debug("Opening directory %s", directory);
	for (fe = fts_read(f); fe; fe = fts_read(f)) {	// no option
		debug("Testing %c",  fe->fts_name);
		if (fe->fts_namelen > 0 && fe->fts_name[0] == '.') continue;
		if (fe->fts_info & FTS_F) {
			str filename = ref(fe->fts_accpath,strlen(fe->fts_accpath)); 
			debug("Opening file %s",filename);
			server.files[server.file_index++] = open_file(filename);
		}
	}
	fts_close(f);
}

void
load_files()
{
	set_cwd();
	load_directory(_("%s",server.cwd));
}

