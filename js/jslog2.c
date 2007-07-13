#include "jsstddef.h"
#include "jsbit.h"

/*
** Compute the log of the least power of 2 greater than or equal to n
*/
JS_PUBLIC_API(JSIntn) JS_CeilingLog2(JSUint32 n)
{
	JSIntn          log2 = 0;

	if (n & (n - 1))
		log2++;
	if (n >> 16)
		log2 += 16, n >>= 16;
	if (n >> 8)
		log2 += 8, n >>= 8;
	if (n >> 4)
		log2 += 4, n >>= 4;
	if (n >> 2)
		log2 += 2, n >>= 2;
	if (n >> 1)
		log2++;
	return log2;
}

/*
** Compute the log of the greatest power of 2 less than or equal to n.
** This really just finds the highest set bit in the word.
*/
JS_PUBLIC_API(JSIntn) JS_FloorLog2(JSUint32 n)
{
	JSIntn          log2 = 0;

	if (n >> 16)
		log2 += 16, n >>= 16;
	if (n >> 8)
		log2 += 8, n >>= 8;
	if (n >> 4)
		log2 += 4, n >>= 4;
	if (n >> 2)
		log2 += 2, n >>= 2;
	if (n >> 1)
		log2++;
	return log2;
}
