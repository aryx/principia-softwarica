/*s: portdat_processes.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

/*s: hook proctrace */
void (*proctrace)(Proc*, /*enum<tevent>*/int, vlong) = 0; // was in devproc.c
/*e: hook proctrace */
/*s: hook kproftimer */
void (*kproftimer)(ulong);
/*e: hook kproftimer */

/*s: global [[active]] */
struct Active active;
/*e: global [[active]] */
/*e: portdat_processes.c */
