// Files.h
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __FILES_H__
#define __FILES_H__

#include <sys/types.h>
#include <sys/stat.h>

typedef struct file_cache_struct* File;
struct file_cache_struct {
	File next;
	struct stat st;
	char* data;
	int fd;
	int count;
	char name[0];
};

File open_file(File cache, char* filename);
File query_cache(File* cache, char* filename);
File close_file(File fc, char* filename);

#endif
