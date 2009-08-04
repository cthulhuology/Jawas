
#ifndef prmjtime_h___
#define prmjtime_h___
/*
 * PR date stuff for mocha and java. Placed here temporarily not to break
 * Navigator and localize changes to mocha.
 */
#include <time.h>
#include "jslong.h"
#ifdef MOZILLA_CLIENT
#include "jscompat.h"
#endif

JS_BEGIN_EXTERN_C

typedef struct PRMJTime       PRMJTime;

/*
 * Broken down form of 64 bit time value.
 */
struct PRMJTime {
    JSInt32 tm_usec;            /* microseconds of second (0-999999) */
    JSInt8 tm_sec;              /* seconds of minute (0-59) */
    JSInt8 tm_min;              /* minutes of hour (0-59) */
    JSInt8 tm_hour;             /* hour of day (0-23) */
    JSInt8 tm_mday;             /* day of month (1-31) */
    JSInt8 tm_mon;              /* month of year (0-11) */
    JSInt8 tm_wday;             /* 0=sunday, 1=monday, ... */
    JSInt16 tm_year;            /* absolute year, AD */
    JSInt16 tm_yday;            /* day of year (0 to 365) */
    JSInt8 tm_isdst;            /* non-zero if DST in effect */
};

/* Some handy constants */
#define PRMJ_USEC_PER_SEC       1000000L
#define PRMJ_USEC_PER_MSEC      1000L

/* Return the current local time in micro-seconds */
extern JSInt64
PRMJ_Now(void);

/* get the difference between this time zone and  gmt timezone in seconds */
extern JSInt32
PRMJ_LocalGMTDifference(void);

/* Format a time value into a buffer. Same semantics as strftime() */
extern size_t
PRMJ_FormatTime(char *buf, int buflen, char *fmt, PRMJTime *tm);

/* Get the DST offset for the local time passed in */
extern JSInt64
PRMJ_DSTOffset(JSInt64 local_time);

JS_END_EXTERN_C

#endif /* prmjtime_h___ */

