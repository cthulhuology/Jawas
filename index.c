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

static char* indexes[] = {
	"/index.html",
	"/index.shtml",
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
	free(fname);
	return st.st_mode & S_IFDIR;
}

str
get_index(str filename)
{
	struct stat st;
	int i;
	for (i = 0; indexes[i]; ++i) {
		str index_path = Str("%s%c",filename,indexes[i]);
		char *fname = dump(index_path);
		if (!stat(fname,&st)) {
			free(fname);
			return index_path;
		}
		free(fname);
	}
	return NULL;
}
