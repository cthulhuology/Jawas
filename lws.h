// lws.h
//
// Lua Bindings for Jawas
//
// © 2009 David J. Goehrig
// All Rights Reserved
//


int lws_handler(File fc);
int run_lua_script(File fc, Headers data);
int process_callback(str cb, Headers headers);
