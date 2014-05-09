/*s: portdat_memory.c */
/*s: kernel basic includes */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
/*e: kernel basic includes */

/*s: global palloc */
// Page allocator
struct Palloc palloc;
/*e: global palloc */

/*s: global swapalloc */
// Swap allocator
struct Swapalloc swapalloc;
/*e: global swapalloc */
/*s: global swapimage */
KImage  swapimage;
/*e: global swapimage */

/*e: portdat_memory.c */
