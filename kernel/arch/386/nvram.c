/*s: nvram.c */
/*s: kernel basic includes */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
/*e: kernel basic includes */

// used to be static
Lock nvrtlock;

uchar
nvramread(int addr)
{
    uchar data;

    ilock(&nvrtlock);
    outb(Paddr, addr);
    data = inb(PdataPort);
    iunlock(&nvrtlock);

    return data;
}

void
nvramwrite(int addr, uchar data)
{
    ilock(&nvrtlock);
    outb(Paddr, addr);
    outb(PdataPort, data);
    iunlock(&nvrtlock);
}
/*e: nvram.c */
