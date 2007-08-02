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

File open_file(File cache, str filename);
File query_cache(File* cache, str filename);
File close_file(File fc, str filename);

#endif
