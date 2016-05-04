/*s: lib_graphics/libdraw/emenuhit.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>
#include <event.h>

/*s: enum _anon_ (lib_graphics/libdraw/emenuhit.c) */
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
/*e: enum _anon_ (lib_graphics/libdraw/emenuhit.c) */

/*s: global menutxt */
static	Image	*menutxt;
/*e: global menutxt */
/*s: global back */
static	Image	*back;
/*e: global back */
/*s: global high */
static	Image	*high;
/*e: global high */
/*s: global bord */
static	Image	*bord;
/*e: global bord */
/*s: global text */
static	Image	*text;
/*e: global text */
/*s: global htext */
static	Image	*htext;
/*e: global htext */

/*s: function menucolors */
static
void
menucolors(void)
{
    /* Main tone is greenish, with negative selection */
    back = allocimagemix(display, DPalegreen, DWhite);
    high = allocimage(display, Rect(0,0,1,1), CMAP8, 1, DDarkgreen);	/* dark green */
    bord = allocimage(display, Rect(0,0,1,1), CMAP8, 1, DMedgreen);	/* not as dark green */
    if(back == nil || high == nil || bord == nil)
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
/*e: function menucolors */

/*s: function menurect */
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
/*e: function menurect */

/*s: function menusel */
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
/*e: function menusel */

/*s: function paintitem */
static
void
paintitem(Menu *menu, Rectangle textr, int off, int i, int highlight, Image *save, Image *restore)
{
    char *item;
    Rectangle r;
    Point pt;

    if(i < 0)
        return;
    r = menurect(textr, i);
    if(restore){
        draw(view, r, restore, nil, restore->r.min);
        return;
    }
    if(save)
        draw(save, save->r, view, nil, r.min);
    item = menu->item? menu->item[i+off] : (*menu->gen)(i+off);
    pt.x = (textr.min.x+textr.max.x-stringwidth(font, item))/2;
    pt.y = textr.min.y+i*(font->height+Vspacing);
    draw(view, r, highlight? high : back, nil, pt);
    string(view, pt, highlight? htext : text, pt, font, item);
}
/*e: function paintitem */

/*s: function menuscan */
/*
 * menur is a rectangle holding all the highlightable text elements.
 * track mouse while inside the box, return what's selected when button
 * is raised, -1 as soon as it leaves box.
 * invariant: nothing is highlighted on entry or exit.
 */
static int
menuscan(Menu *menu, int but, Mouse *m, Rectangle textr, int off, int lasti, Image *save)
{
    int i;

    paintitem(menu, textr, off, lasti, 1, save, nil);
    flushimage(display, true);	/* in case display->locking is set */
    *m = emouse();
    while(m->buttons & (1<<(but-1))){
        flushimage(display, true);	/* in case display->locking is set */
        *m = emouse();
        i = menusel(textr, m->xy);
        if(i != -1 && i == lasti)
            continue;
        paintitem(menu, textr, off, lasti, 0, nil, save);
        if(i == -1)
            return i;
        lasti = i;
        paintitem(menu, textr, off, lasti, 1, save, nil);
    }
    return lasti;
}
/*e: function menuscan */

/*s: function menupaint */
static void
menupaint(Menu *menu, Rectangle textr, int off, int nitemdrawn)
{
    int i;

    draw(view, insetrect(textr, Border-Margin), back, nil, ZP);
    for(i = 0; i<nitemdrawn; i++)
        paintitem(menu, textr, off, i, 0, nil, nil);
}
/*e: function menupaint */

/*s: function menuscrollpaint */
static void
menuscrollpaint(Rectangle scrollr, int off, int nitem, int nitemdrawn)
{
    Rectangle r;

    draw(view, scrollr, back, nil, ZP);
    r.min.x = scrollr.min.x;
    r.max.x = scrollr.max.x;
    r.min.y = scrollr.min.y + (Dy(scrollr)*off)/nitem;
    r.max.y = scrollr.min.y + (Dy(scrollr)*(off+nitemdrawn))/nitem;
    if(r.max.y < r.min.y+2)
        r.max.y = r.min.y+2;
    border(view, r, 1, bord, ZP);
    if(menutxt == 0)
        menutxt = allocimage(display, Rect(0, 0, 1, 1), CMAP8, 1, DDarkgreen);
    if(menutxt)
        draw(view, insetrect(r, 1), menutxt, nil, ZP);
}
/*e: function menuscrollpaint */

/*s: function emenuhit */
int
emenuhit(int but, Mouse *m, Menu *menu)
{
    int i, nitem, nitemdrawn, maxwid, lasti, off, noff, wid, screenitem;
    int scrolling;
    Rectangle r, menur, sc, textr, scrollr;
    Image *b, *save;
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
    r = rectaddpt(r, m->xy);
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

    b = allocimage(display, menur, view->chan, 0, 0);
    if(b == 0)
        b = view;
    draw(b, menur, view, nil, menur.min);
    draw(view, menur, back, nil, ZP);
    border(view, menur, Blackborder, bord, ZP);
    save = allocimage(display, menurect(textr, 0), view->chan, 0, -1);
    r = menurect(textr, lasti);
    emoveto(divpt(addpt(r.min, r.max), 2));
    menupaint(menu, textr, off, nitemdrawn);
    if(scrolling)
        menuscrollpaint(scrollr, off, nitem, nitemdrawn);
    while(m->buttons & (1<<(but-1))){
        lasti = menuscan(menu, but, m, textr, off, lasti, save);
        if(lasti >= 0)
            break;
        while(!ptinrect(m->xy, textr) && (m->buttons & (1<<(but-1)))){
            if(scrolling && ptinrect(m->xy, scrollr)){
                noff = ((m->xy.y-scrollr.min.y)*nitem)/Dy(scrollr);
                noff -= nitemdrawn/2;
                if(noff < 0)
                    noff = 0;
                if(noff > nitem-nitemdrawn)
                    noff = nitem-nitemdrawn;
                if(noff != off){
                    off = noff;
                    menupaint(menu, textr, off, nitemdrawn);
                    menuscrollpaint(scrollr, off, nitem, nitemdrawn);
                }
            }
            flushimage(display, true);	/* in case display->locking is set */
            *m = emouse();
        }
    }
    draw(view, menur, b, nil, menur.min);
    if(b != view)
        freeimage(b);
    freeimage(save);
    replclipr(view, 0, sc);
    if(lasti >= 0){
        menu->lasthit = lasti+off;
        return menu->lasthit;
    }
    return -1;
}
/*e: function emenuhit */
/*e: lib_graphics/libdraw/emenuhit.c */
