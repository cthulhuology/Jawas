
#ifndef jsgc_h___
#define jsgc_h___
/*
 * JS Garbage Collector.
 */
#include "jsprvtd.h"
#include "jspubtd.h"
#include "jsdhash.h"

JS_BEGIN_EXTERN_C

/* GC thing type indexes. */
#define GCX_OBJECT              0               /* JSObject */
#define GCX_STRING              1               /* JSString */
#define GCX_DOUBLE              2               /* jsdouble */
#define GCX_MUTABLE_STRING      3               /* JSString that's mutable --
                                                   single-threaded only! */
#define GCX_PRIVATE             4               /* private (unscanned) data */
#define GCX_NAMESPACE           5               /* JSXMLNamespace */
#define GCX_QNAME               6               /* JSXMLQName */
#define GCX_XML                 7               /* JSXML */
#define GCX_EXTERNAL_STRING     8               /* JSString w/ external chars */

#define GCX_NTYPES_LOG2         4               /* type index bits */
#define GCX_NTYPES              JS_BIT(GCX_NTYPES_LOG2)

/* GC flag definitions, must fit in 8 bits (type index goes in the low bits). */
#define GCF_TYPEMASK    JS_BITMASK(GCX_NTYPES_LOG2)
#define GCF_MARK        JS_BIT(GCX_NTYPES_LOG2)
#define GCF_FINAL       JS_BIT(GCX_NTYPES_LOG2 + 1)
#define GCF_SYSTEM      JS_BIT(GCX_NTYPES_LOG2 + 2)
#define GCF_LOCKSHIFT   (GCX_NTYPES_LOG2 + 3)   /* lock bit shift */
#define GCF_LOCK        JS_BIT(GCF_LOCKSHIFT)   /* lock request bit in API */

/* Pseudo-flag that modifies GCX_STRING to make GCX_MUTABLE_STRING. */
#define GCF_MUTABLE     2

#if (GCX_STRING | GCF_MUTABLE) != GCX_MUTABLE_STRING
# error "mutable string type index botch!"
#endif

extern uint8 *
js_GetGCThingFlags(void *thing);

/* These are compatible with JSDHashEntryStub. */
struct JSGCRootHashEntry {
    JSDHashEntryHdr hdr;
    void            *root;
    const char      *name;
};

struct JSGCLockHashEntry {
    JSDHashEntryHdr hdr;
    const JSGCThing *thing;
    uint32          count;
};

#if 1
/*
 * Since we're forcing a GC from JS_GC anyway, don't bother wasting cycles
 * loading oldval.  XXX remove implied force, fix jsinterp.c's "second arg
 * ignored", etc.
 */
#define GC_POKE(cx, oldval) ((cx)->runtime->gcPoke = JS_TRUE)
#else
#define GC_POKE(cx, oldval) ((cx)->runtime->gcPoke = JSVAL_IS_GCTHING(oldval))
#endif

extern intN
js_ChangeExternalStringFinalizer(JSStringFinalizeOp oldop,
                                 JSStringFinalizeOp newop);

extern JSBool
js_InitGC(JSRuntime *rt, uint32 maxbytes);

extern void
js_FinishGC(JSRuntime *rt);

extern JSBool
js_AddRoot(JSContext *cx, void *rp, const char *name);

extern JSBool
js_AddRootRT(JSRuntime *rt, void *rp, const char *name);

extern JSBool
js_RemoveRoot(JSRuntime *rt, void *rp);

/*
 * The private JSGCThing struct, which describes a gcFreeList element.
 */
struct JSGCThing {
    JSGCThing   *next;
    uint8       *flagp;
};

#define GC_NBYTES_MAX           (10 * sizeof(JSGCThing))
#define GC_NUM_FREELISTS        (GC_NBYTES_MAX / sizeof(JSGCThing))
#define GC_FREELIST_NBYTES(i)   (((i) + 1) * sizeof(JSGCThing))
#define GC_FREELIST_INDEX(n)    (((n) / sizeof(JSGCThing)) - 1)

extern void *
js_NewGCThing(JSContext *cx, uintN flags, size_t nbytes);

extern JSBool
js_LockGCThing(JSContext *cx, void *thing);

extern JSBool
js_LockGCThingRT(JSRuntime *rt, void *thing);

extern JSBool
js_UnlockGCThingRT(JSRuntime *rt, void *thing);

extern JSBool
js_IsAboutToBeFinalized(JSContext *cx, void *thing);

extern void
js_MarkAtom(JSContext *cx, JSAtom *atom, void *arg);

/* We avoid a large number of unnecessary calls by doing the flag check first */
#define GC_MARK_ATOM(cx, atom, arg)                                           \
    JS_BEGIN_MACRO                                                            \
        if (!((atom)->flags & ATOM_MARK))                                     \
            js_MarkAtom(cx, atom, arg);                                       \
    JS_END_MACRO

extern void
js_MarkGCThing(JSContext *cx, void *thing, void *arg);

#define GC_MARK(cx, thing, name, prev)   js_MarkGCThing(cx, thing, NULL)

/*
 * Flags to modify how a GC marks and sweeps:
 *   GC_KEEP_ATOMS      Don't sweep unmarked atoms, they may be in use by the
 *                      compiler, or by an API function that calls js_Atomize,
 *                      when the GC is called from js_NewGCThing, due to a
 *                      malloc failure or the runtime GC-thing limit.
 *   GC_LAST_CONTEXT    Called from js_DestroyContext for last JSContext in a
 *                      JSRuntime, when it is imperative that rt->gcPoke gets
 *                      cleared early in js_GC, if it is set.
 *   GC_ALREADY_LOCKED  rt->gcLock is already held on entry to js_GC, and kept
 *                      on return to its caller.
 */
#define GC_KEEP_ATOMS       0x1
#define GC_LAST_CONTEXT     0x2
#define GC_ALREADY_LOCKED   0x4

extern void
js_ForceGC(JSContext *cx, uintN gcflags);

extern void
js_GC(JSContext *cx, uintN gcflags);

#ifdef JS_GCMETER

typedef struct JSGCStats {
    uint32  alloc;      /* number of allocation attempts */
    uint32  freelen[GC_NUM_FREELISTS];
                        /* gcFreeList lengths */
    uint32  recycle[GC_NUM_FREELISTS];
                        /* number of things recycled through gcFreeList */
    uint32  retry;      /* allocation attempt retries after running the GC */
    uint32  retryhalt;  /* allocation retries halted by the branch callback */
    uint32  fail;       /* allocation failures */
    uint32  finalfail;  /* finalizer calls allocator failures */
    uint32  lockborn;   /* things born locked */
    uint32  lock;       /* valid lock calls */
    uint32  unlock;     /* valid unlock calls */
    uint32  depth;      /* mark tail recursion depth */
    uint32  maxdepth;   /* maximum mark tail recursion depth */
    uint32  cdepth;     /* mark recursion depth of C functions */
    uint32  maxcdepth;  /* maximum mark recursion depth of C functions */
    uint32  dswmark;    /* mark C stack overflows => Deutsch-Schorr-Waite */
    uint32  dswdepth;   /* DSW mark depth */
    uint32  maxdswdepth;/* maximum DSW mark depth */
    uint32  dswup;      /* DSW moves up the mark spanning tree */
    uint32  dswupstep;  /* steps in obj->slots to find DSW-reversed pointer */
    uint32  maxlevel;   /* maximum GC nesting (indirect recursion) level */
    uint32  poke;       /* number of potentially useful GC calls */
    uint32  nopoke;     /* useless GC calls where js_PokeGC was not set */
    uint32  afree;      /* thing arenas freed so far */
    uint32  stackseg;   /* total extraordinary stack segments scanned */
    uint32  segslots;   /* total stack segment jsval slots scanned */
} JSGCStats;

extern JS_FRIEND_API(void)
js_DumpGCStats(JSRuntime *rt, FILE *fp);

#endif /* JS_GCMETER */

#ifdef WAY_TOO_MUCH_GC
#define TOO_MUCH_GC 1
#endif

JS_END_EXTERN_C

#endif /* jsgc_h___ */
