// index.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "index.h"

int
is_directory(char* filename)
{
	struct stat st;
	stat(filename,&st);
	return st.st_mode & S_IFDIR;
}


const str indexes[] = {
	{ 11, "/index.html" },
	{ 10, "/index.jws" },
	{ 10, "/index.xml" },
	{ 0, NULL },
};

char*
get_index(char* filename)
{
	struct stat st;
	int i;
	int l = strlen(filename);
	char* buffer = (char*)malloc(l + MAX_INDEX_LEN);
	memcpy(buffer,filename,l);
	for (i = 0; indexes[i].data; ++i) {
		memcpy(buffer+l, indexes[i].data, indexes[i].len);
		buffer[l+indexes[i].len] = '\0';
		if (!stat(buffer,&st)) return buffer;
	}
	free(buffer);
	return NULL;
}
