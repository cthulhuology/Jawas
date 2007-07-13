
/*
 * JS configuration macros.
 */
#ifndef JS_VERSION
#define JS_VERSION 160
#endif

#if JS_VERSION == 160

#define JS_HAS_PROP_DELETE      1       /* delete o.p removes p from o */
#define JS_HAS_CALL_OBJECT      1       /* fun.caller is stack frame obj */
#define JS_HAS_LABEL_STATEMENT  1       /* has break/continue to label: */
#define JS_HAS_DO_WHILE_LOOP    1       /* has do {...} while (b) */
#define JS_HAS_SWITCH_STATEMENT 1       /* has switch (v) {case c: ...} */
#define JS_HAS_SOME_PERL_FUN    1       /* has array.join/reverse/sort */
#define JS_HAS_MORE_PERL_FUN    1       /* has array.push, str.substr, etc */
#define JS_HAS_STR_HTML_HELPERS 1       /* has str.anchor, str.bold, etc. */
#define JS_HAS_PERL_SUBSTR      1       /* has str.substr */
#define JS_HAS_VALUEOF_HINT     1       /* valueOf(hint) where hint is typeof */
#define JS_HAS_LEXICAL_CLOSURE  1       /* nested functions, lexically closed */
#define JS_HAS_APPLY_FUNCTION   1       /* has apply(fun, arg1, ... argN) */
#define JS_HAS_CALL_FUNCTION    1       /* has fun.call(obj, arg1, ... argN) */
#define JS_HAS_OBJ_PROTO_PROP   1       /* has o.__proto__ etc. */
#define JS_HAS_REGEXPS          1       /* has perl r.e.s via RegExp, /pat/ */
#define JS_HAS_SEQUENCE_OPS     1       /* has array.slice, string.concat */
#define JS_HAS_INITIALIZERS     1       /* has var o = {'foo': 42, 'bar':3} */
#define JS_HAS_OBJ_WATCHPOINT   1       /* has o.watch and o.unwatch */
#define JS_HAS_EXPORT_IMPORT    1       /* has export fun; import obj.fun */
#define JS_HAS_EVAL_THIS_SCOPE  1       /* Math.eval is same as with (Math) */
#define JS_HAS_TRIPLE_EQOPS     1       /* has === and !== identity eqops */
#define JS_HAS_SHARP_VARS       1       /* has #n=, #n# for object literals */
#define JS_HAS_REPLACE_LAMBDA   1       /* has string.replace(re, lambda) */
#define JS_HAS_SCRIPT_OBJECT    1       /* has (new Script("x++")).exec() */
#define JS_HAS_XDR              1       /* has XDR API and internal support */
#define JS_HAS_XDR_FREEZE_THAW  0       /* has XDR freeze/thaw script methods */
#define JS_HAS_EXCEPTIONS       1       /* has exception handling */
#define JS_HAS_UNDEFINED        1       /* has global "undefined" property */
#define JS_HAS_TOSOURCE         1       /* has Object/Array toSource method */
#define JS_HAS_IN_OPERATOR      1       /* has in operator ('p' in {p:1}) */
#define JS_HAS_INSTANCEOF       1       /* has {p:1} instanceof Object */
#define JS_HAS_ARGS_OBJECT      1       /* has minimal ECMA arguments object */
#define JS_HAS_DEBUGGER_KEYWORD 1       /* has hook for debugger keyword */
#define JS_HAS_ERROR_EXCEPTIONS 1       /* rt errors reflected as exceptions */
#define JS_HAS_CATCH_GUARD      1       /* has exception handling catch guard */
#define JS_HAS_NEW_OBJ_METHODS  1       /* has Object.prototype query methods */
#define JS_HAS_SPARSE_ARRAYS    0       /* array methods preserve empty elems */
#define JS_HAS_DFLT_MSG_STRINGS 1       /* provides English error messages */
#define JS_HAS_NUMBER_FORMATS   1       /* numbers have formatting methods */
#define JS_HAS_GETTER_SETTER    1       /* has JS2 getter/setter functions */
#define JS_HAS_UNEVAL           1       /* has uneval() top-level function */
#define JS_HAS_CONST            1       /* has JS2 const as alternative var */
#define JS_HAS_FUN_EXPR_STMT    1       /* has function expression statement */
#define JS_HAS_LVALUE_RETURN    1       /* has o.item(i) = j; for native item */
#define JS_HAS_NO_SUCH_METHOD   1       /* has o.__noSuchMethod__ handler */
#define JS_HAS_XML_SUPPORT      1       /* has ECMAScript for XML support */
#define JS_HAS_ARRAY_EXTRAS     1       /* has indexOf and Lispy extras */

#else

#error "unknown JS_VERSION"

#endif
