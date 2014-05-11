/*s: archgeneric.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

//todo: weird, if don't include this file, then can't declare arch in this file
// or I get some type signature mismatch
#include "io.h"

/*
 *  the following is a generic version of the
 *  architecture specific stuff
 */

/*s: function unimplemented */
static int
unimplemented(int)
{
    return 0;
}
/*e: function unimplemented */

/*s: function archreset */
static void
archreset(void)
{
    i8042reset();

    /*
     * Often the BIOS hangs during restart if a conventional 8042
     * warm-boot sequence is tried. The following is Intel specific and
     * seems to perform a cold-boot, but at least it comes back.
     * And sometimes there is no keyboard...
     *
     * The reset register (0xcf9) is usually in one of the bridge
     * chips. The actual location and sequence could be extracted from
     * ACPI but why bother, this is the end of the line anyway.
     */
    print("Takes a licking and keeps on ticking...\n");
    *(ushort*)KADDR(0x472) = 0x1234;    /* BIOS warm-boot flag */
    outb(0xcf9, 0x02);
    outb(0xcf9, 0x06);

    for(;;)
        idle();
}
/*e: function archreset */

/*s: global archgeneric */
PCArch archgeneric = {
    .id=        "generic",
    .ident=     0,
    .reset=     archreset,
    .serialpower=   unimplemented,
    .modempower=    unimplemented,
    
    // i8259
    .intrinit=  i8259init,
    .intrenable=    i8259enable,
    .intrvecno= i8259vecno,
    .intrdisable=   i8259disable,
    .intron=    i8259on,
    .introff=   i8259off,
    
    // i8253
    .clockenable=   i8253enable,
    .fastclock= i8253read,
    .timerset=  i8253timerset,
};
/*e: global archgeneric */

/*s: function archrevert */
void
archrevert(void)
{
    arch = &archgeneric;
}
/*e: function archrevert */

/*e: archgeneric.c */
