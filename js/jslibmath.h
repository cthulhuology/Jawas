/*
 * By default all math calls go to fdlibm.  The defines for each platform
 * remap the math calls to native routines.
 */

#ifndef _LIBMATH_H
#define _LIBMATH_H

#include <math.h>
#include "jsconfig.h"

/*
 * Define which platforms on which to use fdlibm.  Not used
 * by default since there can be problems with endian-ness and such.
 */

#if defined(linux)
#define JS_USE_FDLIBM_MATH 1
#else
#define JS_USE_FDLIBM_MATH 0
#endif

#if !JS_USE_FDLIBM_MATH

/*
 * Use system provided math routines.
 */

#define fd_acos acos
#define fd_asin asin
#define fd_atan atan
#define fd_atan2 atan2
#define fd_ceil ceil
#define fd_copysign copysign
#define fd_cos cos
#define fd_exp exp
#define fd_fabs fabs
#define fd_floor floor
#define fd_fmod fmod
#define fd_log log
#define fd_pow pow
#define fd_sin sin
#define fd_sqrt sqrt
#define fd_tan tan

#else

/*
 * Use math routines in fdlibm.
 */

#undef __P
#ifdef __STDC__
#define __P(p)  p
#else
#define __P(p)  ()
#endif

#if defined(linux)

#define fd_atan atan
#define fd_atan2 atan2
#define fd_ceil ceil
#define fd_cos cos
#define fd_fabs fabs
#define fd_floor floor
#define fd_fmod fmod
#define fd_sin sin
#define fd_sqrt sqrt
#define fd_tan tan
#define fd_copysign copysign

extern double fd_asin __P((double));
extern double fd_acos __P((double));
extern double fd_exp __P((double));
extern double fd_log __P((double));
extern double fd_pow __P((double, double));

#endif

#endif /* JS_USE_FDLIBM_MATH */

#endif /* _LIBMATH_H */

