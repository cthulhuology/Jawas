
/*
 * JS Date class interface.
 */

#ifndef jsdate_h___
#define jsdate_h___

JS_BEGIN_EXTERN_C

extern JSObject *
js_InitDateClass(JSContext *cx, JSObject *obj);

/*
 * These functions provide a C interface to the date/time object
 */

/*
 * Construct a new Date Object from a time value given in milliseconds UTC
 * since the epoch.
 */
extern JS_FRIEND_API(JSObject*)
js_NewDateObjectMsec(JSContext* cx, jsdouble msec_time);

/*
 * Construct a new Date Object from an exploded local time value.
 */
extern JS_FRIEND_API(JSObject*)
js_NewDateObject(JSContext* cx, int year, int mon, int mday,
                 int hour, int min, int sec);

/*
 * Detect whether the internal date value is NaN.  (Because failure is
 * out-of-band for js_DateGet*)
 */
extern JS_FRIEND_API(JSBool)
js_DateIsValid(JSContext *cx, JSObject* obj);

extern JS_FRIEND_API(int)
js_DateGetYear(JSContext *cx, JSObject* obj);

extern JS_FRIEND_API(int)
js_DateGetMonth(JSContext *cx, JSObject* obj);

extern JS_FRIEND_API(int)
js_DateGetDate(JSContext *cx, JSObject* obj);

extern JS_FRIEND_API(int)
js_DateGetHours(JSContext *cx, JSObject* obj);

extern JS_FRIEND_API(int)
js_DateGetMinutes(JSContext *cx, JSObject* obj);

extern JS_FRIEND_API(int)
js_DateGetSeconds(JSContext *cx, JSObject* obj);

extern JS_FRIEND_API(void)
js_DateSetYear(JSContext *cx, JSObject *obj, int year);

extern JS_FRIEND_API(void)
js_DateSetMonth(JSContext *cx, JSObject *obj, int year);

extern JS_FRIEND_API(void)
js_DateSetDate(JSContext *cx, JSObject *obj, int date);

extern JS_FRIEND_API(void)
js_DateSetHours(JSContext *cx, JSObject *obj, int hours);

extern JS_FRIEND_API(void)
js_DateSetMinutes(JSContext *cx, JSObject *obj, int minutes);

extern JS_FRIEND_API(void)
js_DateSetSeconds(JSContext *cx, JSObject *obj, int seconds);

extern JS_FRIEND_API(jsdouble)
js_DateGetMsecSinceEpoch(JSContext *cx, JSObject *obj);

JS_END_EXTERN_C

#endif /* jsdate_h___ */
