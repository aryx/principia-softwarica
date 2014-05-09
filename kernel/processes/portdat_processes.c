/*s: portdat_processes.c */
/*s: kernel basic includes */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
/*e: kernel basic includes */

/*s: hook proctrace */
void (*proctrace)(Proc*, int, vlong) = 0; // was in devproc.c
/*e: hook proctrace */
/*s: hook kproftimer */
void (*kproftimer)(ulong);
/*e: hook kproftimer */

/*s: global active */
struct Active active;
/*e: global active */
/*e: portdat_processes.c */
