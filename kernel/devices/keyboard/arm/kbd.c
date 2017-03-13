/*s: devices/keyboard/arm/kbd.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

// In the Raspberry Pi, the keyboard is handled by usbd
// with help from /dev/kbin (which then calls functions from portkbd
// which calls arch_setleds below).

/*s: function arch_setleds(arm) */
// empty callback for portkbd.c
void arch_setleds(Kbscan *kbscan) { USED(kbscan); }
/*e: function arch_setleds(arm) */
/*e: devices/keyboard/arm/kbd.c */
