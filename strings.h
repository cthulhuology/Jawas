// strings.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_JAWAS_STRINGS__H__
#define __HAVE_JAWAS_STRINGS__H__

#define ext_const_str(x) extern str x
#define const_str(x) str x = NULL
#define init_str(x) x = $(#x)

ext_const_str(OK);
ext_const_str(ERROR);
ext_const_str(CMSS);
ext_const_str(CMGW);
ext_const_str(CMGR);
ext_const_str(CMGD);
ext_const_str(CMTI);

int init_strings();

extern char* Cache_Control_MSG;
extern char* Connection_MSG;
extern char* Date_MSG;
extern char* Transfer_Encoding_MSG;
extern char* Content_Length_MSG;
extern char* Content_Type_MSG;
extern char* Expires_MSG;
extern char* Location_MSG;
extern char* Server_MSG;
#endif
