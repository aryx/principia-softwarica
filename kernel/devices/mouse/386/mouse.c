/*s: mouse.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

/*
 *  ps/2 mouse message is three bytes
 *
 *  byte 0 -    0 0 SDY SDX 1 M R L
 *  byte 1 -    DX
 *  byte 2 -    DY
 *
 *  shift & right button is the same as middle button
 *
 * Intellimouse and AccuPoint with extra buttons deliver
 *  byte 3 -    00 or 01 or FF according to extra button state.
 * extra buttons are mapped in this code to buttons 4 and 5.
 * AccuPoint generates repeated events for these buttons;
*  it and Intellimouse generate 'down' events only, so
 * user-level code is required to generate button 'up' events
 * if they are needed by the application.
 * Also on laptops with AccuPoint AND external mouse, the
 * controller may deliver 3 or 4 bytes according to the type
 * of the external mouse; code must adapt.
 *
 * On the NEC Versa series (and perhaps others?) we seem to
 * lose a byte from the packet every once in a while, which
 * means we lose where we are in the instruction stream.
 * To resynchronize, if we get a byte more than two seconds
 * after the previous byte, we assume it's the first in a packet.
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

    /* 
     *  check byte 0 for consistency
     */
    if(nb==0 && (c&0xc8)!=0x08)
        if(intellimouse && (c==0x00 || c==0x01 || c==0xFF)){
            /* last byte of 4-byte packet */
            packetsize = 4;
            return;
        }

    msg[nb] = c;
    if(++nb == packetsize){
        nb = 0;
        if(msg[0] & 0x10)
            msg[1] |= 0xFF00;
        if(msg[0] & 0x20)
            msg[2] |= 0xFF00;

        buttons = b[(msg[0]&7) | (shift ? 8 : 0)];
        if(intellimouse && packetsize==4){
            if((msg[3]&0xc8) == 0x08){
                /* first byte of 3-byte packet */
                packetsize = 3;
                msg[0] = msg[3];
                nb = 1;
                /* fall through to emit previous packet */
            }else{
                /* The AccuPoint on the Toshiba 34[48]0CT
                 * encodes extra buttons as 4 and 5. They repeat
                 * and don't release, however, so user-level
                 * timing code is required. Furthermore,
                 * intellimice with 3buttons + scroll give a
                 * two's complement number in the lower 4 bits
                 * (bit 4 is sign extension) that describes
                 * the amount the scroll wheel has moved during
                 * the last sample. Here we use only the sign to
                 * decide whether the wheel is moving up or down
                 * and generate a single button 4 or 5 click
                 * accordingly.
                 */
                if((msg[3] >> 3) & 1)
                    buttons |= 1<<3;
                else if(msg[3] & 0x7)
                    buttons |= 1<<4;
            }
        }
        dx = msg[1];
        dy = -msg[2];

        // call in devmouse.c
        mousetrack(dx, dy, buttons, TK2MS(CPUS(0)->ticks));
    }
}


void arch_ps2mouse(void)
{
    i8042auxenable(ps2mouseputc);

    /* make mouse streaming, enabled */
    i8042auxcmd(0xEA);
    i8042auxcmd(0xF4);
}

void arch_setaccelerated(void)
{
    if(mousehwaccel){
        switch(mousetype){
        case MousePS2:
            i8042auxcmd(0xE7);
            return;
        }
    }
}

void arch_setlinear(void)
{
    if(mousehwaccel){
        switch(mousetype){
        case MousePS2:
            i8042auxcmd(0xE6);
            return;
        }
    }
}


void arch_setres(int n)
{
    switch(mousetype){
    case MousePS2:
        i8042auxcmd(0xE8);
        i8042auxcmd(n);
        break;
    }
}


void arch_setintellimouse()
{
    switch(mousetype){
    case MousePS2:
        i8042auxcmd(0xF3);  /* set sample */
        i8042auxcmd(0xC8);
        i8042auxcmd(0xF3);  /* set sample */
        i8042auxcmd(0x64);
        i8042auxcmd(0xF3);  /* set sample */
        i8042auxcmd(0x50);
        break;
    case Mouseserial:
        //i8250setmouseputc(mouseport, m5mouseputc);
        break;
    }
}

void arch_resetmouse(void)
{
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

/*e: mouse.c */
