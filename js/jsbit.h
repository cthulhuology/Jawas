#ifndef jsbit_h___
#define jsbit_h___

#include "jstypes.h"
JS_BEGIN_EXTERN_C

/*
** A jsbitmap_t is a long integer that can be used for bitmaps
*/
typedef JSUword     jsbitmap_t;     /* NSPR name, a la Unix system types */
typedef jsbitmap_t  jsbitmap;       /* JS-style scalar typedef name */

#define JS_TEST_BIT(_map,_bit) \
    ((_map)[(_bit)>>JS_BITS_PER_WORD_LOG2] & (1L << ((_bit) & (JS_BITS_PER_WORD-1))))
#define JS_SET_BIT(_map,_bit) \
    ((_map)[(_bit)>>JS_BITS_PER_WORD_LOG2] |= (1L << ((_bit) & (JS_BITS_PER_WORD-1))))
#define JS_CLEAR_BIT(_map,_bit) \
    ((_map)[(_bit)>>JS_BITS_PER_WORD_LOG2] &= ~(1L << ((_bit) & (JS_BITS_PER_WORD-1))))

/*
** Compute the log of the least power of 2 greater than or equal to n
*/
extern JS_PUBLIC_API(JSIntn) JS_CeilingLog2(JSUint32 i);

/*
** Compute the log of the greatest power of 2 less than or equal to n
*/
extern JS_PUBLIC_API(JSIntn) JS_FloorLog2(JSUint32 i);

/*
** Macro version of JS_CeilingLog2: Compute the log of the least power of
** 2 greater than or equal to _n. The result is returned in _log2.
*/
#define JS_CEILING_LOG2(_log2,_n)                                             \
    JS_BEGIN_MACRO                                                            \
        JSUint32 j_ = (JSUint32)(_n);                                         \
        (_log2) = 0;                                                          \
        if ((j_) & ((j_)-1))                                                  \
            (_log2) += 1;                                                     \
        if ((j_) >> 16)                                                       \
            (_log2) += 16, (j_) >>= 16;                                       \
        if ((j_) >> 8)                                                        \
            (_log2) += 8, (j_) >>= 8;                                         \
        if ((j_) >> 4)                                                        \
            (_log2) += 4, (j_) >>= 4;                                         \
        if ((j_) >> 2)                                                        \
            (_log2) += 2, (j_) >>= 2;                                         \
        if ((j_) >> 1)                                                        \
            (_log2) += 1;                                                     \
    JS_END_MACRO

/*
** Macro version of JS_FloorLog2: Compute the log of the greatest power of
** 2 less than or equal to _n. The result is returned in _log2.
**
** This is equivalent to finding the highest set bit in the word.
*/
#define JS_FLOOR_LOG2(_log2,_n)                                               \
    JS_BEGIN_MACRO                                                            \
        JSUint32 j_ = (JSUint32)(_n);                                         \
        (_log2) = 0;                                                          \
        if ((j_) >> 16)                                                       \
            (_log2) += 16, (j_) >>= 16;                                       \
        if ((j_) >> 8)                                                        \
            (_log2) += 8, (j_) >>= 8;                                         \
        if ((j_) >> 4)                                                        \
            (_log2) += 4, (j_) >>= 4;                                         \
        if ((j_) >> 2)                                                        \
            (_log2) += 2, (j_) >>= 2;                                         \
        if ((j_) >> 1)                                                        \
            (_log2) += 1;                                                     \
    JS_END_MACRO

JS_END_EXTERN_C
#endif /* jsbit_h___ */
