// index.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "str.h"
#include "log.h"
#include "index.h"
#include "headers.h"
#include "server.h"

static char* indexes[] = {
	"/index.html",
	"/index.shtml",
	"/index.lws",
	"/index.svg",
	"/index.jws",
	"/index.xml",
	NULL
};

int
is_directory(str filename)
{
	struct stat st;
	char* fname = dump(filename);
	if (stat(fname,&st)) {
		error("Failed to stat file %s",filename);
		return 0;
	}
	free_region(fname);
	return st.st_mode & S_IFDIR;
}

int
is_file(str filename)
{
	struct stat st;
	char* fname = dump(filename);
	if (stat(fname,&st)) {
		error("Failed to stat file %s",filename);
		return 0;
	}
	free_region(fname);
	return st.st_mode & (S_IFREG|S_IFLNK);
}

str
deauth_path(str filename)
{
	size_t l = len(filename);
	if (at(filename,l-1) == '/') --l;
	if (is_directory(filename) || is_file(filename)) return filename;
	for (int i = l; i-- > 0;) {
		if (at(filename,i-1) == '/' && is_directory(from(filename,0,i))) {
			debug("Found auth token [%s]",from(filename,i,l-i));
			append_header(Req->headers,Str("Token"),from(filename,i,l-i));
			return from(filename,0,i);
		}
	}
	return filename;
}

str
get_index(str filename)
{
	struct stat st;
	for (int i = 0; indexes[i]; ++i) {
		str index_path = Str("%s%c",filename,indexes[i]);
		char *fname = dump(index_path);
		if (!stat(fname,&st)) {
			free_region(fname);
			return index_path;
		}
		free_region(fname);
	}
	return NULL;
}

