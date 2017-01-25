/*s: kernel/devices/screen/swconsole.c */
/*
 * Text output using memdraw
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

// The main entry point of this module is the function 
// swconsole_screenputs() which is assigned to screenputs in devcons.c and
// called from print() to display text on the screen.
// swconsole_screenputs() uses the default text font in memdraw
// and the memdraw functions to draw characters on the screen (via gscreen).
// See pc/cga.c for another screenputs function using the CGA hardware
// to display text.

enum {
    Scroll      = 8, // want this to be configurable?
    Tabstop     = 4,
};

/*s: global swconsole_screenlock */
Lock      swconsole_screenlock;
/*e: global swconsole_screenlock */

/*s: global curpos */
Point     swconsole_curpos;
/*e: global curpos */

/*s: global window bis */
Rectangle swconsole_window;
/*e: global window bis */

/*s: global conscol */
Memimage *swconsole_conscol;
/*e: global conscol */

/*s: global back2 */
Memimage *swconsole_back;
/*e: global back2 */

/*s: global memdefont2 */
Memsubfont *swconsole_memdefont;
/*e: global memdefont2 */

/*s: global swconsole h w */
static int h, w;
/*e: global swconsole h w */

/*s: function swconsole_init */
void swconsole_init(void)
{
    Rectangle r;

    swconsole_back = memwhite;
    swconsole_conscol = memblack;

    swconsole_memdefont = getmemdefont();
    w = swconsole_memdefont->info[' '].width;
    h = swconsole_memdefont->height;

    r = insetrect(gscreen->r, 4);
    memimagedraw(gscreen, r, memblack, ZP, memopaque, ZP, S);
    swconsole_window = insetrect(r, 4);
    memimagedraw(gscreen, swconsole_window, memwhite, ZP, memopaque, ZP, S);
}
/*e: function swconsole_init */

/*s: function swconsole_scroll */
void
swconsole_scroll(void)
{
    int o;
    Point p;
    Rectangle r;

    o = Scroll*h;
    r = Rpt(swconsole_window.min, 
            Pt(swconsole_window.max.x, swconsole_window.max.y-o));
    p = Pt(swconsole_window.min.x, swconsole_window.min.y+o);
    memimagedraw(gscreen, r, gscreen, p, nil, p, S);
    arch_flushmemscreen(r);
    r = Rpt(Pt(swconsole_window.min.x, swconsole_window.max.y-o), 
            swconsole_window.max);
    memimagedraw(gscreen, r, swconsole_back, ZP, nil, ZP, S);
    arch_flushmemscreen(r);

    swconsole_curpos.y -= o;
}
/*e: function swconsole_scroll */

/*s: function swconsole_screenputc */
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
        if (swconsole_curpos.y + h >= swconsole_window.max.y)
            swconsole_scroll();
        swconsole_curpos.y += h;
        swconsole_screenputc("\r");
        break;
    case '\r':
        xp = xbuf;
        swconsole_curpos.x = swconsole_window.min.x;
        break;
    case '\t':
        p = memsubfontwidth(swconsole_memdefont, " ");
        w = p.x;
        if (swconsole_curpos.x >= swconsole_window.max.x - Tabstop * w)
            swconsole_screenputc("\n");

        pos = (swconsole_curpos.x - swconsole_window.min.x) / w;
        pos = Tabstop - pos % Tabstop;
        *xp++ = swconsole_curpos.x;
        r = Rect(swconsole_curpos.x, swconsole_curpos.y, 
                 swconsole_curpos.x + pos * w, swconsole_curpos.y + h);
        memimagedraw(gscreen, r, swconsole_back, 
                     swconsole_back->r.min, nil, swconsole_back->r.min, S);
        arch_flushmemscreen(r);
        swconsole_curpos.x += pos * w;
        break;
    case '\b':
        if (xp <= xbuf)
            break;
        xp--;
        r = Rect(*xp, swconsole_curpos.y, 
                 swconsole_curpos.x, swconsole_curpos.y + h);
        memimagedraw(gscreen, r, swconsole_back, 
                     swconsole_back->r.min, nil, swconsole_back->r.min, S);
        arch_flushmemscreen(r);
        swconsole_curpos.x = *xp;
        break;
    case '\0':
        break;
    default:
        p = memsubfontwidth(swconsole_memdefont, buf);
        w = p.x;

        if (swconsole_curpos.x >= swconsole_window.max.x - w)
            swconsole_screenputc("\n");

        *xp++ = swconsole_curpos.x;
        r = Rect(swconsole_curpos.x, swconsole_curpos.y, 
                 swconsole_curpos.x + w, swconsole_curpos.y + h);
        memimagedraw(gscreen, r, swconsole_back, 
                     swconsole_back->r.min, nil, swconsole_back->r.min, S);
        memimagestring(gscreen, swconsole_curpos, swconsole_conscol, ZP, 
                       swconsole_memdefont, buf);
        arch_flushmemscreen(r);
        swconsole_curpos.x += w;
        break;
    }
}
/*e: function swconsole_screenputc */

/*s: function swconsole_screenputs */
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
/*e: function swconsole_screenputs */

/*e: kernel/devices/screen/swconsole.c */
