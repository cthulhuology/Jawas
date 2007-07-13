
/*
 * PR assertion checker.
 */

#ifndef jsutil_h___
#define jsutil_h___

JS_BEGIN_EXTERN_C

#define JS_ASSERT(expr) ((void) 0)
#define JS_NOT_REACHED(reasonStr)

/*
** Abort the process in a non-graceful manner. This will cause a core file,
** call to the debugger or other moral equivalent as well as causing the
** entire process to stop.
*/
extern JS_PUBLIC_API(void) JS_Abort(void);

#ifdef XP_UNIX

typedef struct JSCallsite JSCallsite;

struct JSCallsite {
    uint32      pc;
    char        *name;
    const char  *library;
    int         offset;
    JSCallsite  *parent;
    JSCallsite  *siblings;
    JSCallsite  *kids;
    void        *handy;
};

extern JSCallsite *JS_Backtrace(int skip);

#endif

JS_END_EXTERN_C

#endif /* jsutil_h___ */
