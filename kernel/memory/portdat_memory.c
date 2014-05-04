/*s: portdat_memory.c */
#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"

// Page allocator
struct Palloc palloc;

// Swap allocator
struct Swapalloc swapalloc;
KImage  swapimage;

/*e: portdat_memory.c */
