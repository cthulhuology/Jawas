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

int
is_directory(str filename)
{
	struct stat st;
	if (stat(filename->data,&st)) {
		error("Failed to stat file %s",filename);
		return 0;
	}
	debug("IS_DIRECTORY Filename %s[%i] is dir ? %c",filename,filename->len, (st.st_mode&S_IFDIR ? "true" : "false"));
	return st.st_mode & S_IFDIR;
}

str
get_index(str filename)
{
	struct stat st;
	int i;
	for (i = 0; indexes[i]; ++i) {
		str index_path = Str("%s%c",filename,indexes[i]);
		if (!stat(index_path->data,&st)) 
			return index_path;
	}
	return NULL;
}
