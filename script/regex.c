// regex.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

char* exp;
char** matches;
int nmatches = 0;

char* spaces_class = " \t\n\r";
char* digits_class = "0123456789";
char* az_class = "abcdefghijklmnopqrstuvwxyz";
char* AZ_class = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char* reserved_class = "^$\\.*+?(){}[]|";

typedef struct rule_struct* rule_t;
struct rule_struct {
	char* (*apply)(rule_t,char*,char*);
	char* a_str;
	rule_t a_rule;
	char* b;
	rule_t b_rule;
};
	

rule_t*
compile_regex(char* regexp)
{

}

char*
apply_rules(char* regexp, char* src)
{
	int count = 1;
	rule_t rules = compile_regex(regexp);
	exp = src;
	for (int i = 0; exp[i]; ++i)
		if (exp[i] == '(' && exp[i-1] != '\\')
			++count;
	matches = (char**)malloc(sizeof(char*) * count);
	nmatches = 0;
	rules->apply(
	return exp;
}

int
in(char c, char* r)
{
	if (!r) return 1;
	for (int i  = 0; r[i]; ++i)
		if (r[i] == c) return 1;
	return 0;
}

int
eq(char* src)
{
	if (!src) return exp == NULL;
	return !strcmp(src,exp);
}

char*
match_rule(rule_t match, char* src, char* null)
{
	if (match) return match->apply(src);
	return eq(src) ? src : NULL;
}

char*
or_rule(rule_t match, char* a_src,char* b_src)
{
	if (match->apply(match->a_rule, a_src, NULL)) return a_src;
	if (match->apply(match->b_rule, b_src, NULL)) return b_src;
	return NULL;
}

char*
capture_rule(rule_t match, char* src, char* null)
{
	if (match->apply(src)) {
		matches[nmatches++] = src;
		return src;
	}
	return NULL;	
}

char*
not_rule(rule_t match, char* src, char* null)
{
	return match->apply(match->a_rule,src) ? NULL : src;
}

char*
class_rule(rule_t match, char* src, char* range)
{
	ITERATE(src) 
		if (!in(src[i],range))
			return NULL;
	return src;
}

