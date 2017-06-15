/*s: kernel/devices/screen/drawmesg.c */
#include    "u.h"
#include    "../port/lib.h"
#include    "../port/error.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"

#include    <draw.h>
#include    <marshal.h>
#include    <memdraw.h>
#include    <memlayer.h>

#include    <cursor.h>
#include    "portscreen.h"

#include    "devdraw.h"

/*s: function drawclientop */
static int
drawclientop(Client *cl)
{
    int op;

    op = cl->op;
    cl->op = SoverD;
    return op;
}
/*e: function drawclientop */

/*s: function drawimage */
Memimage*
drawimage(Client *client, byte *a)
{
    DImage *d;

    d = drawlookup(client, BGLONG(a), true);
    /*s: [[drawimage()]] sanity check d */
    if(d == nil)
        error(Enodrawimage);
    /*e: [[drawimage()]] sanity check d */
    return d->image;
}
/*e: function drawimage */

/*s: function drawrectangle */
void
drawrectangle(Rectangle *r, uchar *a)
{
    r->min.x = BGLONG(a+0*4);
    r->min.y = BGLONG(a+1*4);
    r->max.x = BGLONG(a+2*4);
    r->max.y = BGLONG(a+3*4);
}
/*e: function drawrectangle */

/*s: function drawpoint */
void
drawpoint(Point *p, uchar *a)
{
    p->x = BGLONG(a+0*4);
    p->y = BGLONG(a+1*4);
}
/*e: function drawpoint */

/*s: function drawcoord */
uchar*
drawcoord(uchar *p, uchar *maxp, int oldx, int *newx)
{
    int b, x;

    if(p >= maxp)
        error(Eshortdraw);
    b = *p++;
    x = b & 0x7F;
    if(b & 0x80){
        if(p+1 >= maxp)
            error(Eshortdraw);
        x |= *p++ << 7;
        x |= *p++ << 15;
        if(x & (1<<22))
            x |= ~0<<23;
    }else{
        if(b & 0x40)
            x |= ~0<<7;
        x += oldx;
    }
    *newx = x;
    return p;
}
/*e: function drawcoord */



/*s: function drawchar */
Point
drawchar(Memimage *dst, Memimage *rdst, Point p, Memimage *src, Point *sp, DImage *font, int index, int op)
{
    FChar *fc;
    Rectangle r;
    Point sp1;
    /*s: [[drawchar()]] other locals */
    static Memimage *tmp;
    /*e: [[drawchar()]] other locals */

    fc = &font->fchar[index];

    r.min.x = p.x + fc->left;
    r.min.y = p.y - (font->ascent - fc->miny);
    r.max.x = r.min.x + (fc->maxx - fc->minx);
    r.max.y = r.min.y + (fc->maxy - fc->miny);

    sp1.x = sp->x + fc->left;
    sp1.y = sp->y + fc->miny;

    /*s: [[drawchar()]] optimization when possible */
    /*
     * If we're drawing greyscale fonts onto a VGA screen,
     * it's very costly to read the screen memory to do the
     * alpha blending inside memdraw.  If this is really a stringbg,
     * then rdst is the bg image (in main memory) which we can
     * refer to for the underlying dst pixels instead of reading dst
     * directly.
     */
    if(arch_ishwimage(dst) && !arch_ishwimage(rdst) && font->image->depth > 1){
        if(tmp == nil || tmp->chan != dst->chan || Dx(tmp->r) < Dx(r) || Dy(tmp->r) < Dy(r)){
            if(tmp)
                freememimage(tmp);
            tmp = allocmemimage(Rect(0,0,Dx(r),Dy(r)), dst->chan);
            if(tmp == nil)
                goto fallback;
        }
        memdraw(tmp, Rect(0,0,Dx(r),Dy(r)), rdst, r.min, memopaque, ZP, S);
        memdraw(tmp, Rect(0,0,Dx(r),Dy(r)), src, sp1, font->image, 
          Pt(fc->minx, fc->miny), op);
        memdraw(dst, r, tmp, ZP, memopaque, ZP, S);
    }
    /*e: [[drawchar()]] optimization when possible */
    else{
    fallback:
        memdraw(dst, r, src, sp1, font->image, Pt(fc->minx, fc->miny), op);
    }

    p.x   += fc->width;
    sp->x += fc->width;
    return p;
}
/*e: function drawchar */


/*s: function printmesg */
static void
printmesg(char *fmt, uchar *a, bool plsprnt)
{
    char buf[256];
    char *p, *q;
    int s, left;

    if(!plsprnt && !drawdebug){ //old: 1 || !plsprnt
        SET(s,q,p);
        USED(fmt, a, buf, p, q, s);
        return;
    }
    // else, plsprnt || drawdebug

    q = buf;
    *q++ = *a++;
    for(p=fmt; *p; p++){
        left = sizeof buf - 2 - (q - buf);  /* 2 for \n\0 */
        switch(*p){
        case 'l':
            q += snprint(q, left, " %ld", (long)BGLONG(a));
            a += 4;
            break;
        case 'L':
            q += snprint(q, left, " %.8lux", (ulong)BGLONG(a));
            a += 4;
            break;
        case 'R':
            q += snprint(q, left, " [%d %d %d %d]", BGLONG(a),
                BGLONG(a+4), BGLONG(a+8), BGLONG(a+12));
            a += 16;
            break;
        case 'P':
            q += snprint(q, left, " [%d %d]", BGLONG(a), BGLONG(a+4));
            a += 8;
            break;
        case 'b':
            q += snprint(q, left, " %d", *a++);
            break;
        case 's':
            q += snprint(q, left, " %d", BGSHORT(a));
            a += 2;
            break;
        case 'S':
            q += snprint(q, left, " %.4ux", BGSHORT(a));
            a += 2;
            break;
        }
    }
    *q++ = '\n';
    *q = '\0';
    iprint("%.*s", (int)(q-buf), buf);
}
/*e: function printmesg */

/*s: function drawmesg */
void
drawmesg(Client *client, void *av, int n)
{
    /*s: [[drawmesg()]] locals */
    byte *a;
    int m = 0;
    char *fmt = nil;
    /*x: [[drawmesg()]] locals */
    int dstid, scrnid;
    byte refresh;
    ulong chan;
    int repl;
    Rectangle r, clipr;
    // rgba
    ulong value;

    Memimage *i;
    /*x: [[drawmesg()]] locals */
    DImage *ll;
    /*x: [[drawmesg()]] locals */
    DImage *ddst;
    Memimage *dst;
    /*x: [[drawmesg()]] locals */
    int c, j;
    DImage *di;
    DName *dn;
    /*x: [[drawmesg()]] locals */
    Memimage *src, *mask;
    Point p, q;
    // enum<Drawop>
    int op;
    /*x: [[drawmesg()]] locals */
    Point sp;
    // enum<EndLine>
    int e0, e1;
    /*x: [[drawmesg()]] locals */
    int ni;
    Point *pp;
    bool doflush;
    int ox, oy;
    int esize, oesize;
    byte *u;
    int y;
    /*x: [[drawmesg()]] locals */
    DImage *font;
    Memimage  *bg;
    int ci;
    /*x: [[drawmesg()]] locals */
    FChar *fc;
    /*x: [[drawmesg()]] locals */
    DImage *dsrc;
    /*x: [[drawmesg()]] locals */
    CScreen *cs;
    /*x: [[drawmesg()]] locals */
    DScreen *dscrn;
    /*x: [[drawmesg()]] locals */
    Memscreen *scrn;
    Memimage *l;
    /*x: [[drawmesg()]] locals */
    int nw;
    Memimage **lp;
    /*x: [[drawmesg()]] locals */
    Refreshfn reffn;
    Refx *refx;
    /*e: [[drawmesg()]] locals */

    a = av;
    if(waserror()){
        if(fmt) 
            printmesg(fmt, a, false);
        nexterror();
    }
    while((n-=m) > 0){
        USED(fmt);
        a += m;
        // dispatch on first letter of the drawing command
        switch(*a){
        /*s: [[drawmesg()]] cases */
        /* new allocate: 'b' id[4] screenid[4] refresh[1] chan[4] repl[1] R[4*4] clipR[4*4] rrggbbaa[4] */
        case 'b':
            printmesg(fmt="LLbLbRRL", a, false);
            m = 1+4+4+1+4+1+4*4+4*4+4;
             /*s: [[drawmesg()]] sanity check n with m */
             if(n < m)
                 error(Eshortdraw);
             /*e: [[drawmesg()]] sanity check n with m */
            dstid = BGLONG(a+1);

            scrnid = BGSHORT(a+5);
            refresh = a[9];

            chan = BGLONG(a+10);
            repl = a[14];
            drawrectangle(&r, a+15);
            drawrectangle(&clipr, a+31);
            value = BGLONG(a+47);

            /*s: [[drawmesg()]] allocate image case, sanity check dstid */
            if(drawlookup(client, dstid, false))
                error(Eimageexists);
            /*e: [[drawmesg()]] allocate image case, sanity check dstid */
            /*s: [[drawmesg()]] allocate image case, if screen id */
            if(scrnid){
                dscrn = drawlookupscreen(client, scrnid, &cs);
                scrn = dscrn->screen;
                /*s: [[drawmesg()]] when allocate window, sanity check repl and chan */
                if(repl || chan != scrn->image->chan)
                    error("image parameters incompatible with screen");
                /*e: [[drawmesg()]] when allocate window, sanity check repl and chan */
                /*s: [[drawmesg()]] when allocate window, set reffn */
                reffn = nil;
                switch(refresh){
                case Refbackup:
                    break;
                case Refnone:
                    reffn = memlnorefresh;
                    break;
                case Refmesg:
                    reffn = drawrefresh;
                    break;
                default:
                    error("unknown refresh method");
                }
                /*e: [[drawmesg()]] when allocate window, set reffn */
                l = memlalloc(scrn, r, reffn, nil, value); // The call
                /*s: [[drawmesg()]] when allocate window, sanity check l */
                if(l == nil)
                    error(Edrawmem);
                /*e: [[drawmesg()]] when allocate window, sanity check l */
                /*s: [[drawmesg()]] when allocate window, addflush */
                addflush(l->layer->screenr);
                /*e: [[drawmesg()]] when allocate window, addflush */

                // similar to regular allocate image case
                l->clipr = clipr;
                rectclip(&l->clipr, r);

                if(drawinstall(client, dstid, l, dscrn) == 0){
                    memldelete(l);
                    error(Edrawmem);
                }
                dscrn->ref++;

                /*s: [[drawmesg()]] when allocate window, if reffn */
                if(reffn){
                    refx = nil;
                    if(reffn == drawrefresh){
                        refx = malloc(sizeof(Refx));
                        if(refx == 0){
                            drawuninstall(client, dstid);
                            error(Edrawmem);
                        }
                        refx->client = client;
                        refx->dimage = drawlookup(client, dstid, true);
                    }
                    memlsetrefresh(l, reffn, refx);
                }
                /*e: [[drawmesg()]] when allocate window, if reffn */
                continue;
            }
            /*e: [[drawmesg()]] allocate image case, if screen id */
            // else

            i = allocmemimage(r, chan); // server side allocation
            /*s: [[drawmesg()]] allocate image case, sanity check i */
            if(i == nil)
                error(Edrawmem);
            /*e: [[drawmesg()]] allocate image case, sanity check i */
            if(repl)
                i->flags |= Frepl;
            i->clipr = clipr;
            /*s: [[drawmesg()]] allocate image case, clip clipr */
            if(!repl)
                rectclip(&i->clipr, r);
            /*e: [[drawmesg()]] allocate image case, clip clipr */

            if(drawinstall(client, dstid, i, nil) == nil){
                freememimage(i);
                error(Edrawmem);
            }
            memfillcolor(i, value);
            continue;
        /*x: [[drawmesg()]] cases */
        /* free: 'f' id[4] */
        case 'f':
            printmesg(fmt="L", a, false);
            m = 1+4;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            ll = drawlookup(client, BGLONG(a+1), false);
            /*s: [[drawmesg()]] free image case, if dscreen */
            if(ll && ll->dscreen && ll->dscreen->owner != client)
                ll->dscreen->owner->refreshme = 1;
            /*e: [[drawmesg()]] free image case, if dscreen */
            drawuninstall(client, BGLONG(a+1)); // The call

            continue;
        /*x: [[drawmesg()]] cases */
        /* set repl and clip: 'c' dstid[4] repl[1] clipR[4*4] */
        case 'c':
            printmesg(fmt="LbR", a, false);
            m = 1+4+1+4*4;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            ddst = drawlookup(client, BGLONG(a+1), true);
            /*s: [[drawmesg()]] clipping case, sanity check ddst */
            if(ddst == nil)
                error(Enodrawimage);
            /*x: [[drawmesg()]] clipping case, sanity check ddst */
            if(ddst->name)
                error("cannot change repl/clipr of shared image");
            /*e: [[drawmesg()]] clipping case, sanity check ddst */
            dst = ddst->image;

            if(a[5])
                dst->flags |= Frepl;
            drawrectangle(&dst->clipr, a+6); // The call

            continue;

        /*x: [[drawmesg()]] cases */
        /* visible: 'v' */
        case 'v':
            printmesg(fmt="", a, false);
            m = 1;

            drawflush(); // The call

            continue;

        /*x: [[drawmesg()]] cases */
        /* name an image: 'N' dstid[4] in[1] j[1] name[j] */
        case 'N':
            printmesg(fmt="Lbz", a, false);
            m = 1+4+1+1;
             /*s: [[drawmesg()]] sanity check n with m */
             if(n < m)
                 error(Eshortdraw);
             /*e: [[drawmesg()]] sanity check n with m */
            c = a[5];
            j = a[6];
             /*s: [[drawmesg()]] name an image case, sanity check j */
             if(j == 0)  /* give me a non-empty name please */
                 error(Eshortdraw);
             /*e: [[drawmesg()]] name an image case, sanity check j */
            m += j;
             /*s: [[drawmesg()]] sanity check n with m */
             if(n < m)
                 error(Eshortdraw);
             /*e: [[drawmesg()]] sanity check n with m */

            di = drawlookup(client, BGLONG(a+1), false);
            /*s: [[drawmesg()]] name an image case, sanity check di */
            if(di == nil)
                error(Enodrawimage);
            if(di->name)
                error(Enamed);
            /*e: [[drawmesg()]] name an image case, sanity check di */

            if(c)
                drawaddname(client, di, j, (char*)a+7); // The call
            else{
                dn = drawlookupname(j, (char*)a+7);
                /*s: [[drawmesg()]] name an image case, sanity check dn */
                if(dn == nil)
                    error(Enoname);
                if(dn->dimage != di)
                    error(Ewrongname);
                /*e: [[drawmesg()]] name an image case, sanity check dn */
                drawdelname(dn);
            }
            continue;
        /*x: [[drawmesg()]] cases */
        /* attach to a named image: 'n' dstid[4] j[1] name[j] */
        case 'n':
            printmesg(fmt="Lz", a, false);
            m = 1+4+1;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            j = a[5];
            /*s: [[drawmesg()]] name an image case, sanity check j */
            if(j == 0)  /* give me a non-empty name please */
                error(Eshortdraw);
            /*e: [[drawmesg()]] name an image case, sanity check j */
            m += j;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            dstid = BGLONG(a+1);

            if(drawlookup(client, dstid, false))
                error(Eimageexists);

            dn = drawlookupname(j, (char*)a+6); // The call
             /*s: [[drawmesg()]] attach to an image case, sanity check dn */
             if(dn == nil)
                 error(Enoname);
             /*e: [[drawmesg()]] attach to an image case, sanity check dn */
            // create new DImage with shared underlying Memimage of dn
            if(drawinstall(client, dstid, dn->dimage->image, nil) == 0)
                error(Edrawmem);
            di = drawlookup(client, dstid, false);
            /*s: [[drawmesg()]] attach to an image case, sanity check di */
            if(di == nil)
                error("draw: cannot happen");
            /*e: [[drawmesg()]] attach to an image case, sanity check di */

            /*s: [[drawmesg()]] attach to an image case, set di name fields using dn */
            di->name = smalloc(j+1);
            memmove(di->name, a+6, j);
            di->name[j] = '\0';
            di->fromname = dn->dimage;
            di->fromname->ref++;
            di->vers = dn->vers;
            /*e: [[drawmesg()]] attach to an image case, set di name fields using dn */

            // for further read on /dev/draw/x/ctl
            client->infoid = dstid;
            continue;
        /*x: [[drawmesg()]] cases */
        /* draw: 'd' dstid[4] srcid[4] maskid[4] R[4*4] P[2*4] P[2*4] */
        case 'd':
            printmesg(fmt="LLLRPP", a, false);
            m = 1+4+4+4+4*4+2*4+2*4;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            dstid = BGLONG(a+1);
            dst = drawimage(client, a+1);
            src = drawimage(client, a+5);
            mask = drawimage(client, a+9);
            drawrectangle(&r, a+13);
            drawpoint(&p, a+29);
            drawpoint(&q, a+37);

            op = drawclientop(client);
            memdraw(dst, r, src, p, mask, q, op); // the call!

            dstflush(dstid, dst, r);
            continue;
        /*x: [[drawmesg()]] cases */
        /* set compositing operator for next draw operation: 'O' op */
        case 'O':
            printmesg(fmt="b", a, false);
            m = 1+1;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            client->op = a[1];
            continue;
        /*x: [[drawmesg()]] cases */
        /* draw line: 'L' dstid[4] p0[2*4] p1[2*4] end0[4] end1[4] radius[4] srcid[4] sp[2*4] */
        case 'L':
            printmesg(fmt="LPPlllLP", a, false);
            m = 1+4+2*4+2*4+4+4+4+4+2*4;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            dst = drawimage(client, a+1);
            dstid = BGLONG(a+1);
            drawpoint(&p, a+5);
            drawpoint(&q, a+13);
            e0 = BGLONG(a+21);
            e1 = BGLONG(a+25);
            j = BGLONG(a+29);
            /*s: [[drawmesg()]] when draw line, sanity check j */
            if(j < 0)
                error("negative line width");
            /*e: [[drawmesg()]] when draw line, sanity check j */
            src = drawimage(client, a+33);
            drawpoint(&sp, a+37);
            op = drawclientop(client);

            memline(dst, p, q, e0, e1, j, src, sp, op); // The call
            /*s: [[drawmesg()]] when draw line, possible flush */
            /* avoid memlinebbox if possible */
            if(dstid==0 || dst->layer!=nil){
                /* BUG: this is terribly inefficient: update maximal containing rect*/
                r = memlinebbox(p, q, e0, e1, j);
                dstflush(dstid, dst, insetrect(r, -(1+1+j)));
            }
            /*e: [[drawmesg()]] when draw line, possible flush */
            continue;
        /*x: [[drawmesg()]] cases */
        /* filled polygon: 'P' dstid[4] n[2] wind[4] ignore[2*4] srcid[4] sp[2*4] p0[2*4] dp[2*2*n] */
        /* polygon: 'p' dstid[4] n[2] end0[4] end1[4] radius[4] srcid[4] sp[2*4] p0[2*4] dp[2*2*n] */
        case 'p':
        case 'P':
            printmesg(fmt="LslllLPP", a, false);
            m = 1+4+2+4+4+4+4+2*4;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            dstid = BGLONG(a+1);
            dst = drawimage(client, a+1);
            ni = BGSHORT(a+5);
            /*s: [[drawmesg()]] when draw polygon, sanity check ni */
            if(ni < 0)
                error("negative count in polygon");
            /*e: [[drawmesg()]] when draw polygon, sanity check ni */
            e0 = BGLONG(a+7);
            e1 = BGLONG(a+11);
            j = 0;
            if(*a == 'p'){
                j = BGLONG(a+15);
                /*s: [[drawmesg()]] when draw polygon, sanity check j */
                if(j < 0)
                    error("negative polygon line width");
                /*e: [[drawmesg()]] when draw polygon, sanity check j */
            }
            src = drawimage(client, a+19);
            drawpoint(&sp, a+23);
            drawpoint(&p, a+31);
            ni++;
            pp = malloc(ni*sizeof(Point));
            /*s: [[drawmesg()]] when draw polygon, sanity check pp */
            if(pp == nil)
                error(Enomem);
            /*e: [[drawmesg()]] when draw polygon, sanity check pp */

            /*s: [[drawmesg()]] when draw polygon, set doflush */
            doflush = false;
            if(dstid==0 || (dst->layer && dst->layer->screen->image->data == screenimage->data))
                doflush = true;    /* simplify test in loop */
            /*e: [[drawmesg()]] when draw polygon, set doflush */

            ox = oy = 0;
            esize = 0;
            u = a+m;
            for(y=0; y<ni; y++){
                q = p;
                oesize = esize;
                u = drawcoord(u, a+n, ox, &p.x);
                u = drawcoord(u, a+n, oy, &p.y);
                ox = p.x;
                oy = p.y;

                /*s: [[drawmesg()]] when draw polygon, if doflush */
                if(doflush){
                    esize = j;
                    if(*a == 'p'){
                        if(y == 0){
                            c = memlineendsize(e0);
                            if(c > esize)
                                esize = c;
                        }
                        if(y == ni-1){
                            c = memlineendsize(e1);
                            if(c > esize)
                                esize = c;
                        }
                    }
                    if(*a=='P' && e0!=1 && e0 !=~0)
                        r = dst->clipr;
                    else if(y > 0){
                        r = Rect(q.x-oesize, q.y-oesize, q.x+oesize+1, q.y+oesize+1);
                        combinerect(&r, Rect(p.x-esize, p.y-esize, p.x+esize+1, p.y+esize+1));
                    }
                    if(rectclip(&r, dst->clipr))  /* should perhaps be an arg to dstflush */
                        dstflush(dstid, dst, r);
                }
                /*e: [[drawmesg()]] when draw polygon, if doflush */
                pp[y] = p;
            }
            /*s: [[drawmesg()]] when draw polygon, special flush if y is 1 */
            if(y == 1)
                dstflush(dstid, dst, Rect(p.x-esize, p.y-esize, p.x+esize+1, p.y+esize+1));
            /*e: [[drawmesg()]] when draw polygon, special flush if y is 1 */
            op = drawclientop(client);

            if(*a == 'p')
                mempoly(dst, pp, ni, e0, e1, j, src, sp, op); // The call
            else
                memfillpoly(dst, pp, ni, e0, src, sp, op); // The call

            free(pp);
            m = u-a;
            continue;

        /*x: [[drawmesg()]] cases */
        /* ellipse: 'e' dstid[4] srcid[4] center[2*4] a[4] b[4] thick[4] sp[2*4] alpha[4] phi[4]*/
        case 'e':
        case 'E':
            printmesg(fmt="LLPlllPll", a, false);
            m = 1+4+4+2*4+4+4+4+2*4+2*4;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            dst = drawimage(client, a+1);
            dstid = BGLONG(a+1);
            src = drawimage(client, a+5);
            drawpoint(&p, a+9);
            e0 = BGLONG(a+17);
            e1 = BGLONG(a+21);
            /*s: [[drawmesg()]] when draw ellipse, sanity check widths */
            if(e0<0 || e1<0)
                error("invalid ellipse semidiameter");
            /*e: [[drawmesg()]] when draw ellipse, sanity check widths */
            j = BGLONG(a+25);
            /*s: [[drawmesg()]] when draw ellipse, sanity check j */
            if(j < 0)
                error("negative ellipse thickness");
            /*e: [[drawmesg()]] when draw ellipse, sanity check j */
            drawpoint(&sp, a+29);
            c = j;
            if(*a == 'E')
                c = -1;
            ox = BGLONG(a+37);
            oy = BGLONG(a+41);
            op = drawclientop(client);
 
           /*s: [[drawmesg()]] when draw ellipse, if ox */
           /* high bit indicates arc angles are present */

           if(ox & (1<<31)){
               if((ox & (1<<30)) == 0)
                   ox &= ~(1<<31);
               memarc(dst, p, e0, e1, c, src, sp, ox, oy, op); // The call
           }
           /*e: [[drawmesg()]] when draw ellipse, if ox */
            else
                memellipse(dst, p, e0, e1, c, src, sp, op); // The call

           /*s: [[drawmesg()]] when draw ellipse, dstflush */
           dstflush(dstid, dst, Rect(p.x-e0-j, p.y-e1-j, p.x+e0+j+1, p.y+e1+j+1));
           /*e: [[drawmesg()]] when draw ellipse, dstflush */

            continue;
        /*x: [[drawmesg()]] cases */
        /* string: 's' dstid[4] srcid[4] fontid[4] P[2*4] clipr[4*4] sp[2*4] ni[2] ni*(index[2]) */
        /* stringbg: 'x' dstid[4] srcid[4] fontid[4] P[2*4] clipr[4*4] sp[2*4] ni[2] bgid[4] bgpt[2*4] ni*(index[2]) */
        case 's':
        case 'x':
            printmesg(fmt="LLLPRPs", a, false);
            m = 1+4+4+4+2*4+4*4+2*4+2;
            /*s: [[drawmesg()]] when draw string, if bg part1 */
            if(*a == 'x')
                m += 4+2*4;
            /*e: [[drawmesg()]] when draw string, if bg part1 */
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            dst = drawimage(client, a+1);
            dstid = BGLONG(a+1);
            src = drawimage(client, a+5);
            font = drawlookup(client, BGLONG(a+9), true);
            /*s: [[drawmesg()]] when draw string, sanity check font */
            if(font == nil)
                error(Enodrawimage);
            if(font->nfchar == 0)
                error(Enotfont);
            /*e: [[drawmesg()]] when draw string, sanity check font */
            drawpoint(&p, a+13);
            drawrectangle(&r, a+21);
            drawpoint(&sp, a+37);
            ni = BGSHORT(a+45);
            u = a+m;
            m += ni*2;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            /*s: [[drawmesg()]] change dst clipr */
            clipr = dst->clipr;
            dst->clipr = r;
            /*e: [[drawmesg()]] change dst clipr */
            op = drawclientop(client);
            bg = dst;
            /*s: [[drawmesg()]] when draw string, if paint background */
            if(*a == 'x'){
                /* paint background */
                bg = drawimage(client, a+47);
                drawpoint(&q, a+51);
                r.min.x = p.x;
                r.min.y = p.y - font->ascent;
                r.max.x = p.x;
                r.max.y = r.min.y+Dy(font->image->r);
                j = ni;
                while(--j >= 0){
                    ci = BGSHORT(u);
                    /*s: [[drawmesg()]] when draw string, sanity check ci */
                    if(ci<0 || ci>=font->nfchar){
                        dst->clipr = clipr;
                        error(Eindex);
                    }
                    /*e: [[drawmesg()]] when draw string, sanity check ci */
                    r.max.x += font->fchar[ci].width;
                    u += 2;
                }
                memdraw(dst, r, bg, q, memopaque, ZP, op); // The call
                u -= 2*ni;
            }
            /*e: [[drawmesg()]] when draw string, if paint background */
            q = p;
            while(--ni >= 0){
                ci = BGSHORT(u);
                /*s: [[drawmesg()]] when draw string, sanity check ci */
                if(ci<0 || ci>=font->nfchar){
                    dst->clipr = clipr;
                    error(Eindex);
                }
                /*e: [[drawmesg()]] when draw string, sanity check ci */
                q = drawchar(dst, bg, q, src, &sp, font, ci, op); // The call
                u += 2;
            }
            /*s: [[drawmesg()]] restore dst clipr */
            dst->clipr = clipr;
            /*e: [[drawmesg()]] restore dst clipr */
            /*s: [[drawmesg()]] when draw text, dstflush */
            p.y -= font->ascent;
            dstflush(dstid, dst, Rect(p.x, p.y, q.x, p.y+Dy(font->image->r)));
            /*e: [[drawmesg()]] when draw text, dstflush */
            continue;
        /*x: [[drawmesg()]] cases */
        /* load character: 'l' fontid[4] srcid[4] index[2] R[4*4] P[2*4] left[1] width[1] */
        case 'l':
            printmesg(fmt="LLSRPbb", a, false);
            m = 1+4+4+2+4*4+2*4+1+1;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            font = drawlookup(client, BGLONG(a+1), true);
            /*s: [[drawmesg()]] when load character, sanity check font */
            if(font == nil)
                error(Enodrawimage);
            if(font->nfchar == 0)
                error(Enotfont);
            /*e: [[drawmesg()]] when load character, sanity check font */
            src = drawimage(client, a+5);
            ci = BGSHORT(a+9);
            /*s: [[drawmesg()]] when load character, sanity check ci */
            if(ci >= font->nfchar)
                error(Eindex);
            /*e: [[drawmesg()]] when load character, sanity check ci */
            drawrectangle(&r, a+11);
            drawpoint(&p, a+27);
            memdraw(font->image, r, src, p, memopaque, p, S);

            fc = &font->fchar[ci];

            fc->minx = r.min.x;
            fc->maxx = r.max.x;
            fc->miny = r.min.y;
            fc->maxy = r.max.y;

            fc->left = a[35];
            fc->width = a[36];

            continue;
        /*x: [[drawmesg()]] cases */
        /* initialize font: 'i' fontid[4] nchars[4] ascent[1] */
        case 'i':
            printmesg(fmt="Llb", a, false);
            m = 1+4+4+1;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            dstid = BGLONG(a+1);
            /*s: [[drawmesg()]] when initialize font image and cache, sanity check dstid */
            if(dstid == 0)
                error("cannot use display as font");
            /*e: [[drawmesg()]] when initialize font image and cache, sanity check dstid */
            font = drawlookup(client, dstid, true);
            /*s: [[drawmesg()]] when initialize font image and cache, sanity check font */
            if(font == nil)
                error(Enodrawimage);
            if(font->image->layer)
                error("cannot use window as font");
            /*e: [[drawmesg()]] when initialize font image and cache, sanity check font */
            ni = BGLONG(a+5);
            /*s: [[drawmesg()]] when initialize font image and cache, sanity check ni */
            if(ni<=0 || ni>4096)
                error("bad font size (4096 chars max)");
            /*e: [[drawmesg()]] when initialize font image and cache, sanity check ni */
            free(font->fchar);  /* should we complain if non-zero? */
            font->fchar = malloc(ni * sizeof(FChar));
            /*s: [[drawmesg()]] when initialize font image and cache, sanity check fchar */
            if(font->fchar == nil)
                error("no memory for font");
            /*e: [[drawmesg()]] when initialize font image and cache, sanity check fchar */
            memset(font->fchar, 0, ni*sizeof(FChar));
            font->nfchar = ni;
            font->ascent = a[9];
            continue;

        /*x: [[drawmesg()]] cases */
        /* write: 'y' id[4] R[4*4] data[x*1] */
        /* write from compressed data: 'Y' id[4] R[4*4] data[x*1] */
        case 'y':
        case 'Y':
            printmesg(fmt="LR", a, false);
            m = 1+4+4*4;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            dstid = BGLONG(a+1);
            dst = drawimage(client, a+1);
            drawrectangle(&r, a+5);
            /*s: [[drawmesg()]] when load an image, sanity check r */
            if(!rectinrect(r, dst->r))
                error(Ewriteoutside);
            /*e: [[drawmesg()]] when load an image, sanity check r */
            y = memload(dst, r, a+m, n-m, *a=='Y'); // The call
            /*s: [[drawmesg()]] when load an image, sanity check y */
            if(y < 0)
                error("bad writeimage call");
            /*e: [[drawmesg()]] when load an image, sanity check y */
            dstflush(dstid, dst, r);
            m += y;
            continue;
        /*x: [[drawmesg()]] cases */
        /* read: 'r' id[4] R[4*4] */
        case 'r':
            printmesg(fmt="LR", a, false);
            m = 1+4+4*4;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            i = drawimage(client, a+1);
            drawrectangle(&r, a+5);
            /*s: [[drawmesg()]] when read an image, sanity check r */
            if(!rectinrect(r, i->r))
                error(Ereadoutside);
            /*e: [[drawmesg()]] when read an image, sanity check r */
            c = bytesperline(r, i->depth);
            c *= Dy(r);

            free(client->readdata);
            client->readdata = mallocz(c, 0);
            /*s: [[drawmesg()]] when read an image, sanity check readdata */
            if(client->readdata == nil)
                error("readimage malloc failed");
            /*e: [[drawmesg()]] when read an image, sanity check readdata */
            client->nreaddata = memunload(i, r, client->readdata, c); // The call
            /*s: [[drawmesg()]] when read an image, sanity check nreaddata */
            if(client->nreaddata < 0){
                free(client->readdata);
                client->readdata = nil;
                error("bad readimage call");
            }
            /*e: [[drawmesg()]] when read an image, sanity check nreaddata */
            continue;
        /*x: [[drawmesg()]] cases */
        /* allocate screen: 'A' id[4] imageid[4] fillid[4] public[1] */
        case 'A':
            printmesg(fmt="LLLb", a, false);
            m = 1+4+4+4+1;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            dstid = BGLONG(a+1);
            /*s: [[drawmesg()]] when allocate screen, sanity check dstid */
            if(dstid == 0)
                error(Ebadarg);
            if(drawlookupdscreen(dstid))
                error(Escreenexists);
            /*e: [[drawmesg()]] when allocate screen, sanity check dstid */
            ddst = drawlookup(client, BGLONG(a+5), true);
            dsrc = drawlookup(client, BGLONG(a+9), true);
            /*s: [[drawmesg()]] when allocate screen, sanity check ddst and dsrc */
            if(ddst==nil || dsrc==nil)
                error(Enodrawimage);
            /*e: [[drawmesg()]] when allocate screen, sanity check ddst and dsrc */

            if(drawinstallscreen(client, nil, dstid, ddst, dsrc, a[13]) == 0)// The call
                error(Edrawmem);
            continue;

        /*x: [[drawmesg()]] cases */
        /* free screen: 'F' id[4] */
        case 'F':
            printmesg(fmt="L", a, false);
            m = 1+4;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            drawlookupscreen(client, BGLONG(a+1), &cs);
            drawuninstallscreen(client, cs); // The call
            continue;

        /*x: [[drawmesg()]] cases */
        /* use public screen: 'S' id[4] chan[4] */
        case 'S':
            printmesg(fmt="Ll", a, false);
            m = 1+4+4;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            dstid = BGLONG(a+1);
            /*s: [[drawmesg()]] when use public screen, sanity check dstid */
            if(dstid == 0)
                error(Ebadarg);
            /*e: [[drawmesg()]] when use public screen, sanity check dstid */
            dscrn = drawlookupdscreen(dstid);
            /*s: [[drawmesg()]] when use public screen, sanity check dscrn */
            if(dscrn == nil || (!dscrn->public && dscrn->owner != client))
                error(Enodrawscreen);
            if(dscrn->screen->image->chan != BGLONG(a+5))
                error("inconsistent chan");
            /*e: [[drawmesg()]] when use public screen, sanity check dscrn */
            if(drawinstallscreen(client, dscrn, 0, nil, nil, false) == 0) // The call
                error(Edrawmem);
            continue;
        /*x: [[drawmesg()]] cases */
        /* top or bottom windows: 't' top[1] nw[2] n*id[4] */
        case 't':
            printmesg(fmt="bsL", a, false);
            m = 1+1+2;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            nw = BGSHORT(a+2);
            /*s: [[drawmesg()]] when top or bottom windows, sanity check nw */
            if(nw < 0)
                error(Ebadarg);
            if(nw == 0)
                continue;
            /*e: [[drawmesg()]] when top or bottom windows, sanity check nw */
            m += nw*4;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            lp = malloc(nw * sizeof(Memimage*));
            /*s: [[drawmesg()]] when top or bottom windows, sanity check lp */
            if(lp == nil)
                error(Enomem);
            if(waserror()){
                free(lp);
                nexterror();
            }
            /*e: [[drawmesg()]] when top or bottom windows, sanity check lp */
            for(j=0; j<nw; j++)
                lp[j] = drawimage(client, a+1+1+2+j*4);
            /*s: [[drawmesg()]] when top or bottom windows, sanity check windows */
            if(lp[0]->layer == nil)
                error("images are not windows");
            for(j=1; j<nw; j++)
                if(lp[j]->layer->screen != lp[0]->layer->screen)
                    error("images not on same screen");
            /*e: [[drawmesg()]] when top or bottom windows, sanity check windows */

            if(a[1])
                memltofrontn(lp, nw); // The call
            else
                memltorearn(lp, nw); // The call

            /*s: [[drawmesg()]] when top or bottom windows, addflush */
            if(lp[0]->layer->screen->image->data == screenimage->data)
                for(j=0; j<nw; j++)
                    addflush(lp[j]->layer->screenr);
            /*e: [[drawmesg()]] when top or bottom windows, addflush */
            /*s: [[drawmesg()]] when top or bottom windows, refresh */
            ll = drawlookup(client, BGLONG(a+1+1+2), true);
            drawrefreshscreen(ll, client);
            /*e: [[drawmesg()]] when top or bottom windows, refresh */

            poperror();
            free(lp);
            continue;

        /*x: [[drawmesg()]] cases */
        /* position window: 'o' id[4] r.min [2*4] screenr.min [2*4] */
        case 'o':
            printmesg(fmt="LPP", a, false);
            m = 1+4+2*4+2*4;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            dst = drawimage(client, a+1);
            if(dst->layer){
                drawpoint(&p, a+5);
                drawpoint(&q, a+13);
                r = dst->layer->screenr;

                ni = memlorigin(dst, p, q); // The call

                /*s: [[drawmesg()]] when position window, sanity check ni */
                if(ni < 0)
                    error("image origin failed");
                /*e: [[drawmesg()]] when position window, sanity check ni */
                if(ni > 0){
                    /*s: [[drawmesg()]] when position window, addflush */
                    addflush(r);
                    addflush(dst->layer->screenr);
                    /*e: [[drawmesg()]] when position window, addflush */
                    /*s: [[drawmesg()]] when position window, refresh */
                    ll = drawlookup(client, BGLONG(a+1), true);
                    drawrefreshscreen(ll, client);
                    /*e: [[drawmesg()]] when position window, refresh */
                }
            }
            // else, could display error
            continue;
        /*x: [[drawmesg()]] cases */
        /* toggle debugging: 'D' val[1] */
        case 'D':
            printmesg(fmt="b", a, false);
            m = 1+1;
            /*s: [[drawmesg()]] sanity check n with m */
            if(n < m)
                error(Eshortdraw);
            /*e: [[drawmesg()]] sanity check n with m */
            drawdebug = a[1];
            continue;
        /*e: [[drawmesg()]] cases */
        default:
            error("bad draw command");
        }
    }
    poperror();
}
/*e: function drawmesg */
/*e: kernel/devices/screen/drawmesg.c */
