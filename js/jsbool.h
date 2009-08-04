
#ifndef jsbool_h___
#define jsbool_h___
/*
 * JS boolean interface.
 */

JS_BEGIN_EXTERN_C

/*
 * Crypto-booleans, not visible to script but used internally by the engine.
 *
 * JSVAL_HOLE is a useful value for identifying a hole in an array.  It's also
 * used in the interpreter to represent "no exception pending".  In general it
 * can be used to represent "no value".
 */
#define JSVAL_HOLE      BOOLEAN_TO_JSVAL(2)

extern JSClass js_BooleanClass;

extern JSObject *
js_InitBooleanClass(JSContext *cx, JSObject *obj);

extern JSObject *
js_BooleanToObject(JSContext *cx, JSBool b);

extern JSString *
js_BooleanToString(JSContext *cx, JSBool b);

extern JSBool
js_ValueToBoolean(JSContext *cx, jsval v, JSBool *bp);

JS_END_EXTERN_C

#endif /* jsbool_h___ */
