// parser.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_PARSER_H__
#define __HAVE_PARSER_H__

#include "cstr.h"

#define typedefof(x) typedef cstruct x # _cstruct* x
typedefof(exp);
typedefof(blk);
typedefof(try);
typedefof(ctch);
typedefof(ife);
typedefof(swtch);
typedefof(cse);
typedefof(wth);
typedefof(lbl);
typedefof(itr);
typedefof(obj);
typedefof(func);
typedefof(stmt);

typedef stmt(*parse_t)(cstr);

cstruct exp_cstruct {
	cstr object;
	int msg;
	exp args;
};

cstruct wth_cstruct {
	exp exp;
	stmt state;	
};

cstruct swtch_cstruct {
	exp exp;
	cse cases;
};

// default: exp == NULL
cstruct cse_cstruct {
	exp exp;
	stmt state;
};

cstruct lbl_cstruct {
	cstr id;
	stmt state;
};

cstruct try_cstruct {
	blk try;
	ctch catch;
};

// finally ident == NULL
cstruct ctch_cstruct {
	cstr id;
	blk blk;
	ctch tail;	
};

cstruct function_cstruct {
	cstr id;
	exp params;
	blk blk;
};

cstruct obj_cstruct {
	cstr key;
	exp value;	
	obj tail;
};

// Else test == NULL
cstruct ife_cstruct {
	exp test;
	stmt blk;
	ife tail;
};

cstruct iter_cstruct {
	stmt init_exp;
	exp test_exp;
	exp final_exp;
	blk state;
};

cstruct blk_cstruct {
	stmt state;
	blk tail;
};

enum stmt_flags { UNKNOWN, BLK, VAR, NEW, EXP, IFE, ITR, CNT, BRK, RET, WTH, LBL, SWT, THR, TRY };

cstruct stmt_cstruct {
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

static char* reserved_words_list[] = {
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

#endif
