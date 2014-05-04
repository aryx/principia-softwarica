/*s: portdat_processes.c */
#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"

void (*proctrace)(Proc*, int, vlong) = 0; // was in devproc.c
void (*kproftimer)(ulong);

struct Active active;
/*e: portdat_processes.c */
