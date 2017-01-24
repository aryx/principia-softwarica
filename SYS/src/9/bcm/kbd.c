#include "u.h"
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

// In the Raspberry Pi the keyboard is handled by usbd
// with help from /dev/kbin

// empty callback for portkbd.c
void arch_setleds(Kbscan *kbscan) { USED(kbscan); }
