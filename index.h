// index.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_INDEX_H__
#define __HAVE_INDEX_H__

static char* indexes[] = {
	"/index.html",
	"/index.jws",
	"/index.xml",
	NULL
};

int is_directory(str filename);
str get_index(str filename);

#endif
