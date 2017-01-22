/*s: cga.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

/*s: cga.c enum color(x86) */
enum color {
    Black,
    Blue,
    Green,
    Cyan,
    Red,
    Magenta,
    Brown,
    Grey,

    Bright = 0x08,
    Blinking = 0x80,

    Yellow = Bright|Brown,
    White = Bright|Grey,
};
/*e: cga.c enum color(x86) */

/*s: cga.c enum misc(x86) */
enum {
    Width       = 80*2,
    Height      = 25,

    Attr        = (Black<<4)|Grey,  /* high nibble background
                         * low foreground
                         */

    Poststrlen  = 0,
    Postcodelen = 2,
    Postlen     = Poststrlen+Postcodelen,
};
/*e: cga.c enum misc(x86) */

/*s: constant CGASCREENBASE(x86) */
#define CGASCREENBASE   ((byte*)KADDR(0xB8000))
/*e: constant CGASCREENBASE(x86) */

/*s: global cgapos(x86) */
static int cgapos;
/*e: global cgapos(x86) */
/*s: global cgascreenlock(x86) */
static Lock cgascreenlock;
/*e: global cgascreenlock(x86) */

/*s: function cgaregr(x86) */
static byte
cgaregr(int index)
{
    outb(0x3D4, index);
    return inb(0x3D4+1) & 0xFF;
}
/*e: function cgaregr(x86) */

/*s: function cgaregw(x86) */
static void
cgaregw(int index, int data)
{
    outb(0x3D4, index);
    outb(0x3D4+1, data);
}
/*e: function cgaregw(x86) */

/*s: function movecursor(x86) */
static void
movecursor(void)
{
    cgaregw(0x0E, (cgapos/2>>8) & 0xFF);
    cgaregw(0x0F, cgapos/2 & 0xFF);
    CGASCREENBASE[cgapos+1] = Attr;
}
/*e: function movecursor(x86) */

/*s: function cgascreenputc(x86) */
static void
cgascreenputc(char c)
{
    int i;
    byte *p;

    if(c == '\n'){
        cgapos = cgapos/Width;
        cgapos = (cgapos+1)*Width;
    }
    else if(c == '\t'){
        i = 8 - ((cgapos/2)&7);
        while(i-->0)
            cgascreenputc(' ');
    }
    else if(c == '\b'){
        if(cgapos >= 2)
            cgapos -= 2;
        cgascreenputc(' ');
        cgapos -= 2;
    }
    else{
        CGASCREENBASE[cgapos++] = c;
        CGASCREENBASE[cgapos++] = Attr;
    }
    if(cgapos >= Width*Height){
        memmove(CGASCREENBASE, &CGASCREENBASE[Width], Width*(Height-1));
        p = &CGASCREENBASE[Width*(Height-1)];
        for(i=0; i<Width/2; i++){
            *p++ = ' ';
            *p++ = Attr;
        }
        cgapos = Width*(Height-1);
    }
    movecursor();
}
/*e: function cgascreenputc(x86) */

/*s: function cgascreenputs(x86) */
static void
cgascreenputs(char* s, int n)
{
    if(!arch_islo()){
        /*
         * Don't deadlock trying to
         * print in an interrupt.
         */
        if(!canlock(&cgascreenlock))
            return;
    }
    else
        lock(&cgascreenlock);

    while(n-- > 0)
        cgascreenputc(*s++);

    unlock(&cgascreenlock);
}
/*e: function cgascreenputs(x86) */

/*s: function cgapost(x86) */
char hex[] = "0123456789ABCDEF";

void
cgapost(int code)
{
    byte *cga;

    cga = CGASCREENBASE;
    cga[Width*Height-Postcodelen*2] = hex[(code>>4) & 0x0F];
    cga[Width*Height-Postcodelen*2+1] = Attr;
    cga[Width*Height-Postcodelen*2+2] = hex[code & 0x0F];
    cga[Width*Height-Postcodelen*2+3] = Attr;
}
/*e: function cgapost(x86) */

/*s: function screeninit(x86) */
void
screeninit(void)
{

    cgapos = cgaregr(0x0E)<<8;
    cgapos |= cgaregr(0x0F);
    cgapos *= 2;

    screenputs = cgascreenputs;
}
/*e: function screeninit(x86) */
/*e: cga.c */
