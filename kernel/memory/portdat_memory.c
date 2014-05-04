/*s: portdat_memory.c */
/*s: kernel basic includes */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
/*e: kernel basic includes */

// Page allocator
struct Palloc palloc;

// Swap allocator
struct Swapalloc swapalloc;
KImage  swapimage;

/*e: portdat_memory.c */
