
#ifndef _jsfile_h__
#define _jsfile_h__

extern JS_PUBLIC_API(JSObject*)
js_InitFileClass(JSContext *cx, JSObject* obj);

extern JS_PUBLIC_API(JSObject*)
js_NewFileObject(JSContext *cx, char *bytes);
#endif /* _jsfile_h__ */
