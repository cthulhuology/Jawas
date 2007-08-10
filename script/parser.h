// parser.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_PARSER_H__
#define __HAVE_PARSER_H__

#include "str.h"

#define CONSTRUCTOR(x,y) x y() { return (x)salloc(sizeof(struct x ## _struct )); }
#define typedefof(x,y) typedef struct x ## _struct* x; x y();

typedefof(exp,Exp);
typedefof(blk,Block);
typedefof(try,Try);
typedefof(ctch,Catch);
typedefof(ife,IfElse);
typedefof(swtch,Switch);
typedefof(cse,Case);
typedefof(wth,With);
typedefof(lbl,Label);
typedefof(itr,Iterator);
typedefof(obj,Object);
typedefof(func,Func);
typedefof(stmt,Stmt);

typedef stmt(*parse_t)(cstr);

struct exp_struct {
	cstr object;
	int msg;
	exp args;
};

struct wth_struct {
	exp exp;
	stmt state;	
};

struct swtch_struct {
	exp exp;
	cse cases;
};

// default: exp == NULL
struct cse_struct {
	exp exp;
	stmt state;
};

struct lbl_struct {
	cstr id;
	stmt state;
};

struct try_struct {
	blk try;
	ctch catch;
};

// finally ident == NULL
struct ctch_struct {
	cstr id;
	blk blk;
	ctch tail;	
};

struct func_struct {
	cstr id;
	exp params;
	blk blk;
};

struct obj_struct {
	cstr key;
	exp value;	
	obj tail;
};

// Else test == NULL
struct ife_struct {
	exp test;
	stmt blk;
	ife tail;
};

struct itr_struct {
	stmt init_exp;
	exp test_exp;
	exp final_exp;
	blk state;
};

struct blk_struct {
	stmt state;
	blk tail;
};

enum stmt_flags { UNKNOWN, BLK, VAR, NEW, EXP, IFE, ITR, CNT, BRK, RET, WTH, LBL, SWT, THR, TRY };

struct stmt_struct {
	enum stmt_flags flag;
	cstr rep;
	union {
		blk blk;		// blks
		exp var;		// var declarations
		exp new;		// new expressions
		exp exp;		// any expressions
		ife ife;		// if else
		itr itr;		// iterators (do, while, for)
		cstr cnt;		// continue
		cstr brk;		// break
		exp ret;		// return
		exp throw;		// throw
		wth wth;		// wth
		lbl lbl;		// label
		swtch swt;		// switch
		try try;		// try / ctch / finalize
	} val;
};

static char* reserved_word_list[] = {
	"",				// Index starts at 1
	"finally",  "catch", 
	"instanceof", 
	"undefined",
	"continue", "function", 
	"return", "delete", "typeof", 
	"while", "break",
	"throw", "false",
	"with", "else", "void", "case", "true", 
	"null", "this", ">>>=",
	"===", "for", "try", "new", "<<=", ">>=", 
	"++", "--", "*=", "/=", "%=", "+=", "-=",
	"&=", "|=", "^=", "==", "//", "/*", "*/",
	"!=", "<<", "<=", ">>",  ">=", "do", "if", "in", 
	";", ":", "?", ".", "\"", "'", "(", ")", "{", "}", 
	"[", "]", "<", ">", "~", ",", "*", "/", "%", "+", 
	"-", "&", "|", "^", "!", "=", 
	NULL
};

static char eol_chars[] = " ;\n\r";

static char delim_chars[] = " \r\n\t .,!=;:?+-\"'(){}[]+-<>~*/%&|^\v";

cstr parse_block(cstr buf, blk* b);

#endif
