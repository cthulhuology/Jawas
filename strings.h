// strings.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_JAWAS_STRINGS__H__
#define __HAVE_JAWAS_STRINGS__H__

#define ext_const_str(x) extern str x
#define const_str(x) str x = NULL
#define init_str(x) x = Str(#x)

ext_const_str(OK);
ext_const_str(ERROR);
ext_const_str(CMSS);
ext_const_str(CMGW);
ext_const_str(CMGR);
ext_const_str(CMGD);
ext_const_str(CMTI);

int init_strings();
#endif
