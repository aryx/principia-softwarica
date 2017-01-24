#include	"u.h"
#include	"../port/lib.h"
#include	"../port/error.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"

// empty callback for portkbd.c
void arch_setleds(Kbscan *kbscan) { USED(kbscan); }
