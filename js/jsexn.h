
/*
 * JS runtime exception classes.
 */

#ifndef jsexn_h___
#define jsexn_h___

JS_BEGIN_EXTERN_C

/*
 * Initialize the exception constructor/prototype hierarchy.
 */
extern JSObject *
js_InitExceptionClasses(JSContext *cx, JSObject *obj);

/*
 * String constants naming the exception classes.
 */
extern const char js_Error_str[];
extern const char js_InternalError_str[];
extern const char js_EvalError_str[];
extern const char js_RangeError_str[];
extern const char js_ReferenceError_str[];
extern const char js_SyntaxError_str[];
extern const char js_TypeError_str[];
extern const char js_URIError_str[];

/*
 * Given a JSErrorReport, check to see if there is an exception associated with
 * the error number.  If there is, then create an appropriate exception object,
 * set it as the pending exception, and set the JSREPORT_EXCEPTION flag on the
 * error report.  Exception-aware host error reporters should probably ignore
 * error reports so flagged.  Returns JS_TRUE if an associated exception is
 * found and set, JS_FALSE otherwise..
 */
extern JSBool
js_ErrorToException(JSContext *cx, const char *message, JSErrorReport *reportp);

/*
 * Called if a JS API call to js_Execute or js_InternalCall fails; calls the
 * error reporter with the error report associated with any uncaught exception
 * that has been raised.  Returns true if there was an exception pending, and
 * the error reporter was actually called.
 *
 * The JSErrorReport * that the error reporter is called with is currently
 * associated with a JavaScript object, and is not guaranteed to persist after
 * the object is collected.  Any persistent uses of the JSErrorReport contents
 * should make their own copy.
 *
 * The flags field of the JSErrorReport will have the JSREPORT_EXCEPTION flag
 * set; embeddings that want to silently propagate JavaScript exceptions to
 * other contexts may want to use an error reporter that ignores errors with
 * this flag.
 */
extern JSBool
js_ReportUncaughtException(JSContext *cx);

extern JSErrorReport *
js_ErrorFromException(JSContext *cx, jsval exn);

JS_END_EXTERN_C

#endif /* jsexn_h___ */
