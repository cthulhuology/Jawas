// Files.h
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __FILES_H__
#define __FILES_H__

#include <sys/types.h>
#include <sys/stat.h>
#include "str.h"

typedef struct file_cache_struct* File;
struct file_cache_struct {
	File next;
	struct stat st;
	char* data;
	int fd;
	int count;
	char name[0];
};

extern str cwd;

File open_file(File cache, str filename);
File query_cache(File* cache, str filename);
File close_file(File fc, str filename);

void set_cwd();
str file_path(str host, str filename);

#define file_path(h,f) \
	Str("%s/%s%s",cwd,h,f)

#endif
