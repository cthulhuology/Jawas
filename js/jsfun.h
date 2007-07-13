
#ifndef jsfun_h___
#define jsfun_h___
/*
 * JS function definitions.
 */
#include "jsprvtd.h"
#include "jspubtd.h"

JS_BEGIN_EXTERN_C

struct JSFunction {
    jsrefcount   nrefs;         /* number of referencing objects */
    JSObject     *object;       /* back-pointer to GC'ed object header */
    union {
        JSNative native;        /* native method pointer or null */
        JSScript *script;       /* interpreted bytecode descriptor or null */
    } u;
    uint16       nargs;         /* minimum number of actual arguments */
    uint16       extra;         /* number of arg slots for local GC roots */
    uint16       nvars;         /* number of local variables */
    uint8        flags;         /* bound method and other flags, see jsapi.h */
    JSPackedBool interpreted;   /* use u.script if true, u.native if false */
    uint16       nregexps;      /* number of regular expressions literals */
    uint16       spare;         /* reserved for future use */
    JSAtom       *atom;         /* name for diagnostics and decompiling */
    JSClass      *clasp;        /* if non-null, constructor for this class */
};

#define FUN_NATIVE(fun)         ((fun)->interpreted ? NULL : (fun)->u.native)
#define FUN_SCRIPT(fun)         ((fun)->interpreted ? (fun)->u.script : NULL)

extern JSClass js_ArgumentsClass;
extern JSClass js_CallClass;

/* JS_FRIEND_DATA so that JSVAL_IS_FUNCTION is callable from outside */
extern JS_FRIEND_DATA(JSClass) js_FunctionClass;

/*
 * NB: jsapi.h and jsobj.h must be included before any call to this macro.
 */
#define JSVAL_IS_FUNCTION(cx, v)                                              \
    (!JSVAL_IS_PRIMITIVE(v) &&                                                \
     OBJ_GET_CLASS(cx, JSVAL_TO_OBJECT(v)) == &js_FunctionClass)

extern JSBool
js_fun_toString(JSContext *cx, JSObject *obj, uint32 indent,
                uintN argc, jsval *argv, jsval *rval);

extern JSBool
js_IsIdentifier(JSString *str);

extern JSObject *
js_InitFunctionClass(JSContext *cx, JSObject *obj);

extern JSObject *
js_InitArgumentsClass(JSContext *cx, JSObject *obj);

extern JSObject *
js_InitCallClass(JSContext *cx, JSObject *obj);

extern JSFunction *
js_NewFunction(JSContext *cx, JSObject *funobj, JSNative native, uintN nargs,
               uintN flags, JSObject *parent, JSAtom *atom);

extern JSObject *
js_CloneFunctionObject(JSContext *cx, JSObject *funobj, JSObject *parent);

extern JSBool
js_LinkFunctionObject(JSContext *cx, JSFunction *fun, JSObject *object);

extern JSFunction *
js_DefineFunction(JSContext *cx, JSObject *obj, JSAtom *atom, JSNative native,
                  uintN nargs, uintN flags);

/*
 * Flags for js_ValueToFunction and js_ReportIsNotFunction.  We depend on the
 * fact that JSINVOKE_CONSTRUCT (aka JSFRAME_CONSTRUCTING) is 1, and test that
 * with #if/#error in jsfun.c.
 */
#define JSV2F_CONSTRUCT         JSINVOKE_CONSTRUCT
#define JSV2F_SEARCH_STACK      2

extern JSFunction *
js_ValueToFunction(JSContext *cx, jsval *vp, uintN flags);

extern JSObject *
js_ValueToFunctionObject(JSContext *cx, jsval *vp, uintN flags);

extern JSObject *
js_ValueToCallableObject(JSContext *cx, jsval *vp, uintN flags);

extern void
js_ReportIsNotFunction(JSContext *cx, jsval *vp, uintN flags);

extern JSObject *
js_GetCallObject(JSContext *cx, JSStackFrame *fp, JSObject *parent);

extern JSBool
js_PutCallObject(JSContext *cx, JSStackFrame *fp);

extern JSBool
js_GetCallVariable(JSContext *cx, JSObject *obj, jsval id, jsval *vp);

extern JSBool
js_SetCallVariable(JSContext *cx, JSObject *obj, jsval id, jsval *vp);

extern JSBool
js_GetArgsValue(JSContext *cx, JSStackFrame *fp, jsval *vp);

extern JSBool
js_GetArgsProperty(JSContext *cx, JSStackFrame *fp, jsid id,
                   JSObject **objp, jsval *vp);

extern JSObject *
js_GetArgsObject(JSContext *cx, JSStackFrame *fp);

extern JSBool
js_PutArgsObject(JSContext *cx, JSStackFrame *fp);

extern JSBool
js_XDRFunction(JSXDRState *xdr, JSObject **objp);

JS_END_EXTERN_C

#endif /* jsfun_h___ */
