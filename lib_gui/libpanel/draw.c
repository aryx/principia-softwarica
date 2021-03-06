/*s: lib_gui/libpanel/draw.c */
/*s: [[libpanel]] includes */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>

#include <panel.h>
#include "pldefs.h"
/*e: [[libpanel]] includes */

/*s: constant [[PWID]] */
#define	PWID	1	/* width of label border */
/*e: constant [[PWID]] */
/*s: constant [[BWID]] */
#define	BWID	1	/* width of button relief */
/*e: constant [[BWID]] */
/*s: constant [[FWID]] */
#define	FWID	2	/* width of frame relief */
/*e: constant [[FWID]] */
/*s: constant [[SPACE]] */
#define	SPACE	1	/* space inside relief of button or frame */
/*e: constant [[SPACE]] */
/*s: constant [[CKSIZE]] */
#define	CKSIZE	3	/* size of check mark */
/*e: constant [[CKSIZE]] */
/*s: constant [[CKSPACE]] */
#define	CKSPACE	2	/* space around check mark */
/*e: constant [[CKSPACE]] */
/*s: constant [[CKWID]] */
#define	CKWID	1	/* width of frame around check mark */
/*e: constant [[CKWID]] */
/*s: constant [[CKINSET]] */
#define	CKINSET	1	/* space around check mark frame */
/*e: constant [[CKINSET]] */
/*s: constant [[CKBORDER]] */
#define	CKBORDER 2	/* space around X inside frame */
/*e: constant [[CKBORDER]] */

/*s: global [[plldepth]] */
static int plldepth;
/*e: global [[plldepth]] */

/*s: globals [[pl_xxx]] */
static Image *pl_white, *pl_light, *pl_dark, *pl_black, *pl_hilit;
/*e: globals [[pl_xxx]] */

/*s: function [[pl_drawinit]] */
error0 pl_drawinit(int ldepth){

    plldepth=ldepth;

    pl_black=allocimage(display, Rect(0,0,1,1), view->chan, 1, 0x000000FF);
    pl_white=allocimage(display, Rect(0,0,1,1), view->chan, 1, 0xFFFFFFFF);

    pl_dark =allocimage(display, Rect(0,0,1,1), view->chan, 1, DPurpleblue);
    pl_light=allocimagemix(display, DPalebluegreen, DWhite);

    pl_hilit=allocimage(display, Rect(0,0,1,1), CHAN1(CAlpha,8), 1, 0x80);

    if(pl_white==nil || pl_light==nil || pl_black==nil || pl_dark==nil) 
        return ERROR_0;
    return OK_1;
}
/*e: function [[pl_drawinit]] */
/*s: function [[pl_relief]] */
void pl_relief(Image *b, Image *ul, Image *lr, Rectangle r, int wid){
    int x, y;

    draw(b, Rect(r.min.x, r.max.y-wid, r.max.x, r.max.y), lr, 0, ZP);/* bottom*/
    draw(b, Rect(r.max.x-wid, r.min.y, r.max.x, r.max.y), lr, 0, ZP);/* right */
    draw(b, Rect(r.min.x, r.min.y, r.min.x+wid, r.max.y), ul, 0, ZP);/* left */
    draw(b, Rect(r.min.x, r.min.y, r.max.x, r.min.y+wid), ul, 0, ZP);/* top */

    for(x=0;x!=wid;x++) 
      for(y=wid-1-x;y!=wid;y++){
        draw(b, rectaddpt(Rect(0,0,1,1), Pt(x+r.max.x-wid, y+r.min.y)), lr, 0, ZP);
        draw(b, rectaddpt(Rect(0,0,1,1), Pt(x+r.min.x, y+r.max.y-wid)), lr, 0, ZP);
    }
}
/*e: function [[pl_relief]] */
/*s: function [[pl_boxoutline]] */
Rectangle pl_boxoutline(Image *b, Rectangle r, int style, bool fill){
    /*s: [[pl_boxoutline()]] if [[plldepth]] is zero */
    if(plldepth==0) switch(style){
    case UP:
        pl_relief(b, pl_black, pl_black, r, BWID);
        r=insetrect(r, BWID);
        if(fill) draw(b, r, pl_white, 0, ZP);
        else border(b, r, SPACE, pl_white, ZP);
        break;
    case DOWN:
    case DOWN1:
    case DOWN2:
    case DOWN3:
        pl_relief(b, pl_black, pl_black, r, BWID);
        r=insetrect(r, BWID);
        if(fill) draw(b, r, pl_black, 0, ZP);
        border(b, r, SPACE, pl_black, ZP);
        break;
    case PASSIVE:
        if(fill) draw(b, r, pl_white, 0, ZP);
        r=insetrect(r, PWID);
        if(!fill) border(b, r, SPACE, pl_white, ZP);
        break;
    case FRAME:
        pl_relief(b, pl_white, pl_black, r, FWID);
        r=insetrect(r, FWID);
        pl_relief(b, pl_black, pl_white, r, FWID);
        r=insetrect(r, FWID);
        if(fill) draw(b, r, pl_white, 0, ZP);
        else border(b, r, SPACE, pl_white, ZP);
        break;
    }
    /*e: [[pl_boxoutline()]] if [[plldepth]] is zero */
    else 
    switch(style){
    /*s: [[pl_boxoutline()]] switch style cases */
    case PASSIVE:
        if(fill) 
            draw(b, r, pl_light, 0, ZP);
        r=insetrect(r, PWID);
        if(!fill) 
            border(b, r, SPACE, pl_white, ZP);
        break;
    /*x: [[pl_boxoutline()]] switch style cases */
    case UP:
        pl_relief(b, pl_white, pl_black, r, BWID);
        r=insetrect(r, BWID);
        if(fill) 
            draw(b, r, pl_light, nil, ZP);
        else 
            border(b, r, SPACE, pl_white, ZP);
        break;
    /*x: [[pl_boxoutline()]] switch style cases */
    case DOWN:
    case DOWN1:
    case DOWN2:
    case DOWN3:
        pl_relief(b, pl_black, pl_white, r, BWID);
        r=insetrect(r, BWID);
        if(fill) 
            draw(b, r, pl_dark, 0, ZP);
        else 
            border(b, r, SPACE, pl_black, ZP);
        break;
    /*x: [[pl_boxoutline()]] switch style cases */
    case FRAME:
        pl_relief(b, pl_white, pl_black, r, FWID);
        r=insetrect(r, FWID);
        pl_relief(b, pl_black, pl_white, r, FWID);
        r=insetrect(r, FWID);
        if(fill) 
            draw(b, r, pl_light, nil, ZP);
        else 
            border(b, r, SPACE, pl_white, ZP);
        break;
    /*e: [[pl_boxoutline()]] switch style cases */
    }
    return insetrect(r, SPACE);
}
/*e: function [[pl_boxoutline]] */
/*s: function [[pl_outline]] */
Rectangle pl_outline(Image *b, Rectangle r, int style){
    return pl_boxoutline(b, r, style, false);
}
/*e: function [[pl_outline]] */
/*s: function [[pl_box]] */
Rectangle pl_box(Image *b, Rectangle r, int style){
    return pl_boxoutline(b, r, style, true);
}
/*e: function [[pl_box]] */
/*s: function [[pl_boxsize]] */
Vector pl_boxsize(Vector interior, int state){
    switch(state){
    /*s: [[pl_boxsize()]] switch state cases */
    case PASSIVE:
        return addpt(interior, Pt(2*(PWID+SPACE), 2*(PWID+SPACE)));
    /*x: [[pl_boxsize()]] switch state cases */
    case UP:
    case DOWN:
    case DOWN1:
    case DOWN2:
    case DOWN3:
        return addpt(interior, Pt(2*(BWID+SPACE), 2*(BWID+SPACE)));
    /*x: [[pl_boxsize()]] switch state cases */
    case FRAME:
        return addpt(interior, Pt(4*FWID+2*SPACE, 4*FWID+2*SPACE));
    /*e: [[pl_boxsize()]] switch state cases */
    }
    // else
    return Pt(0, 0);
}
/*e: function [[pl_boxsize]] */
/*s: function [[pl_interior]] */
void pl_interior(int state, Point *ul, Vector *size){
    switch(state){
    /*s: [[pl_interior()]] switch state cases */
    case UP:
    case DOWN:
    case DOWN1:
    case DOWN2:
    case DOWN3:
        *ul=addpt(*ul, Pt(BWID+SPACE, BWID+SPACE));
        *size=subpt(*size, Pt(2*(BWID+SPACE), 2*(BWID+SPACE)));
        break;
    /*x: [[pl_interior()]] switch state cases */
    case PASSIVE:
        *ul=addpt(*ul, Pt(PWID+SPACE, PWID+SPACE));
        *size=subpt(*size, Pt(2*(PWID+SPACE), 2*(PWID+SPACE)));
        break;
    /*x: [[pl_interior()]] switch state cases */
    case FRAME:
        *ul=addpt(*ul, Pt(2*FWID+SPACE, 2*FWID+SPACE));
        *size=subpt(*size, Pt(4*FWID+2*SPACE, 4*FWID+2*SPACE));
    /*e: [[pl_interior()]] switch state cases */
    }
}
/*e: function [[pl_interior]] */

/*s: function [[pl_drawicon]] */
void pl_drawicon(Image *b, Rectangle r, int stick, int flags, Icon *s){
    Point ul;
    Vector offs;
    /*s: [[pl_drawicon()]] other locals */
    Rectangle save;
    /*e: [[pl_drawicon()]] other locals */

    ul=r.min;
    offs=subpt(subpt(r.max, r.min), pl_iconsize(flags, s));

    switch(stick){
    /*s: [[pl_drawicon()]] switch placement cases, adjust [[ul]] */
    case PLACENW:	/** Nothing **/                 break;
    case PLACEN:	ul.x+=offs.x/2;                 break;
    case PLACENE:	ul.x+=offs.x;                   break;
    case PLACEW:	                ul.y+=offs.y/2; break;
    case PLACECEN:	ul.x+=offs.x/2; ul.y+=offs.y/2; break;
    case PLACEE:	ul.x+=offs.x;                   break;
    case PLACESW:	                ul.y+=offs.y;   break;
    case PLACES:	ul.x+=offs.x/2; ul.y+=offs.y;   break;
    case PLACESE:	ul.x+=offs.x;   ul.y+=offs.y;   break;
    /*e: [[pl_drawicon()]] switch placement cases, adjust [[ul]] */
    }

    /*s: [[pl_drawicon()]] save and adjust clip rectangle */
    save=b->clipr;
    if(!rectclip(&r, save))
        return;
    replclipr(b, b->repl, r);
    /*e: [[pl_drawicon()]] save and adjust clip rectangle */
    if(flags&BITMAP) 
        draw(b, Rpt(ul, addpt(ul, pl_iconsize(flags, s))), s, nil, ZP);
    else 
        string(b, ul, pl_black, ZP, font, s);
    /*s: [[pl_drawicon()]] restore saved clip rectangle */
    replclipr(b, b->repl, save);
    /*e: [[pl_drawicon()]] restore saved clip rectangle */
}
/*e: function [[pl_drawicon]] */
/*s: function [[pl_radio]] */
/*
 * Place a check mark at the left end of r.  Return the unused space.
 * Caller must guarantee that r.max.x-r.min.x>=r.max.y-r.min.y!
 */
Rectangle pl_radio(Image *b, Rectangle r, bool val){
    Rectangle remainder = r;

    r.max.x=r.min.x+r.max.y-r.min.y;
    remainder.min.x=r.max.x;
    r=insetrect(r, CKINSET);
    /*s: [[pl_radio()]] if null [[plldepth]] part1 */
    if(plldepth==0)
        pl_relief(b, pl_black, pl_black, r, CKWID);
    /*e: [[pl_radio()]] if null [[plldepth]] part1 */
    else
        pl_relief(b, pl_black, pl_white, r, CKWID);
    r=insetrect(r, CKWID);
    /*s: [[pl_radio()]] if null [[plldepth]] part2 */
    if(plldepth==0)
        draw(b, r, pl_white, 0, ZP);
    /*e: [[pl_radio()]] if null [[plldepth]] part2 */
    else
        draw(b, r, pl_light, 0, ZP);
    if(val) 
        draw(b, insetrect(r, CKSPACE), pl_black, nil, ZP);
    return remainder;
}
/*e: function [[pl_radio]] */
/*s: function [[pl_check]] */
Rectangle pl_check(Image *b, Rectangle r, bool val){
    Rectangle remainder = r;

    r.max.x=r.min.x+r.max.y-r.min.y;
    remainder.min.x=r.max.x;
    r=insetrect(r, CKINSET);
    /*s: [[pl_check()]] if null [[plldepth]] part1 */
    if(plldepth==0)
        pl_relief(b, pl_black, pl_black, r, CKWID);
    /*e: [[pl_check()]] if null [[plldepth]] part1 */
    else
        pl_relief(b, pl_black, pl_white, r, CKWID);
    r=insetrect(r, CKWID);
    /*s: [[pl_check()]] if null [[plldepth]] part2 */
    if(plldepth==0)
        draw(b, r, pl_white, 0, ZP);
    /*e: [[pl_check()]] if null [[plldepth]] part2 */
    else
        draw(b, r, pl_light, nil, ZP);
    r=insetrect(r, CKBORDER);
    if(val){
        /*s: [[pl_check()]] if checked button */
        line(b, Pt(r.min.x,   r.min.y+1), Pt(r.max.x-1, r.max.y  ), Endsquare, Endsquare, 0, pl_black, ZP);
        line(b, Pt(r.min.x,   r.min.y  ), Pt(r.max.x,   r.max.y  ), Endsquare, Endsquare, 0, pl_black, ZP);
        line(b, Pt(r.min.x+1, r.min.y  ), Pt(r.max.x,   r.max.y-1), Endsquare, Endsquare, 0, pl_black, ZP);
        line(b, Pt(r.min.x  , r.max.y-2), Pt(r.max.x-1, r.min.y-1), Endsquare, Endsquare, 0, pl_black, ZP);
        line(b, Pt(r.min.x,   r.max.y-1), Pt(r.max.x,   r.min.y-1), Endsquare, Endsquare, 0, pl_black, ZP);
        line(b, Pt(r.min.x+1, r.max.y-1), Pt(r.max.x,   r.min.y  ), Endsquare, Endsquare, 0, pl_black, ZP);
        /*e: [[pl_check()]] if checked button */
    }
    return remainder;
}
/*e: function [[pl_check]] */
/*s: function [[pl_ckwid]] */
int pl_ckwid(void){
    return 2*(CKINSET+CKSPACE+CKWID)+CKSIZE;
}
/*e: function [[pl_ckwid]] */
/*s: function [[pl_sliderupd]] */
void pl_sliderupd(Image *b, Rectangle r1, int dir, int lo, int hi){
    Rectangle r2, r3;

    r2=r1;
    r3=r1;

    /*s: [[pl_sliderupd()]] sanitize [[lo]] and [[hi]] */
    if(lo<0) lo=0;
    if(hi<=lo) hi=lo+1;
    /*e: [[pl_sliderupd()]] sanitize [[lo]] and [[hi]] */

    switch(dir){
    case HORIZ:
        r1.max.x=r1.min.x+lo;
        r2.min.x=r1.max.x;
        r2.max.x=r1.min.x+hi;
        if(r2.max.x>r3.max.x) r2.max.x=r3.max.x;
        r3.min.x=r2.max.x;
        break;
    case VERT:
        r1.max.y=r1.min.y+lo;
        r2.min.y=r1.max.y;
        r2.max.y=r1.min.y+hi;
        if(r2.max.y>r3.max.y) r2.max.y=r3.max.y;
        r3.min.y=r2.max.y;
        break;
    }
    draw(b, r1, pl_light, nil, ZP);
    draw(b, r2, pl_dark, nil, ZP);
    draw(b, r3, pl_light, nil, ZP);
}
/*e: function [[pl_sliderupd]] */

void pl_draw1(Panel *p, Image *b);

/*s: function [[pl_drawall]] */
void pl_drawall(Panel *p, Image *b){
    /*s: [[pl_drawall()]] if invisible widget */
    if(p->flags&INVIS) return;
    /*e: [[pl_drawall()]] if invisible widget */
    p->b=b;
    // widget-specific method
    p->draw(p);
    for(p=p->child;p;p=p->next) 
        // recurse
        pl_draw1(p, b);
}
/*e: function [[pl_drawall]] */
/*s: function [[pl_draw1]] */
void pl_draw1(Panel *p, Image *b){
    if(b!=nil)
        pl_drawall(p, b);
}
/*e: function [[pl_draw1]] */
/*s: function [[pldraw]] */
void pldraw(Panel *p, Image *b){
    pl_draw1(p, b);
    flushimage(display, true);
}
/*e: function [[pldraw]] */
/*s: function [[pl_invis]] */
void pl_invis(Panel *p, bool v){
    for(;p;p=p->next){
        if(v) 
            p->flags|=INVIS; 
        else 
            p->flags&=~INVIS;
        pl_invis(p->child, v);
    }
}
/*e: function [[pl_invis]] */
/*s: function [[pl_iconsize]] */
Vector pl_iconsize(int flags, Icon *p){
    if(flags&BITMAP) 
        return subpt(((Image *)p)->r.max, ((Image *)p)->r.min);
    else 
        return stringsize(font, (char *)p);
}
/*e: function [[pl_iconsize]] */
/*s: function [[pl_highlight]] */
void pl_highlight(Image *b, Rectangle r){
    draw(b, r, pl_dark, pl_hilit, ZP);
}
/*e: function [[pl_highlight]] */
/*s: function [[pl_clr]] */
void pl_clr(Image *b, Rectangle r){
    draw(b, r, display->white, 0, ZP);
}
/*e: function [[pl_clr]] */
/*s: function [[pl_fill]] */
void pl_fill(Image *b, Rectangle r){
    draw(b, r, plldepth==0? pl_white : pl_light, 0, ZP);
}
/*e: function [[pl_fill]] */
/*s: function [[pl_cpy]] */
void pl_cpy(Image *b, Point dst, Rectangle src){
    draw(b, Rpt(dst, addpt(dst, subpt(src.max, src.min))), b, nil, src.min);
}
/*e: function [[pl_cpy]] */
/*e: lib_gui/libpanel/draw.c */
