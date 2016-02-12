/*s: lib_graphics/libdraw/alloc.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function allocimage */
Image*
allocimage(Display *d, Rectangle r, ulong chan, bool repl, ulong val)
{
    Image*	i;

    i =  _allocimage(nil, d, r, chan, repl, val, 0, 0);
    /*s: [[allocimage()]] set malloc tag for debug */
    if (i)
        setmalloctag(i, getcallerpc(&d));
    /*e: [[allocimage()]] set malloc tag for debug */
    return i;
}
/*e: function allocimage */

/*s: function _allocimage */
Image*
_allocimage(Image *ai, Display *d, Rectangle r, ulong chan, bool repl, ulong val, int screenid, int refresh)
{
    Image *i = nil;
    char *err = nil;
    byte *a;
    Rectangle clipr;
    int id;
    int depth;

    /*s: [[_allocimage()]] sanity check chan */
    if(chan == 0){
        werrstr("bad channel descriptor");
        return nil;
    }
    /*e: [[_allocimage()]] sanity check chan */
    depth = chantodepth(chan);
    /*s: [[_allocimage()]] sanity check depth */
    if(depth == 0){
        err = "bad channel descriptor";
    Error:
        if(err)
            werrstr("allocimage: %s", err);
        else
            werrstr("allocimage: %r");
        free(i);
        return nil;
    }
    /*e: [[_allocimage()]] sanity check depth */

    /* flush pending data so we don't get error allocating the image */
    flushimage(d, false);

    // new allocate: 'b' id[4] screenid[4] refresh[1] chan[4] repl[1] R[4*4] clipR[4*4] rrggbbaa[4]
    a = bufimage(d, 1+4+4+1+4+1+4*4+4*4+4);
    /*s: [[_allocimage()]] sanity check a */
    if(a == nil)
        goto Error;
    /*e: [[_allocimage()]] sanity check a */

    d->imageid++;
    id = d->imageid;

    a[0] = 'b';
    BPLONG(a+1, id);

    BPLONG(a+5, screenid);
    a[9] = refresh;

    BPLONG(a+10, chan);
    a[14] = repl;
    BPLONG(a+15, r.min.x);
    BPLONG(a+19, r.min.y);
    BPLONG(a+23, r.max.x);
    BPLONG(a+27, r.max.y);
    /*s: [[_allocimage()]] set clipr */
    if(repl)
        /* huge but not infinite, so various offsets will leave it huge, not overflow */
        clipr = Rect(-0x3FFFFFFF, -0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFFFF);
    else
        clipr = r;
    /*e: [[_allocimage()]] set clipr */
    BPLONG(a+31, clipr.min.x);
    BPLONG(a+35, clipr.min.y);
    BPLONG(a+39, clipr.max.x);
    BPLONG(a+43, clipr.max.y);
    BPLONG(a+47, val);

    if(flushimage(d, false) < 0)
        goto Error;

    /*s: [[_allocimage()]] if passed image */
    if(ai)
        i = ai;
    /*e: [[_allocimage()]] if passed image */
    else{
        i = malloc(sizeof(Image)); // client side allocation
        /*s: [[_allocimage()]] sanity check i */
        if(i == nil){
            // free: 'f' id[4]
            a = bufimage(d, 1+4);
            if(a){
                a[0] = 'f';
                BPLONG(a+1, id);
                flushimage(d, false);
            }
            goto Error;
        }
        /*e: [[_allocimage()]] sanity check i */
    }

    i->display = d;
    i->id = id;
    i->depth = depth;
    i->chan = chan;
    i->r = r;
    i->clipr = clipr;
    i->repl = repl;

    i->screen = nil;
    i->next = nil;

    return i;
}
/*e: function _allocimage */

/*s: function namedimage */
Image*
namedimage(Display *d, char *name)
{
    /*s: [[namedimage()]] body */
    byte *a;
    char *err = nil;
    char buf[12*12+1]; // NINFO+1
    Image *i = nil;
    int id, n;
    ulong chan;

    n = strlen(name);
    /*s: [[namedimage()]] sanity check n */
    if(n >= 256){
        err = "name too long";
    Error:
        if(err)
            werrstr("namedimage: %s", err);
        else
            werrstr("namedimage: %r");
        if(i)
            free(i);
        return nil;
    }
    /*e: [[namedimage()]] sanity check n */
    /* flush pending data so we don't get error allocating the image */
    flushimage(d, false);

    // attach to a named image: 'n' dstid[4] j[1] name[j]
    a = bufimage(d, 1+4+1+n);
    /*s: [[namedimage()]] sanity check a */
    if(a == nil)
        goto Error;
    /*e: [[namedimage()]] sanity check a */

    d->imageid++;
    id = d->imageid;

    a[0] = 'n';
    BPLONG(a+1, id);
    a[5] = n;
    memmove(a+6, name, n);

    if(flushimage(d, false) < 0)
        goto Error;

    if(pread(d->ctlfd, buf, sizeof buf, 0) < 12*12)
        goto Error;
    buf[12*12] = '\0';

    i = malloc(sizeof(Image));
    /*s: [[namedimage()]] sanity check i */
    if(i == nil){
    Error1:
        a = bufimage(d, 1+4);
        if(a){
            a[0] = 'f';
            BPLONG(a+1, id);
            flushimage(d, false);
        }
        goto Error;
    }
    /*e: [[namedimage()]] sanity check i */

    i->display = d;
    i->id = id;
    chan=strtochan(buf+2*12);
    /*s: [[namedimage()]] sanity check chan */
    if(chan == 0){
        werrstr("bad channel '%.12s' from devdraw", buf+2*12);
        goto Error1;
    }
    /*e: [[namedimage()]] sanity check chan */
    i->chan = chan;
    i->depth = chantodepth(chan);
    i->repl = atoi(buf+3*12);
    i->r.min.x = atoi(buf+4*12);
    i->r.min.y = atoi(buf+5*12);
    i->r.max.x = atoi(buf+6*12);
    i->r.max.y = atoi(buf+7*12);
    i->clipr.min.x = atoi(buf+8*12);
    i->clipr.min.y = atoi(buf+9*12);
    i->clipr.max.x = atoi(buf+10*12);
    i->clipr.max.y = atoi(buf+11*12);

    i->screen = nil;
    i->next = nil;

    return i;
    /*e: [[namedimage()]] body */
}
/*e: function namedimage */

/*s: function nameimage */
error0
nameimage(Image *i, char *name, bool in)
{
    /*s: [[nameimage()]] body */
    byte *a;
    int n;

    n = strlen(name);
    // name an image: 'N' dstid[4] in[1] j[1] name[j]
    a = bufimage(i->display, 1+4+1+1+n);
    /*s: [[nameimage()]] sanity check a */
    if(a == nil)
        return 0;
    /*e: [[nameimage()]] sanity check a */
    a[0] = 'N';
    BPLONG(a+1, i->id);
    a[5] = in;
    a[6] = n;
    memmove(a+7, name, n);

    if(flushimage(i->display, false) < 0)
        return ERROR_0;
    return OK_1;
    /*e: [[nameimage()]] body */
}
/*e: function nameimage */

/*s: function _freeimage1 */
errorneg1
_freeimage1(Image *i)
{
    byte *a;
    Display *d;
    Image *w;

    /*s: [[_freeimage1()]] sanity check i */
    if(i == nil || i->display == nil)
        return OK_0;
    /*e: [[_freeimage1()]] sanity check i */

    /* make sure no refresh events occur on this if we block in the write */
    d = i->display;
    /* flush pending data so we don't get error deleting the image */
    flushimage(d, false);

    // free: 'f' id[4]
    a = bufimage(d, 1+4);
    /*s: [[_freeimage1()]] sanity check a */
    if(a == nil)
        return ERROR_NEG1;
    /*e: [[_freeimage1()]] sanity check a */
    a[0] = 'f';
    BPLONG(a+1, i->id);
    /*s: [[_freeimage1()]] if screen */
    if(i->screen){
        // remove_list(i, d->windows)
        w = d->windows;
        if(w == i)
            d->windows = i->next;
        else
            while(w){
                if(w->next == i){
                    w->next = i->next;
                    break;
                }
                w = w->next;
            }
    }
    /*e: [[_freeimage1()]] if screen */

    if(flushimage(d, i->screen != nil) < 0)
        return ERROR_NEG1;

    return OK_0;
}
/*e: function _freeimage1 */

/*s: function freeimage */
errorneg1
freeimage(Image *i)
{
    errorneg1 ret;

    ret = _freeimage1(i);
    free(i);
    return ret;
}
/*e: function freeimage */
/*e: lib_graphics/libdraw/alloc.c */
