// index.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_INDEX_H__
#define __HAVE_INDEX_H__
#include "files.h"

int is_directory(str filename);
File get_index(str filename);
str deauth_path(str host, str filename);

#endif
