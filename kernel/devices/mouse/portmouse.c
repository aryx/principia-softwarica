/*s: portmouse.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

// enum<MouseType>
int mousetype;

int packetsize;
static int resolution;
static int accelerated;

bool mousehwaccel;
bool intellimouse;

static QLock mousectlqlock;

// for serial mouse
//static char mouseport[5];

enum
{
    CMaccelerated,
    CMhwaccel,
    CMintellimouse,
    CMlinear,
    CMps2,
    CMps2intellimouse,
    CMres,
    CMreset,
    CMserial,
};

static Cmdtab mousectlmsg[] =
{
    CMaccelerated     , "accelerated"     , 0 ,
    CMhwaccel         , "hwaccel"         , 2 ,
    CMintellimouse    , "intellimouse"    , 1 ,
    CMlinear          , "linear"          , 1 ,
    CMps2             , "ps2"             , 1 ,
    CMps2intellimouse , "ps2intellimouse" , 1 ,
    CMres             , "res"             , 0 ,
    CMreset           , "reset"           , 1 ,
    CMserial          , "serial"          , 0 ,
};


/*
 *  set up a ps2 mouse
 */
static void
ps2mouse(void)
{
    if(mousetype == MousePS2)
        return;
    arch_ps2mouse();

    mousetype = MousePS2;
    packetsize = 3;
    mousehwaccel = 1;
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
    arch_setaccelerated();
    // call in devmouse.c
    mouseaccelerate(x);
}

static void
setlinear(void)
{
    accelerated = 0;
    arch_setlinear();
    mouseaccelerate(0);
}

static void
setres(int n)
{
    resolution = n;
    arch_setres(n);
}

static void
setintellimouse(void)
{
    intellimouse = 1;
    packetsize = 4;
    arch_setintellimouse();
}

static void
resetmouse(void)
{
    packetsize = 3;
    arch_resetmouse();
}

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
        setaccelerated(cb->nf == 1 ? 1: atoi(cb->f[1]));
        break;
    case CMlinear:
        setlinear();
        break;
    case CMps2:
        intellimouse = 0;
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
        if(intellimouse)
            setintellimouse();
        break;
    case CMhwaccel:
        if(strcmp(cb->f[1], "on")==0)
            mousehwaccel = 1;
        else if(strcmp(cb->f[1], "off")==0)
            mousehwaccel = 0;
        else
            cmderror(cb, "bad mouse control message");
        break;
    case CMintellimouse:
        setintellimouse();
        break;
    case CMps2intellimouse:
        ps2mouse();
        setintellimouse();
        break;
    case CMserial:
        //if(mousetype == Mouseserial)
        //  error(Emouseset);
        //
        //if(cb->nf > 2){
        //  if(strcmp(cb->f[2], "M") == 0)
        //      i8250mouse(cb->f[1], m3mouseputc, 0);
        //  else if(strcmp(cb->f[2], "MI") == 0)
        //      i8250mouse(cb->f[1], m5mouseputc, 0);
        //  else
        //      i8250mouse(cb->f[1], mouseputc, cb->nf == 1);
        //} else
        //  i8250mouse(cb->f[1], mouseputc, cb->nf == 1);
        //
        //mousetype = Mouseserial;
        //strncpy(mouseport, cb->f[1], sizeof(mouseport)-1);
        //packetsize = 3;
        error("serial mice not supported anymore");
        break;
    }
    qunlock(&mousectlqlock);
    poperror();
}

/*e: portmouse.c */
