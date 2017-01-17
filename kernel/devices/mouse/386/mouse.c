/*s: mouse.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

#include "io.h"

#include <draw.h>
#include <memdraw.h>
#include <cursor.h>

#include "../port/portscreen.h"

/*
 *  mouse types
 */
enum
{
    Mouseother= 0,
    MousePS2=   1,
};

extern int mouseshifted;

static int mousetype;
static int packetsize;
static int resolution;
static int accelerated;
static bool mousehwaccel;

static QLock mousectlqlock;

enum
{
    CMaccelerated,
    CMhwaccel,
    CMlinear,
    CMps2,
    CMres,
    CMreset,
};

static Cmdtab mousectlmsg[] =
{
    CMaccelerated,      "accelerated",      0,
    CMhwaccel,      "hwaccel",      2,
    CMlinear,       "linear",       1,
    CMps2,          "ps2",          1,
    CMres,          "res",          0,
    CMreset,        "reset",        1,
};

/*
 *  ps/2 mouse message is three bytes
 *
 *  byte 0 -    0 0 SDY SDX 1 M R L
 *  byte 1 -    DX
 *  byte 2 -    DY
 *
 *  shift & right button is the same as middle button
 *
 */
static void
ps2mouseputc(int c, int shift)
{
    static short msg[4];
    static int nb;
    static uchar b[] = {0, 1, 4, 5, 2, 3, 6, 7, 0, 1, 2, 3, 2, 3, 6, 7 };
    static ulong lasttick;
    ulong m;
    int buttons, dx, dy;

    /*
     * non-ps2 keyboards might not set shift
     * but still set mouseshifted.
     */
    shift |= mouseshifted;
    /*
     * Resynchronize in stream with timing; see comment above.
     */
    m = CPUS(0)->ticks;
    if(TK2SEC(m - lasttick) > 2)
        nb = 0;
    lasttick = m;

    msg[nb] = c;
    if(++nb == packetsize){
        nb = 0;
        if(msg[0] & 0x10)
            msg[1] |= 0xFF00;
        if(msg[0] & 0x20)
            msg[2] |= 0xFF00;

        buttons = b[(msg[0]&7) | (shift ? 8 : 0)];

        dx = msg[1];
        dy = -msg[2];
        mousetrack(dx, dy, buttons, TK2MS(CPUS(0)->ticks));
    }
    return;
}

/*
 *  set up a ps2 mouse
 */
static void
ps2mouse(void)
{
    if(mousetype == MousePS2)
        return;

    i8042auxenable(ps2mouseputc);
    /* make mouse streaming, enabled */
    i8042auxcmd(0xEA);
    i8042auxcmd(0xF4);

    mousetype = MousePS2;
    packetsize = 3;
    mousehwaccel = true;
}

/*
 * The PS/2 Trackpoint multiplexor on the IBM Thinkpad T23 ignores
 * acceleration commands.  It is supposed to pass them on
 * to the attached device, but my Logitech mouse is simply
 * not behaving any differently.  For such devices, we allow
 * the user to use "hwaccel off" to tell us to back off to
 * software acceleration even if we're using the PS/2 port.
 * (Serial mice are always software accelerated.)
 * For more information on the Thinkpad multiplexor, see
 * http://wwwcssrv.almaden.ibm.com/trackpoint/
 */
static void
setaccelerated(int x)
{
    accelerated = x;
    if(mousehwaccel){
        switch(mousetype){
        case MousePS2:
            i8042auxcmd(0xE7);
            return;
        }
    }
    mouseaccelerate(x);
}

static void
setlinear(void)
{
    accelerated = 0;
    if(mousehwaccel){
        switch(mousetype){
        case MousePS2:
            i8042auxcmd(0xE6);
            return;
        }
    }
    mouseaccelerate(0);
}

static void
setres(int n)
{
    resolution = n;
    switch(mousetype){
    case MousePS2:
        i8042auxcmd(0xE8);
        i8042auxcmd(n);
        break;
    }
}


static void
resetmouse(void)
{
    packetsize = 3;
    switch(mousetype){
    case MousePS2:
        i8042auxcmd(0xF6);
        i8042auxcmd(0xEA);  /* streaming */
        i8042auxcmd(0xE8);  /* set resolution */
        i8042auxcmd(3);
        i8042auxcmd(0xF4);  /* enabled */
        break;
    }
}

/*s: function kmousectl(x86) */
// for screen.h
void
kmousectl(Cmdbuf *cb)
{
    Cmdtab *ct;

    qlock(&mousectlqlock);
    if(waserror()){
        qunlock(&mousectlqlock);
        nexterror();
    }

    ct = lookupcmd(cb, mousectlmsg, nelem(mousectlmsg));
    switch(ct->index){
    case CMaccelerated:
        setaccelerated(cb->nf == 1 ? 1 : atoi(cb->f[1]));
        break;
    case CMlinear:
        setlinear();
        break;
    case CMps2:
        ps2mouse();
        break;
    case CMres:
        if(cb->nf >= 2)
            setres(atoi(cb->f[1]));
        else
            setres(1);
        break;
    case CMreset:
        resetmouse();
        if(accelerated)
            setaccelerated(accelerated);
        if(resolution)
            setres(resolution);
        break;
    case CMhwaccel:
        if(strcmp(cb->f[1], "on")==0)
            mousehwaccel = true;
        else if(strcmp(cb->f[1], "off")==0)
            mousehwaccel = false;
        else
            cmderror(cb, "bad mouse control message");
    }

    qunlock(&mousectlqlock);
    poperror();
}
/*e: function kmousectl(x86) */
/*e: mouse.c */
