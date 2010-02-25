// Files.h
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __FILES_H__
#define __FILES_H__

#include <sys/types.h>
#include <sys/stat.h>
#include "str.h"

typedef struct parsed_struct* Parsed;
struct parsed_struct {
	enum { TEXT = 1, SCRIPT = 2, EMIT = 3 } kind;
	int offset;
	int length;
};

typedef struct file_cache_struct* File;
typedef struct mime_struct MimeTypes;
struct mime_struct {
	struct { int len; char* data; } ending;
	struct { int len; char* data; } type;
	int (*handler)(File);
};

struct file_cache_struct {
	File next;
	struct stat st;
	Parsed parsed;
	MimeTypes* mime;
	char* data;
	int fd;
	struct str_struct  name;
};

extern File files;
extern str cwd;

File load(str filename);
File reload(File fc);

File open_file(str filename);
File query_cache(str filename);
File parse_file(File fc);
void close_files();

void set_cwd();
str temp_file();

#define file_path(h,f) _("%s/%s%s",cwd,h,f)

#endif
