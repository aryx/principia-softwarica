/*s: lib_graphics/libdraw/naming.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>

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

/*e: lib_graphics/libdraw/naming.c */
