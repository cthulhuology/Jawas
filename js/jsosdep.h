
#ifndef jsosdep_h___
#define jsosdep_h___
/*
 * OS (and machine, and compiler XXX) dependent information.
 */

#if defined(XP_WIN) || defined(XP_OS2)

#if defined(_WIN32) || defined (XP_OS2)
#define JS_HAVE_LONG_LONG
#else
#undef JS_HAVE_LONG_LONG
#endif
#endif /* XP_WIN || XP_OS2 */

#ifdef XP_BEOS
#define JS_HAVE_LONG_LONG
#endif


#ifdef XP_UNIX

/*
 * Get OS specific header information.
 */
#if defined(AIXV3) || defined(AIX)
#define JS_HAVE_LONG_LONG

#elif defined(BSDI)
#define JS_HAVE_LONG_LONG

#elif defined(HPUX)
#define JS_HAVE_LONG_LONG

#elif defined(IRIX)
#define JS_HAVE_LONG_LONG

#elif defined(linux)
#define JS_HAVE_LONG_LONG

#elif defined(OSF1)
#define JS_HAVE_LONG_LONG

#elif defined(_SCO_DS)
#undef JS_HAVE_LONG_LONG

#elif defined(SOLARIS)
#define JS_HAVE_LONG_LONG

#elif defined(FREEBSD)
#define JS_HAVE_LONG_LONG

#elif defined(SUNOS4)
#undef JS_HAVE_LONG_LONG

/*
** Missing function prototypes
*/

extern void *sbrk(int);

#elif defined(UNIXWARE)
#undef JS_HAVE_LONG_LONG

#elif defined(VMS) && defined(__ALPHA)
#define JS_HAVE_LONG_LONG

#endif

#endif /* XP_UNIX */

#endif /* jsosdep_h___ */

