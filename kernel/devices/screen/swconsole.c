/*s: kernel/devices/screen/swconsole.c */
/*
 * Software cursor
 */
#include    "u.h"
#include    "../port/lib.h"
#include    "../port/error.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"

#include    <draw.h>
#include    <font.h>
#include    <memdraw.h>
#include    <memlayer.h>
#include    <cursor.h>

#include    "../port/portscreen.h"

enum {
    Scroll      = 8, // want this to be configurable?
    Tabstop     = 4,
};

Lock      swconsole_screenlock;

Point     curpos;
Rectangle window;

Memimage *conscol;
Memimage *back;
Memsubfont *memdefont;

static int h, w;

//static void myscreenputs(char *s, int n);
//static void screenputc(char *buf);

void swconsole_init(void)
{
    Rectangle r;

    back = memwhite;
    conscol = memblack;

    memdefont = getmemdefont();
    w = memdefont->info[' '].width;
    h = memdefont->height;

    r = insetrect(gscreen->r, 4);
    memimagedraw(gscreen, r, memblack, ZP, memopaque, ZP, S);
    window = insetrect(r, 4);
    memimagedraw(gscreen, window, memwhite, ZP, memopaque, ZP, S);
}

void
swconsole_scroll(void)
{
    int o;
    Point p;
    Rectangle r;

    o = Scroll*h;
    r = Rpt(window.min, Pt(window.max.x, window.max.y-o));
    p = Pt(window.min.x, window.min.y+o);
    memimagedraw(gscreen, r, gscreen, p, nil, p, S);
    arch_flushmemscreen(r);
    r = Rpt(Pt(window.min.x, window.max.y-o), window.max);
    memimagedraw(gscreen, r, back, ZP, nil, ZP, S);
    arch_flushmemscreen(r);

    curpos.y -= o;
}


void
swconsole_screenputc(char *buf)
{
    int w;
    uint pos;
    Point p;
    Rectangle r;
    static int *xp;
    static int xbuf[256];

    if (xp < xbuf || xp >= &xbuf[sizeof(xbuf)])
        xp = xbuf;

    switch (buf[0]) {
    case '\n':
        if (curpos.y + h >= window.max.y)
            swconsole_scroll();
        curpos.y += h;
        swconsole_screenputc("\r");
        break;
    case '\r':
        xp = xbuf;
        curpos.x = window.min.x;
        break;
    case '\t':
        p = memsubfontwidth(memdefont, " ");
        w = p.x;
        if (curpos.x >= window.max.x - Tabstop * w)
            swconsole_screenputc("\n");

        pos = (curpos.x - window.min.x) / w;
        pos = Tabstop - pos % Tabstop;
        *xp++ = curpos.x;
        r = Rect(curpos.x, curpos.y, curpos.x + pos * w, curpos.y + h);
        memimagedraw(gscreen, r, back, back->r.min, nil, back->r.min, S);
        arch_flushmemscreen(r);
        curpos.x += pos * w;
        break;
    case '\b':
        if (xp <= xbuf)
            break;
        xp--;
        r = Rect(*xp, curpos.y, curpos.x, curpos.y + h);
        memimagedraw(gscreen, r, back, back->r.min, nil, back->r.min, S);
        arch_flushmemscreen(r);
        curpos.x = *xp;
        break;
    case '\0':
        break;
    default:
        p = memsubfontwidth(memdefont, buf);
        w = p.x;

        if (curpos.x >= window.max.x - w)
            swconsole_screenputc("\n");

        *xp++ = curpos.x;
        r = Rect(curpos.x, curpos.y, curpos.x + w, curpos.y + h);
        memimagedraw(gscreen, r, back, back->r.min, nil, back->r.min, S);
        memimagestring(gscreen, curpos, conscol, ZP, memdefont, buf);
        arch_flushmemscreen(r);
        curpos.x += w;
        break;
    }
}

void
swconsole_screenputs(char *s, int n)
{
    int i;
    Rune r;
    char buf[4];

    if(!arch_islo()) {
        /* don't deadlock trying to print in interrupt */
        if(!canlock(&swconsole_screenlock))
            return; 
    }
    else
        lock(&swconsole_screenlock);

    while(n > 0){
        i = chartorune(&r, s);
        if(i == 0){
            s++;
            --n;
            continue;
        }
        memmove(buf, s, i);
        buf[i] = 0;
        n -= i;
        s += i;
        swconsole_screenputc(buf);
    }
    unlock(&swconsole_screenlock);
}



/*e: kernel/devices/screen/swconsole.c */
