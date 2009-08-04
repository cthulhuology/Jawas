/*
 * PR assertion checker.
 */
#include "jsstddef.h"
#include <stdio.h>
#include <stdlib.h>
#include "jstypes.h"
#include "jsutil.h"

JS_PUBLIC_API(void) JS_Assert(const char *s, const char *file, JSIntn ln)
{
	fprintf(stderr, "Assertion failure: %s, at %s:%d\n", s, file, ln);
	abort();
}
