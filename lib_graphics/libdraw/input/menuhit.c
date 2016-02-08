/*s: lib_graphics/libdraw/menuhit.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>
#include <mouse.h>

/*s: enum _anon_ (lib_graphics/libdraw/menuhit.c) */
enum
{
    Margin = 4,		/* outside to text */
    Border = 2,		/* outside to selection boxes */
    Blackborder = 2,	/* width of outlining border */
    Vspacing = 2,		/* extra spacing between lines of text */
    Maxunscroll = 25,	/* maximum #entries before scrolling turns on */
    Nscroll = 20,		/* number entries in scrolling part */
    Scrollwid = 14,		/* width of scroll bar */
    Gap = 4,			/* between text and scroll bar */
};
/*e: enum _anon_ (lib_graphics/libdraw/menuhit.c) */

/*s: global menutxt (lib_graphics/libdraw/menuhit.c) */
static	Image	*menutxt;
/*e: global menutxt (lib_graphics/libdraw/menuhit.c) */
/*s: global back (lib_graphics/libdraw/menuhit.c) */
static	Image	*back;
/*e: global back (lib_graphics/libdraw/menuhit.c) */
/*s: global high (lib_graphics/libdraw/menuhit.c) */
static	Image	*high;
/*e: global high (lib_graphics/libdraw/menuhit.c) */
/*s: global bord (lib_graphics/libdraw/menuhit.c) */
static	Image	*bord;
/*e: global bord (lib_graphics/libdraw/menuhit.c) */
/*s: global text (lib_graphics/libdraw/menuhit.c) */
static	Image	*text;
/*e: global text (lib_graphics/libdraw/menuhit.c) */
/*s: global htext (lib_graphics/libdraw/menuhit.c) */
static	Image	*htext;
/*e: global htext (lib_graphics/libdraw/menuhit.c) */

/*s: function menucolors (lib_graphics/libdraw/menuhit.c) */
static
void
menucolors(void)
{
    /* Main tone is greenish, with negative selection */
    back = allocimagemix(display, DPalegreen, DWhite);
    high = allocimage(display, Rect(0,0,1,1), view->chan, 1, DDarkgreen);	/* dark green */
    bord = allocimage(display, Rect(0,0,1,1), view->chan, 1, DMedgreen);	/* not as dark green */
    if(back==nil || high==nil || bord==nil)
        goto Error;
    text = display->black;
    htext = back;
    return;

    Error:
    freeimage(back);
    freeimage(high);
    freeimage(bord);
    back = display->white;
    high = display->black;
    bord = display->black;
    text = display->black;
    htext = display->white;
}
/*e: function menucolors (lib_graphics/libdraw/menuhit.c) */

/*s: function menurect (lib_graphics/libdraw/menuhit.c) */
/*
 * r is a rectangle holding the text elements.
 * return the rectangle, including its black edge, holding element i.
 */
static Rectangle
menurect(Rectangle r, int i)
{
    if(i < 0)
        return Rect(0, 0, 0, 0);
    r.min.y += (font->height+Vspacing)*i;
    r.max.y = r.min.y+font->height+Vspacing;
    return insetrect(r, Border-Margin);
}
/*e: function menurect (lib_graphics/libdraw/menuhit.c) */

/*s: function menusel (lib_graphics/libdraw/menuhit.c) */
/*
 * r is a rectangle holding the text elements.
 * return the element number containing p.
 */
static int
menusel(Rectangle r, Point p)
{
    r = insetrect(r, Margin);
    if(!ptinrect(p, r))
        return -1;
    return (p.y-r.min.y)/(font->height+Vspacing);
}
/*e: function menusel (lib_graphics/libdraw/menuhit.c) */

/*s: function paintitem (lib_graphics/libdraw/menuhit.c) */
static
void
paintitem(Image *m, Menu *menu, Rectangle textr, int off, int i, int highlight, Image *save, Image *restore)
{
    char *item;
    Rectangle r;
    Point pt;

    if(i < 0)
        return;
    r = menurect(textr, i);
    if(restore){
        draw(m, r, restore, nil, restore->r.min);
        return;
    }
    if(save)
        draw(save, save->r, m, nil, r.min);
    item = menu->item? menu->item[i+off] : (*menu->gen)(i+off);
    pt.x = (textr.min.x+textr.max.x-stringwidth(font, item))/2;
    pt.y = textr.min.y+i*(font->height+Vspacing);
    draw(m, r, highlight? high : back, nil, pt);
    string(m, pt, highlight? htext : text, pt, font, item);
}
/*e: function paintitem (lib_graphics/libdraw/menuhit.c) */

/*s: function menuscan (lib_graphics/libdraw/menuhit.c) */
/*
 * menur is a rectangle holding all the highlightable text elements.
 * track mouse while inside the box, return what's selected when button
 * is raised, -1 as soon as it leaves box.
 * invariant: nothing is highlighted on entry or exit.
 */
static int
menuscan(Image *m, Menu *menu, int but, Mousectl *mc, Rectangle textr, int off, int lasti, Image *save)
{
    int i;

    paintitem(m, menu, textr, off, lasti, 1, save, nil);
    for(readmouse(mc); mc->buttons & (1<<(but-1)); readmouse(mc)){
        i = menusel(textr, mc->xy);
        if(i != -1 && i == lasti)
            continue;
        paintitem(m, menu, textr, off, lasti, 0, nil, save);
        if(i == -1)
            return i;
        lasti = i;
        paintitem(m, menu, textr, off, lasti, 1, save, nil);
    }
    return lasti;
}
/*e: function menuscan (lib_graphics/libdraw/menuhit.c) */

/*s: function menupaint (lib_graphics/libdraw/menuhit.c) */
static void
menupaint(Image *m, Menu *menu, Rectangle textr, int off, int nitemdrawn)
{
    int i;

    draw(m, insetrect(textr, Border-Margin), back, nil, ZP);
    for(i = 0; i<nitemdrawn; i++)
        paintitem(m, menu, textr, off, i, 0, nil, nil);
}
/*e: function menupaint (lib_graphics/libdraw/menuhit.c) */

/*s: function menuscrollpaint (lib_graphics/libdraw/menuhit.c) */
static void
menuscrollpaint(Image *m, Rectangle scrollr, int off, int nitem, int nitemdrawn)
{
    Rectangle r;

    draw(m, scrollr, back, nil, ZP);
    r.min.x = scrollr.min.x;
    r.max.x = scrollr.max.x;
    r.min.y = scrollr.min.y + (Dy(scrollr)*off)/nitem;
    r.max.y = scrollr.min.y + (Dy(scrollr)*(off+nitemdrawn))/nitem;
    if(r.max.y < r.min.y+2)
        r.max.y = r.min.y+2;
    border(m, r, 1, bord, ZP);
    if(menutxt == 0)
        menutxt = allocimage(display, Rect(0, 0, 1, 1), view->chan, 1, DDarkgreen);	/* border color; BUG? */
    if(menutxt)
        draw(m, insetrect(r, 1), menutxt, nil, ZP);
}
/*e: function menuscrollpaint (lib_graphics/libdraw/menuhit.c) */

/*s: function menuhit */
int
menuhit(int but, Mousectl *mc, Menu *menu, Screen *scr)
{
    int i, nitem, nitemdrawn, maxwid, lasti, off, noff, wid, screenitem;
    int scrolling;
    Rectangle r, menur, sc, textr, scrollr;
    Image *b, *save, *backup;
    Point pt;
    char *item;

    if(back == nil)
        menucolors();
    sc = view->clipr;
    replclipr(view, 0, view->r);
    maxwid = 0;
    for(nitem = 0;
        item = menu->item? menu->item[nitem] : (*menu->gen)(nitem);
        nitem++){
        i = stringwidth(font, item);
        if(i > maxwid)
            maxwid = i;
    }
    if(menu->lasthit<0 || menu->lasthit>=nitem)
        menu->lasthit = 0;
    screenitem = (Dy(view->r)-10)/(font->height+Vspacing);
    if(nitem>Maxunscroll || nitem>screenitem){
        scrolling = 1;
        nitemdrawn = Nscroll;
        if(nitemdrawn > screenitem)
            nitemdrawn = screenitem;
        wid = maxwid + Gap + Scrollwid;
        off = menu->lasthit - nitemdrawn/2;
        if(off < 0)
            off = 0;
        if(off > nitem-nitemdrawn)
            off = nitem-nitemdrawn;
        lasti = menu->lasthit-off;
    }else{
        scrolling = 0;
        nitemdrawn = nitem;
        wid = maxwid;
        off = 0;
        lasti = menu->lasthit;
    }
    r = insetrect(Rect(0, 0, wid, nitemdrawn*(font->height+Vspacing)), -Margin);
    r = rectsubpt(r, Pt(wid/2, lasti*(font->height+Vspacing)+font->height/2));
    r = rectaddpt(r, mc->xy);
    pt = ZP;
    if(r.max.x>view->r.max.x)
        pt.x = view->r.max.x-r.max.x;
    if(r.max.y>view->r.max.y)
        pt.y = view->r.max.y-r.max.y;
    if(r.min.x<view->r.min.x)
        pt.x = view->r.min.x-r.min.x;
    if(r.min.y<view->r.min.y)
        pt.y = view->r.min.y-r.min.y;
    menur = rectaddpt(r, pt);
    textr.max.x = menur.max.x-Margin;
    textr.min.x = textr.max.x-maxwid;
    textr.min.y = menur.min.y+Margin;
    textr.max.y = textr.min.y + nitemdrawn*(font->height+Vspacing);
    if(scrolling){
        scrollr = insetrect(menur, Border);
        scrollr.max.x = scrollr.min.x+Scrollwid;
    }else
        scrollr = Rect(0, 0, 0, 0);

    if(scr){
        b = allocwindow(scr, menur, Refbackup, DWhite);
        if(b == nil)
            b = view;
        backup = nil;
    }else{
        b = view;
        backup = allocimage(display, menur, view->chan, 0, -1);
        if(backup)
            draw(backup, menur, view, nil, menur.min);
    }
    draw(b, menur, back, nil, ZP);
    border(b, menur, Blackborder, bord, ZP);
    save = allocimage(display, menurect(textr, 0), view->chan, 0, -1);
    r = menurect(textr, lasti);
    moveto(mc, divpt(addpt(r.min, r.max), 2));
    menupaint(b, menu, textr, off, nitemdrawn);
    if(scrolling)
        menuscrollpaint(b, scrollr, off, nitem, nitemdrawn);
    while(mc->buttons & (1<<(but-1))){
        lasti = menuscan(b, menu, but, mc, textr, off, lasti, save);
        if(lasti >= 0)
            break;
        while(!ptinrect(mc->xy, textr) && (mc->buttons & (1<<(but-1)))){
            if(scrolling && ptinrect(mc->xy, scrollr)){
                noff = ((mc->xy.y-scrollr.min.y)*nitem)/Dy(scrollr);
                noff -= nitemdrawn/2;
                if(noff < 0)
                    noff = 0;
                if(noff > nitem-nitemdrawn)
                    noff = nitem-nitemdrawn;
                if(noff != off){
                    off = noff;
                    menupaint(b, menu, textr, off, nitemdrawn);
                    menuscrollpaint(b, scrollr, off, nitem, nitemdrawn);
                }
            }
            readmouse(mc);
        }
    }
    if(b != view)
        freeimage(b);
    if(backup){
        draw(view, menur, backup, nil, menur.min);
        freeimage(backup);
    }
    freeimage(save);
    replclipr(view, 0, sc);
    flushimage(display, true);
    if(lasti >= 0){
        menu->lasthit = lasti+off;
        return menu->lasthit;
    }
    return -1;
}
/*e: function menuhit */
/*e: lib_graphics/libdraw/menuhit.c */