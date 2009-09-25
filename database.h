// database.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_DATABASE_H__
#define __HAVE_DATABASE_H__

#include <libpq-fe.h>
#include "str.h"

typedef struct database_struct* Database;
struct database_struct {
	PGconn* conn;
	PGresult* res;
};

str guid();
int query(str q);
int fields();
str field(int i);
str fetch(int row, int col);
str db_error();
void reset();
str singlequote(str s);

Database new_database();
void set_database(Database d);
void close_database(Database d);


#endif
