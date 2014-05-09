/*s: cga.c */
/*s: kernel basic includes */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
/*e: kernel basic includes */

/*s: cga.c enum color */
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
/*e: cga.c enum color */

/*s: cga.c enum misc */
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
/*e: cga.c enum misc */

/*s: constant CGASCREENBASE */
#define CGASCREENBASE   ((uchar*)KADDR(0xB8000))
/*e: constant CGASCREENBASE */

/*s: global cgapos */
static int cgapos;
/*e: global cgapos */
/*s: global cgascreenlock */
static Lock cgascreenlock;
/*e: global cgascreenlock */

/*s: function cgaregr */
static uchar
cgaregr(int index)
{
    outb(0x3D4, index);
    return inb(0x3D4+1) & 0xFF;
}
/*e: function cgaregr */

/*s: function cgaregw */
static void
cgaregw(int index, int data)
{
    outb(0x3D4, index);
    outb(0x3D4+1, data);
}
/*e: function cgaregw */

/*s: function movecursor */
static void
movecursor(void)
{
    cgaregw(0x0E, (cgapos/2>>8) & 0xFF);
    cgaregw(0x0F, cgapos/2 & 0xFF);
    CGASCREENBASE[cgapos+1] = Attr;
}
/*e: function movecursor */

/*s: function cgascreenputc */
static void
cgascreenputc(int c)
{
    int i;
    uchar *p;

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
/*e: function cgascreenputc */

/*s: function cgascreenputs */
static void
cgascreenputs(char* s, int n)
{
    if(!islo()){
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
/*e: function cgascreenputs */

/*s: function cgapost */
char hex[] = "0123456789ABCDEF";

void
cgapost(int code)
{
    uchar *cga;

    cga = CGASCREENBASE;
    cga[Width*Height-Postcodelen*2] = hex[(code>>4) & 0x0F];
    cga[Width*Height-Postcodelen*2+1] = Attr;
    cga[Width*Height-Postcodelen*2+2] = hex[code & 0x0F];
    cga[Width*Height-Postcodelen*2+3] = Attr;
}
/*e: function cgapost */

/*s: function screeninit */
void
screeninit(void)
{

    cgapos = cgaregr(0x0E)<<8;
    cgapos |= cgaregr(0x0F);
    cgapos *= 2;

    screenputs = cgascreenputs;
}
/*e: function screeninit */
/*e: cga.c */
