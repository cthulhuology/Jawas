
#ifndef jsarena_h___
#define jsarena_h___
/*
 * Lifetime-based fast allocation, inspired by much prior art, including
 * "Fast Allocation and Deallocation of Memory Based on Object Lifetimes"
 * David R. Hanson, Software -- Practice and Experience, Vol. 20(1).
 *
 * Also supports LIFO allocation (JS_ARENA_MARK/JS_ARENA_RELEASE).
 */
#include <stdlib.h>
#include "jstypes.h"
#include "jscompat.h"

JS_BEGIN_EXTERN_C

typedef struct JSArena JSArena;
typedef struct JSArenaPool JSArenaPool;

struct JSArena {
    JSArena     *next;          /* next arena for this lifetime */
    jsuword     base;           /* aligned base address, follows this header */
    jsuword     limit;          /* one beyond last byte in arena */
    jsuword     avail;          /* points to next available byte */
};

#ifdef JS_ARENAMETER
typedef struct JSArenaStats JSArenaStats;

struct JSArenaStats {
    JSArenaStats *next;         /* next in arenaStats list */
    char        *name;          /* name for debugging */
    uint32      narenas;        /* number of arenas in pool */
    uint32      nallocs;        /* number of JS_ARENA_ALLOCATE() calls */
    uint32      nreclaims;      /* number of reclaims from freeArenas */
    uint32      nmallocs;       /* number of malloc() calls */
    uint32      ndeallocs;      /* number of lifetime deallocations */
    uint32      ngrows;         /* number of JS_ARENA_GROW() calls */
    uint32      ninplace;       /* number of in-place growths */
    uint32      nreallocs;      /* number of arena grow extending reallocs */
    uint32      nreleases;      /* number of JS_ARENA_RELEASE() calls */
    uint32      nfastrels;      /* number of "fast path" releases */
    size_t      nbytes;         /* total bytes allocated */
    size_t      maxalloc;       /* maximum allocation size in bytes */
    double      variance;       /* size variance accumulator */
};
#endif

struct JSArenaPool {
    JSArena     first;          /* first arena in pool list */
    JSArena     *current;       /* arena from which to allocate space */
    size_t      arenasize;      /* net exact size of a new arena */
    jsuword     mask;           /* alignment mask (power-of-2 - 1) */
#ifdef JS_ARENAMETER
    JSArenaStats stats;
#endif
};

/*
 * If the including .c file uses only one power-of-2 alignment, it may define
 * JS_ARENA_CONST_ALIGN_MASK to the alignment mask and save a few instructions
 * per ALLOCATE and GROW.
 */
#ifdef JS_ARENA_CONST_ALIGN_MASK
#define JS_ARENA_ALIGN(pool, n) (((jsuword)(n) + JS_ARENA_CONST_ALIGN_MASK)   \
                                 & ~(jsuword)JS_ARENA_CONST_ALIGN_MASK)

#define JS_INIT_ARENA_POOL(pool, name, size) \
        JS_InitArenaPool(pool, name, size, JS_ARENA_CONST_ALIGN_MASK + 1)
#else
#define JS_ARENA_ALIGN(pool, n) (((jsuword)(n) + (pool)->mask) & ~(pool)->mask)
#endif

#define JS_ARENA_ALLOCATE(p, pool, nb)                                        \
    JS_ARENA_ALLOCATE_CAST(p, void *, pool, nb)

#define JS_ARENA_ALLOCATE_TYPE(p, type, pool)                                 \
    JS_ARENA_ALLOCATE_COMMON(p, type *, pool, sizeof(type), 0)

#define JS_ARENA_ALLOCATE_CAST(p, type, pool, nb)                             \
    JS_ARENA_ALLOCATE_COMMON(p, type, pool, nb, _nb > _a->limit)

/*
 * NB: In JS_ARENA_ALLOCATE_CAST and JS_ARENA_GROW_CAST, always subtract _nb
 * from a->limit rather than adding _nb to _p, to avoid overflowing a 32-bit
 * address space (possible when running a 32-bit program on a 64-bit system
 * where the kernel maps the heap up against the top of the 32-bit address
 * space).
 *
 * Thanks to Juergen Kreileder <jk@blackdown.de>, who brought this up in
 * https://bugzilla.mozilla.org/show_bug.cgi?id=279273.
 */
#define JS_ARENA_ALLOCATE_COMMON(p, type, pool, nb, guard)                    \
    JS_BEGIN_MACRO                                                            \
        JSArena *_a = (pool)->current;                                        \
        size_t _nb = JS_ARENA_ALIGN(pool, nb);                                \
        jsuword _p = _a->avail;                                               \
        if ((guard) || _p > _a->limit - _nb)                                  \
            _p = (jsuword)JS_ArenaAllocate(pool, _nb);                        \
        else                                                                  \
            _a->avail = _p + _nb;                                             \
        p = (type) _p;                                                        \
        JS_ArenaCountAllocation(pool, nb);                                    \
    JS_END_MACRO

#define JS_ARENA_GROW(p, pool, size, incr)                                    \
    JS_ARENA_GROW_CAST(p, void *, pool, size, incr)

#define JS_ARENA_GROW_CAST(p, type, pool, size, incr)                         \
    JS_BEGIN_MACRO                                                            \
        JSArena *_a = (pool)->current;                                        \
        if (_a->avail == (jsuword)(p) + JS_ARENA_ALIGN(pool, size)) {         \
            size_t _nb = (size) + (incr);                                     \
            _nb = JS_ARENA_ALIGN(pool, _nb);                                  \
            if (_a->limit >= _nb && (jsuword)(p) <= _a->limit - _nb) {        \
                _a->avail = (jsuword)(p) + _nb;                               \
                JS_ArenaCountInplaceGrowth(pool, size, incr);                 \
            } else if ((jsuword)(p) == _a->base) {                            \
                p = (type) JS_ArenaRealloc(pool, p, size, incr);              \
            } else {                                                          \
                p = (type) JS_ArenaGrow(pool, p, size, incr);                 \
            }                                                                 \
        } else {                                                              \
            p = (type) JS_ArenaGrow(pool, p, size, incr);                     \
        }                                                                     \
        JS_ArenaCountGrowth(pool, size, incr);                                \
    JS_END_MACRO

#define JS_ARENA_MARK(pool)     ((void *) (pool)->current->avail)
#define JS_UPTRDIFF(p,q)        ((jsuword)(p) - (jsuword)(q))

#define JS_ARENA_RELEASE(pool, mark)                                          \
    JS_BEGIN_MACRO                                                            \
        char *_m = (char *)(mark);                                            \
        JSArena *_a = (pool)->current;                                        \
        if (_a != &(pool)->first &&                                           \
            JS_UPTRDIFF(_m, _a->base) <= JS_UPTRDIFF(_a->avail, _a->base)) {  \
            _a->avail = (jsuword)JS_ARENA_ALIGN(pool, _m);                    \
            JS_ASSERT(_a->avail <= _a->limit);                                \
            JS_ArenaCountRetract(pool, _m);                                   \
        } else {                                                              \
            JS_ArenaRelease(pool, _m);                                        \
        }                                                                     \
        JS_ArenaCountRelease(pool, _m);                                       \
    JS_END_MACRO

#ifdef JS_ARENAMETER
#define JS_COUNT_ARENA(pool,op) ((pool)->stats.narenas op)
#else
#define JS_COUNT_ARENA(pool,op)
#endif

#define JS_ARENA_DESTROY(pool, a, pnext)                                      \
    JS_BEGIN_MACRO                                                            \
        JS_COUNT_ARENA(pool,--);                                              \
        if ((pool)->current == (a)) (pool)->current = &(pool)->first;         \
        *(pnext) = (a)->next;                                                 \
        free(a);                                                              \
        (a) = NULL;                                                           \
    JS_END_MACRO

/*
 * Initialize an arena pool with the given name for debugging and metering,
 * with a minimum size per arena of size bytes.
 */
extern JS_PUBLIC_API(void)
JS_InitArenaPool(JSArenaPool *pool, const char *name, size_t size,
                 size_t align);

/*
 * Free the arenas in pool.  The user may continue to allocate from pool
 * after calling this function.  There is no need to call JS_InitArenaPool()
 * again unless JS_FinishArenaPool(pool) has been called.
 */
extern JS_PUBLIC_API(void)
JS_FreeArenaPool(JSArenaPool *pool);

/*
 * Free the arenas in pool and finish using it altogether.
 */
extern JS_PUBLIC_API(void)
JS_FinishArenaPool(JSArenaPool *pool);

/*
 * Finish using arenas, freeing all memory associated with them except for
 * any locks needed for thread safety.
 */
extern JS_PUBLIC_API(void)
JS_ArenaFinish(void);

/*
 * Free any locks or other memory needed for thread safety, just before
 * shutting down.  At that point, we must be called by a single thread.
 *
 * After shutting down, the next thread to call JS_InitArenaPool must not
 * race with any other thread.  Once a pool has been initialized, threads
 * may safely call jsarena.c functions on thread-local pools.  The upshot
 * is that pools are per-thread, but the underlying global freelist is
 * thread-safe, provided that both the first pool initialization and the
 * shut-down call are single-threaded.
 */
extern JS_PUBLIC_API(void)
JS_ArenaShutDown(void);

/*
 * Friend functions used by the JS_ARENA_*() macros.
 */
extern JS_PUBLIC_API(void *)
JS_ArenaAllocate(JSArenaPool *pool, size_t nb);

extern JS_PUBLIC_API(void *)
JS_ArenaRealloc(JSArenaPool *pool, void *p, size_t size, size_t incr);

extern JS_PUBLIC_API(void *)
JS_ArenaGrow(JSArenaPool *pool, void *p, size_t size, size_t incr);

extern JS_PUBLIC_API(void)
JS_ArenaRelease(JSArenaPool *pool, char *mark);

/*
 * Function to be used directly when an allocation has likely grown to consume
 * an entire JSArena, in which case the arena is returned to the malloc heap.
 */
extern JS_PUBLIC_API(void)
JS_ArenaFreeAllocation(JSArenaPool *pool, void *p, size_t size);

#ifdef JS_ARENAMETER

#include <stdio.h>

extern JS_PUBLIC_API(void)
JS_ArenaCountAllocation(JSArenaPool *pool, size_t nb);

extern JS_PUBLIC_API(void)
JS_ArenaCountInplaceGrowth(JSArenaPool *pool, size_t size, size_t incr);

extern JS_PUBLIC_API(void)
JS_ArenaCountGrowth(JSArenaPool *pool, size_t size, size_t incr);

extern JS_PUBLIC_API(void)
JS_ArenaCountRelease(JSArenaPool *pool, char *mark);

extern JS_PUBLIC_API(void)
JS_ArenaCountRetract(JSArenaPool *pool, char *mark);

extern JS_PUBLIC_API(void)
JS_DumpArenaStats(FILE *fp);

#else  /* !JS_ARENAMETER */

#define JS_ArenaCountAllocation(ap, nb)                 /* nothing */
#define JS_ArenaCountInplaceGrowth(ap, size, incr)      /* nothing */
#define JS_ArenaCountGrowth(ap, size, incr)             /* nothing */
#define JS_ArenaCountRelease(ap, mark)                  /* nothing */
#define JS_ArenaCountRetract(ap, mark)                  /* nothing */

#endif /* !JS_ARENAMETER */

JS_END_EXTERN_C

#endif /* jsarena_h___ */
