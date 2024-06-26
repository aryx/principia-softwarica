/*s: devices/keyboard/386/kbd.c */
/*
 * keyboard input
 */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

#include    "io.h"

enum {
    Data=       0x60,       /* data port */

    Status=     0x64,       /* status port */
     Inready=   0x01,       /*  input character ready */
     Outbusy=   0x02,       /*  output busy */
     Sysflag=   0x04,       /*  system flag */
     Cmddata=   0x08,       /*  cmd==0, data==1 */
     Inhibit=   0x10,       /*  keyboard/mouse inhibited */
     Minready=  0x20,       /*  mouse character ready */
     Rtimeout=  0x40,       /*  general timeout */
     Parity=    0x80,

    Cmd=        0x64,       /* command port (write only) */
};

enum
{
    /* controller command byte */
    Cscs1=      (1<<6),     /* scan code set 1 */
    Cauxdis=    (1<<5),     /* mouse disable */
    Ckbddis=    (1<<4),     /* kbd disable */
    Csf=        (1<<2),     /* system flag */
    Cauxint=    (1<<1),     /* mouse interrupt enable */
    Ckbdint=    (1<<0),     /* kbd interrupt enable */
};

/*s: global [[i8042lock]](x86) */
static Lock i8042lock;
/*e: global [[i8042lock]](x86) */
/*s: global [[nokbd]](x86) */
static bool nokbd = true;           /* flag: no PS/2 keyboard */
/*e: global [[nokbd]](x86) */

/*s: global [[ccc]](x86) */
static byte ccc;
/*e: global [[ccc]](x86) */
/*s: hook auxputc(x86) */
static void (*auxputc)(int, int);
/*e: hook auxputc(x86) */

static char *initfailed = "i8042: kbdinit failed\n";

/*s: function [[outready]](x86) */
/*
 *  wait for output no longer busy
 */
static int
outready(void)
{
    int tries;

    for(tries = 0; (inb(Status) & Outbusy); tries++){
        if(tries > 500)
            return -1;
        arch_delay(2);
    }
    return 0;
}
/*e: function [[outready]](x86) */

/*s: function [[inready]](x86) */
/*
 *  wait for input
 */
static int
inready(void)
{
    int tries;

    for(tries = 0; !(inb(Status) & Inready); tries++){
        if(tries > 500)
            return -1;
        arch_delay(2);
    }
    return 0;
}
/*e: function [[inready]](x86) */

/*s: function [[i8042reset]](x86) */
/*
 *  ask 8042 to reset the machine
 */
void
i8042reset(void)
{
    int i, x;

    if(nokbd)
        return;

    *((ushort*)KADDR(0x472)) = 0x1234;  /* BIOS warm-boot flag */

    /*
     *  newer reset the machine command
     */
    outready();
    outb(Cmd, 0xFE);
    outready();

    /*
     *  Pulse it by hand (old somewhat reliable)
     */
    x = 0xDF;
    for(i = 0; i < 5; i++){
        x ^= 1;
        outready();
        outb(Cmd, 0xD1);
        outready();
        outb(Data, x);  /* toggle reset */
        arch_delay(100);
    }
}
/*e: function [[i8042reset]](x86) */

/*s: function [[i8042auxcmd]](x86) */
int
i8042auxcmd(int cmd)
{
    unsigned int c;
    int tries;
    static bool badkbd;

    if(badkbd)
        return -1;
    c = 0;
    tries = 0;

    ilock(&i8042lock);
    do{
        if(tries++ > 2)
            break;
        if(outready() < 0)
            break;
        outb(Cmd, 0xD4);
        if(outready() < 0)
            break;
        outb(Data, cmd);
        if(outready() < 0)
            break;
        if(inready() < 0)
            break;
        c = inb(Data);
    } while(c == 0xFE || c == 0);
    iunlock(&i8042lock);

    if(c != 0xFA){
        print("i8042: %2.2ux returned to the %2.2ux command\n", c, cmd);
        badkbd = true; /* don't keep trying; there might not be one */
        return -1;
    }
    return 0;
}
/*e: function [[i8042auxcmd]](x86) */

/*s: function [[setleds]](x86) */
/*
 * set keyboard's leds for lock states (scroll, numeric, caps).
 *
 * at least one keyboard (from Qtronics) also sets its numeric-lock
 * behaviour to match the led state, though it has no numeric keypad,
 * and some BIOSes bring the system up with numeric-lock set and no
 * setting to change that.  this combination steals the keys for these
 * characters and makes it impossible to generate them: uiolkjm&*().
 * thus we'd like to be able to force the numeric-lock led (and behaviour) off.
 */
void
arch_setleds(Kbscan *kbscan)
{
    int leds;

    if(nokbd || kbscan != &kbscans[KbInt])
        return;
    leds = 0;
    if(kbscan->num)
        leds |= 1<<1;
    if(0 && kbscan->caps)       /* we don't implement caps lock */
        leds |= 1<<2;

    ilock(&i8042lock);
    outready();
    outb(Data, 0xed);       /* `reset keyboard lock states' */
    if(inready() == 0)
        inb(Data);

    outready();
    outb(Data, leds);
    if(inready() == 0)
        inb(Data);

    outready();
    iunlock(&i8042lock);
}
/*e: function [[setleds]](x86) */

/*s: interrupt callback i8042intr(x86) */
/*
 *  keyboard interrupt
 */
static void
i8042intr(Ureg*, void*)
{
    byte s, c;

    /*
     *  get status
     */
    ilock(&i8042lock);
    s = inb(Status);
    if(!(s&Inready)){
        iunlock(&i8042lock);
        return;
    }

    /*
     *  get the character
     */
    c = inb(Data);
    iunlock(&i8042lock);

    /*s: [[i8042intr()]] aux port handling(x86) */
        /*
         *  if it's the aux port...
         */
        if(s & Minready){
            if(auxputc != nil)
                auxputc(c, kbscans[KbInt].shift);
            return;
        }
    /*e: [[i8042intr()]] aux port handling(x86) */
    // !!!
    kbdputsc(c, KbInt);
}
/*e: interrupt callback i8042intr(x86) */

/*s: function [[i8042auxenable]](x86) */
void
i8042auxenable(void (*putc)(int, int))
{
    char *err = "i8042: aux init failed\n";

    /* enable kbd/aux xfers and interrupts */
    ccc &= ~Cauxdis;
    ccc |= Cauxint;

    ilock(&i8042lock);
    if(outready() < 0)
        print(err);
    outb(Cmd, 0x60);            /* write control register */
    if(outready() < 0)
        print(err);
    outb(Data, ccc);
    if(outready() < 0)
        print(err);
    outb(Cmd, 0xA8);            /* auxiliary device enable */
    if(outready() < 0){
        iunlock(&i8042lock);
        return;
    }
    auxputc = putc;
    arch_intrenable(IrqAUX, i8042intr, 0, BUSUNKNOWN, "kbdaux");
    iunlock(&i8042lock);
}
/*e: function [[i8042auxenable]](x86) */

/*s: function [[outbyte]](x86) */
static int
outbyte(int port, int c)
{
    outb(port, c);
    if(outready() < 0) {
        print(initfailed);
        return -1;
    }
    return 0;
}
/*e: function [[outbyte]](x86) */

/*s: function [[kbdinit]](x86) */
void
kbdinit(void)
{
    int c, try;

    /* wait for a quiescent controller */
    try = 500;
    while(try-- > 0 && (c = inb(Status)) & (Outbusy | Inready)) {
        if(c & Inready)
            inb(Data);
        arch_delay(1);
    }
    if (try <= 0) {
        print(initfailed);
        return;
    }

    /* get current controller command byte */
    outb(Cmd, 0x20);
    if(inready() < 0){
        print("i8042: kbdinit can't read ccc\n");
        ccc = 0;
    } else
        ccc = inb(Data);

    /* enable kbd xfers and interrupts */
    ccc &= ~Ckbddis;
    ccc |= Csf | Ckbdint | Cscs1;
    if(outready() < 0) {
        print(initfailed);
        return;
    }

    nokbd = false;

    /* disable mouse */
    if (outbyte(Cmd, 0x60) < 0 || outbyte(Data, ccc) < 0)
        print("i8042: kbdinit mouse disable failed\n");

    /* see http://www.computer-engineering.org/ps2keyboard for codes */
    if(getconf("*typematic") != nil)
        /* set typematic rate/delay (0 -> delay=250ms & rate=30cps) */
        if(outbyte(Data, 0xf3) < 0 || outbyte(Data, 0) < 0)
            print("i8042: kbdinit set typematic rate failed\n");
}
/*e: function [[kbdinit]](x86) */

/*s: function [[kbdenable]](x86) */
void
kbdenable(void)
{
    ioalloc(Data, 1, 0, "kbd");
    ioalloc(Cmd, 1, 0, "kbd");

    arch_intrenable(IrqKBD, i8042intr, 0, BUSUNKNOWN, "kbd");

    kbscans[KbInt].num = false;
    arch_setleds(&kbscans[KbInt]);
}
/*e: function [[kbdenable]](x86) */

/*e: devices/keyboard/386/kbd.c */
