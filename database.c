// database.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "str.h"
#include "log.h"
#include "database.h"
#include "usage.h"

Database db;

int
query(str q)
{
	if (db->res) reset();
	dblog("%s",q);
	db->res = PQexec(db->conn,q->data);
	switch(PQresultStatus(db->res)) {
		case PGRES_EMPTY_QUERY:
		case PGRES_COMMAND_OK:
			PQclear(db->res);
			db->res = NULL;
			return 0;
		case PGRES_TUPLES_OK:
			return PQntuples(db->res);	
		case PGRES_BAD_RESPONSE:
		case PGRES_NONFATAL_ERROR:
		case PGRES_FATAL_ERROR:
		default:
			dblog("%s failed %c", q, PQresultErrorMessage(db->res));
			PQclear(db->res);
			db->res = NULL;
	}
	return -1;
}

int
fields()
{
	if (!db->res) return 0;
	return PQnfields(db->res);
}

str
field(int i)
{
	if (!db->res) return NULL;
	return $("%c",PQfname(db->res,i));
}

str
db_error()
{
	if (!db->res) return NULL;
	return $("%c",PQresultErrorMessage(db->res));
}

str
fetch(int row, int col)
{
	if (!db->res) return NULL;
	return $("%c",PQgetvalue(db->res,row,col));	
}

void
reset()
{
	if (db->res) PQclear(db->res);
	db->res = NULL;
}

Database
new_database()
{
	Database retval = (Database)reserve(sizeof(struct database_struct));
	debug(DB_CONNECT_STRING);
	retval->conn = PQconnectdb(DB_CONNECT_STRING);
	if (PQstatus(retval->conn) != CONNECTION_OK) dblog("Failed to connect to database %c",DB_CONNECT_STRING);
	retval->res = NULL;
	set_database(retval);
	return retval;
}

void
set_database(Database d)
{
	db = d;
}

void
close_database(Database d)
{
	PQfinish(d->conn);
	d->conn = NULL;
	d->res = NULL;
}

str
singlequote(str s)
{
	int err = 0;
	if (len(s) == 0) return s;
	str retval = blank(len(s)*2);
	retval->length = PQescapeStringConn(db->conn,retval->data,s->data,len(s),&err);
	if (err) dblog("Failed to escape string %s",s);
	return retval;
}
