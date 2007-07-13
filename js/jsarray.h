
#ifndef jsarray_h___
#define jsarray_h___
/*
 * JS Array interface.
 */
#include "jsprvtd.h"
#include "jspubtd.h"

JS_BEGIN_EXTERN_C

extern JSBool
js_IdIsIndex(jsval id, jsuint *indexp);

extern JSClass js_ArrayClass;

extern JSObject *
js_InitArrayClass(JSContext *cx, JSObject *obj);

extern JSObject *
js_NewArrayObject(JSContext *cx, jsuint length, jsval *vector);

extern JSBool
js_GetLengthProperty(JSContext *cx, JSObject *obj, jsuint *lengthp);

extern JSBool
js_SetLengthProperty(JSContext *cx, JSObject *obj, jsuint length);

extern JSBool
js_HasLengthProperty(JSContext *cx, JSObject *obj, jsuint *lengthp);

/*
 * JS-specific heap sort function.
 */
typedef int (*JSComparator)(const void *a, const void *b, void *arg);

extern void
js_HeapSort(void *vec, size_t nel, void *pivot, size_t elsize,
            JSComparator cmp, void *arg);

JS_END_EXTERN_C

#endif /* jsarray_h___ */
