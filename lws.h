// lws.h
//
// Lua Bindings for Jawas
//
// © 2009 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_LWS_H__
#define  __HAVE_LWS_H__

void init_lua();
int lws_handler(File fc);
int run_lua_script(File fc, Headers data);
int process_callback(str cb, Headers headers);

#endif
