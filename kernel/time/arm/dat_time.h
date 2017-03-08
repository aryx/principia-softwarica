/*s: time/arm/dat_time.h */
/*
 * Time.
 *
 * HZ should divide 1000 evenly, ideally.
 * 100, 125, 200, 250 and 333 are okay.
 */
/*s: constant Arch_HZ(arm) */
#define Arch_HZ     100         /* clock frequency */
/*e: constant Arch_HZ(arm) */

// was vlong in x86
//typedef uvlong        Tval;
/*e: time/arm/dat_time.h */
