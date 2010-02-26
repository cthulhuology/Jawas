// index.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "memory.h"
#include "str.h"
#include "log.h"
#include "index.h"
#include "headers.h"
#include "client.h"
#include "server.h"

static char* indexes[] = {
	"index.html",
	"index.shtml",
	"index.lws",
	"index.svg",
	"index.jws",
	"index.xml",
	NULL
};

int
is_directory(str filename)
{
	struct stat st;
	if (stat(filename->data,&st)) {
		error("Failed to stat file %s",filename);
		return 0;
	}
	debug("Is %s a Directory? %c",filename,st.st_mode & S_IFDIR ? "yes": "no");
	return st.st_mode & S_IFDIR;
}

int
is_file(str filename)
{
	struct stat st;
	if (stat(filename->data,&st)) {
		error("Failed to stat file %s",filename);
		return 0;
	}
	return st.st_mode & (S_IFREG|S_IFLNK);
}

str
deauth_path(str host, str filename)
{
	size_t l = len(filename);
	if (is_directory(file_path(host,filename)) || is_file(file_path(host,filename))) 
		return file_path(host,filename);
	for (int i = 1; i < l; ++i) {
		if (at(filename,i) == '/' && is_directory(file_path(host,ref(filename->data+i,l-i)))) {
			debug("Found auth token [%s]",ref(filename->data+1,i-1));
			append_header(client.request->headers,_("Token"),ref(filename->data+1,i-1));
			return file_path(host,ref(filename->data+i,l-i));
		}
	}
	return file_path(host,filename);
}

File
get_index(str filename)
{
	File retval = NULL;
	for (int i = 0; indexes[i]; ++i) {
		str index_path = _("%s%c",filename,indexes[i]);
		if ((retval = load(index_path))) return retval;
		index_path = _("%s/%c",filename,indexes[i]);
		if ((retval = load(index_path))) return retval;
	}
	return NULL;
}

