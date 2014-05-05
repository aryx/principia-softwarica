
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"

// partial dupe of kbd.c
enum {
    Spec=       0xF800,     /* Unicode private space */

    No=     0x00,       /* peter */
    Shift=      Spec|0x60,
    Break=      Spec|0x61,
    Ctrl=       Spec|0x62,
    Latin=      Spec|0x63,

    Nscan=  128,
};

Rune kbtabctrl[Nscan] =
{
[0x00]  No, '',   '',   '',   '',   '',   '',   '', 
[0x08]  '',   '',   '',   '',   '',   '',   '\b',   '\t',
[0x10]  '',   '',   '',   '',   '',   '',   '',   '\t',
[0x18]  '',   '',   '',   '',   '\n',   Ctrl,   '',   '', 
[0x20]  '',   '',   '',   '\b',   '\n',   '',   '',   '', 
[0x28]  '',   No,     Shift,  '',   '',   '',   '',   '', 
[0x30]  '',   '',   '',   '',   '',   '',   Shift,  '\n',
[0x38]  Latin,  No,     Ctrl,   '',   '',   '',   '',   '', 
[0x40]  '',   '',   '',   '',   '',   '',   '',   '', 
[0x48]  '',   '',   '',   '',   '',   '',   '',   '', 
[0x50]  '',   '',   '',   '',   No, No, No, '', 
[0x58]  '',   No, No, No, No, No, No, No,
[0x60]  No, No, No, No, No, No, No, No,
[0x68]  No, No, No, No, No, No, No, No,
[0x70]  No, No, No, No, No, No, No, No,
[0x78]  No, '',   No, '\b',   No, No, No, No,
};
