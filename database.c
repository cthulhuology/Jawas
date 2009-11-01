// database.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "log.h"
#include "database.h"
#include "usage.h"

Database db;

str
guid()
{
	db->res = PQexec(db->conn,"SELECT nextval('guid_seq')");
	str retval = PQresultStatus(db->res) != PGRES_TUPLES_OK ?  NULL : Str("%c",PQgetvalue(db->res,0,0));
	if (!retval) dblog("%c",PQresultErrorMessage(db->res));
	PQclear(db->res);
	db->res = NULL;
	return retval;
}

int
query(str q)
{
	if (db->res) reset();
	char* qry = dump(q);
	dblog("%s",q);
	Usage u = new_usage();
	start_usage(u);
	db->res = PQexec(db->conn,qry);
	stop_usage(u);
	dump_usage(u);
	free(qry);
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
	return Str("%c",PQfname(db->res,i));
}

str
db_error()
{
	if (!db->res) return NULL;
	return Str("%c",PQresultErrorMessage(db->res));
}

str
fetch(int row, int col)
{
	if (!db->res) return NULL;
	return Str("%c",PQgetvalue(db->res,row,col));	
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
	Database retval = (Database)salloc(sizeof(struct database_struct));
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
