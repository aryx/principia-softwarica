/*s: kernel/devices/screen/devdraw.c */
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

/*s: enum QxxxDraw */
enum
{
    // Directories

    Qtopdir     = 0, // /dev
    Q2nd,            // /dev/draw/
    Q3rd,            // /dev/draw/x/

    // Files

    Qnew,
    Qwinname,

    // /dev/draw/x/y, third level device files associated to a client
    Qctl,  // used in 'x < Qctl' code, so must be the first!
    Qdata, // used for drawing commands, see drawmesg()

    Qcolormap, 
    Qrefresh,
};
/*e: enum QxxxDraw */

/*s: constant QSHIFT */
/*
 * Qid path is:
 *   4 bits of file type (qids above)
 *  24 bits of mux slot number +1; 0 means not attached to client
 */
#define QSHIFT  4   /* location in qid of client # */
/*e: constant QSHIFT */

/*s: function QID bis */
#define QID(q)      ((((ulong)(q).path)&0x0000000F)>>0)
/*e: function QID bis */
/*s: function CLIENTPATH */
#define CLIENTPATH(q)   ((((ulong)q)&0x7FFFFFF0)>>QSHIFT)
/*e: function CLIENTPATH */

/*s: constant IOUNIT */
#define IOUNIT      (64*1024)
/*e: constant IOUNIT */

/*s: global sdraw */
KDraw        sdraw;
/*e: global sdraw */
/*s: global drawlock */
QLock   drawlock;
/*e: global drawlock */


/*s: global flushrect */
static  Rectangle   flushrect;
/*e: global flushrect */
/*s: global waste */
static  int     waste;
/*e: global waste */

// forward decls
void        drawmesg(Client*, void*, int);
void        drawuninstall(Client*, int);
void        drawfreedimage(DImage*);
Client*     drawclientofpath(ulong);
DImage* 	allocdimage(Memimage*);



/*s: function dlock */
void
dlock(void)
{
    qlock(&drawlock);
}
/*e: function dlock */

/*s: function candlock */
int
candlock(void)
{
    return canqlock(&drawlock);
}
/*e: function candlock */

/*s: function dunlock */
void
dunlock(void)
{
    qunlock(&drawlock);
}
/*e: function dunlock */


/*s: function drawgen */
static int
drawgen(Chan *c, char*, Dirtab*, int, int s, Dir *dp)
{
    int t;
    Qid q;
    ulong path;
    Client *cl;

    q.vers = 0;

    /*s: [[drawgen()]] if dotdot */
    if(s == DEVDOTDOT){
        switch(QID(c->qid)){
        case Qtopdir:
        case Q2nd:
            mkqid(&q, Qtopdir, 0, QTDIR);
            devdir(c, q, "#i", 0, eve, 0500, dp);
            break;
        case Q3rd:
            cl = drawclientofpath(c->qid.path);
            if(cl == nil) // when can this happen?
                strncpy(up->genbuf, "??", sizeof up->genbuf);
            else
                snprint(up->genbuf, sizeof up->genbuf,
                    "%d", cl->clientid);
            mkqid(&q, Q2nd, 0, QTDIR);
            devdir(c, q, up->genbuf, 0, eve, 0500, dp);
            break;
        default:
            panic("drawwalk %llux", c->qid.path);
        }
        return 1;
    }
    /*e: [[drawgen()]] if dotdot */
    // else
    t = QID(c->qid);

    /*
     * Top level directory contains the name of the device.
     */
    /*s: [[drawgen()]] toplevel directory listing */
    if(t == Qtopdir){
        switch(s){
        case 0:
            mkqid(&q, Q2nd, 0, QTDIR);
            devdir(c, q, "draw", 0, eve, 0555, dp);
            break;
        case 1:
            mkqid(&q, Qwinname, 0, QTFILE);
            devdir(c, q, "winname", 0, eve, 0444, dp);
            break;
        default:
            return -1;
        }
        return 1;
    }
    // else

    if(t == Qwinname){
        mkqid(&q, Qwinname, 0, QTFILE);
        devdir(c, q, "winname", 0, eve, 0444, dp);
        return 1;
    }
    /*e: [[drawgen()]] toplevel directory listing */
    // else
    /*
     * Second level contains "new" plus all the clients.
     */
    /*s: [[drawgen()]] second level directory listing */
    if(t == Q2nd || t == Qnew){
        if(s == 0){
            mkqid(&q, Qnew, 0, QTFILE);
            devdir(c, q, "new", 0, eve, 0666, dp);
        }
        else if(s <= sdraw.nclient){
            cl = sdraw.client[s-1];
            /*s: [[drawgen()]] in second level directory listing, sanity check cl */
            if(cl == nil)
                return 0;
            /*e: [[drawgen()]] in second level directory listing, sanity check cl */
            snprint(up->genbuf, sizeof up->genbuf, "%d",
                cl->clientid);
            mkqid(&q, (s<<QSHIFT)|Q3rd, 0, QTDIR);
            devdir(c, q, up->genbuf, 0, eve, 0555, dp);
            return 1;
        }
        else
            return -1;
        return 1;
    }
    /*e: [[drawgen()]] second level directory listing */
    // else
    /*
     * Third level.
     */
    /*s: [[drawgen()]] third level directory listing */
    path = c->qid.path & ~((1<<QSHIFT)-1);    /* slot component */
    q.vers = c->qid.vers;
    q.type = QTFILE;
    switch(s){
    case 0:
        q.path = path|Qcolormap;
        devdir(c, q, "colormap", 0, eve, 0600, dp);
        break;
    case 1:
        q.path = path|Qctl;
        devdir(c, q, "ctl", 0, eve, 0600, dp);
        break;
    case 2:
        q.path = path|Qdata;
        devdir(c, q, "data", 0, eve, 0600, dp);
        break;
    case 3:
        q.path = path|Qrefresh;
        devdir(c, q, "refresh", 0, eve, 0400, dp);
        break;
    default:
        return -1;
    }
    return 1;
    /*e: [[drawgen()]] third level directory listing */
}
/*e: function drawgen */




/*s: function addflush */
void
addflush(Rectangle r)
{
    int abb, ar, anbb;
    Rectangle nbb;

    if(!sdraw.softscreen || !rectclip(&r, screenimage->r))
        return;
    if(flushrect.min.x >= flushrect.max.x){
        flushrect = r;
        waste = 0;
        return;
    }
    // else

    nbb = flushrect;
    combinerect(&nbb, r);
    ar = Dx(r)*Dy(r);
    abb = Dx(flushrect)*Dy(flushrect);
    anbb = Dx(nbb)*Dy(nbb);
    /*
     * Area of new waste is area of new bb minus area of old bb,
     * less the area of the new segment, which we assume is not waste.
     * This could be negative, but that's OK.
     */
    waste += anbb-abb - ar;
    if(waste < 0)
        waste = 0;
    /*
     * absorb if:
     *  total area is small
     *  waste is less than half total area
     *  rectangles touch
     */
    if(anbb<=1024 || waste*2<anbb || rectXrect(flushrect, r)){
        flushrect = nbb;
        return;
    }

    /* emit current state */
    if(flushrect.min.x < flushrect.max.x)
        arch_flushmemscreen(flushrect);
    flushrect = r;

    waste = 0;
}
/*e: function addflush */

/*s: function dstflush */
void
dstflush(int dstid, Memimage *dst, Rectangle r)
{
    /*s: [[dstflush()]] locals */
    Memlayer *l;
    /*e: [[dstflush()]] locals */

    if(dstid == 0){ // the screen
        combinerect(&flushrect, r);
        return;
    }
    // else
    /*s: [[dstflush()]] sanity check dst */
    /* how can this happen? -rsc, dec 12 2002 */
    if(dst == nil){
        print("nil dstflush\n");
        return;
    }
    /*e: [[dstflush()]] sanity check dst */
    /*s: [[dstflush()]] if layer */
    l = dst->layer;
    if(l == nil)
        return;

    // else
    do{
        if(l->screen->image->data != screenimage->data)
            return;
        r = rectaddpt(r, l->delta);
        l = l->screen->image->layer;
    }while(l);

    addflush(r);

    /*e: [[dstflush()]] if layer */
}
/*e: function dstflush */

/*s: function drawflush */
void
drawflush(void)
{
    if(flushrect.min.x < flushrect.max.x)
        arch_flushmemscreen(flushrect);
    flushrect = Rect(10000, 10000, -10000, -10000);
}
/*e: function drawflush */




/*s: function drawlookup */
DImage*
drawlookup(Client *client, int id, bool checkname)
{
    DImage *d;

    d = client->dimage[id&HASHMASK];
    while(d){
        if(d->id == id){
            /*s: [[drawlookup()]] if checkname */
            if(checkname && !drawgoodname(d))
                error(Eoldname);
            /*e: [[drawlookup()]] if checkname */
            return d;
        }
        d = d->next;
    }
    return nil;
}
/*e: function drawlookup */


/*s: function drawnewclient */
Client*
drawnewclient(void)
{
    Client *cl, **cp;
    int i;

    // find free slot
    for(i=0; i<sdraw.nclient; i++){
        cl = sdraw.client[i];
        // found one? then i contains the free slot number.
        if(cl == nil)
            break;
    }
    /*s: [[drawnewclient()]] grow array if necessary */
    // growing array
    if(i == sdraw.nclient){
        cp = malloc((sdraw.nclient+1)*sizeof(Client*));
        /*s: [[drawnewclient()]] sanity check cp */
        if(cp == nil)
            return nil;
        /*e: [[drawnewclient()]] sanity check cp */
        memmove(cp, sdraw.client, sdraw.nclient*sizeof(Client*));
        free(sdraw.client);
        sdraw.client = cp;
        sdraw.nclient++;
        cp[i] = nil;
    }
    /*e: [[drawnewclient()]] grow array if necessary */

    cl = malloc(sizeof(Client));
    /*s: [[drawnewclient()]] sanity check cl */
    if(cl == nil)
        return nil;
    /*e: [[drawnewclient()]] sanity check cl */
    memset(cl, 0, sizeof(Client));
    cl->slot = i;
    cl->clientid = ++sdraw.clientid;
    cl->op = SoverD; // The classic
    sdraw.client[i] = cl;
    return cl;
}
/*e: function drawnewclient */


/*s: function drawclientofpath */
Client*
drawclientofpath(ulong path)
{
    Client *cl;
    int slot;

    slot = CLIENTPATH(path);
    if(slot == 0)
        return nil;
    cl = sdraw.client[slot-1];
    if(cl == nil || cl->clientid == 0)
        return nil;
    return cl;
}
/*e: function drawclientofpath */

/*s: function drawclient */
Client*
drawclient(Chan *c)
{
    Client *client;

    client = drawclientofpath(c->qid.path);
    if(client == nil)
        error(Enoclient);
    return client;
}
/*e: function drawclient */











/*s: function drawattach */
static Chan*
drawattach(char *spec)
{
    dlock();
    if(!initscreenimage()){
        dunlock();
        error("no frame buffer");
    }
    dunlock();

    return devattach('i', spec);
}
/*e: function drawattach */

/*s: function drawwalk */
static Walkqid*
drawwalk(Chan *c, Chan *nc, char **name, int nname)
{
    /*s: [[drawwalk()]] sanity check */
    if(screenimage == nil)
        error("no frame buffer");
    /*e: [[drawwalk()]] sanity check */
    return devwalk(c, nc, name, nname, 0, 0, drawgen);
}
/*e: function drawwalk */

/*s: function drawstat */
static int
drawstat(Chan *c, uchar *db, int n)
{
    return devstat(c, db, n, 0, 0, drawgen);
}
/*e: function drawstat */

/*s: function drawopen */
static Chan*
drawopen(Chan *c, int omode)
{
    /*s: [[drawopen()]] locals */
    Client *cl;
    /*x: [[drawopen()]] locals */
    DImage *di;
    DName *dn;
    /*e: [[drawopen()]] locals */

    if(c->qid.type & QTDIR){
        c = devopen(c, omode, 0, 0, drawgen);
        c->iounit = IOUNIT;
    }

    /*s: [[drawxxx()]] lock */
    dlock();
    if(waserror()){
        dunlock();
        nexterror();
    }
    /*e: [[drawxxx()]] lock */
    /*s: [[drawopen()]] if Qnew */
    if(QID(c->qid) == Qnew){
        cl = drawnewclient();
        /*s: [[drawopen()]] when Qnew, sanity check cl */
        if(cl == nil)
            error(Enodev);
        /*e: [[drawopen()]] when Qnew, sanity check cl */
        c->qid.path = Qctl|((cl->slot+1)<<QSHIFT);
    }
    /*e: [[drawopen()]] if Qnew */
    switch(QID(c->qid)){
    /*s: [[drawopen()]] switch qid cases */
    case Qnew:
        break;
    /*x: [[drawopen()]] switch qid cases */
    case Qdata:
    case Qcolormap:
    case Qrefresh:
        cl = drawclient(c);
        incref(&cl->r);
        break;
    /*x: [[drawopen()]] switch qid cases */
    case Qwinname:
        break;
    /*x: [[drawopen()]] switch qid cases */
    case Qctl:
        cl = drawclient(c);

        /*s: [[drawopen()]] switch qid cases, when Qctl, set busy */
        if(cl->busy)
            error(Einuse);
        cl->busy = true;
        /*e: [[drawopen()]] switch qid cases, when Qctl, set busy */

        flushrect = Rect(10000, 10000, -10000, -10000);
        dn = drawlookupname(strlen(screenname), screenname);
        if(dn == nil)
            error("draw: cannot happen 2");

        if(drawinstall(cl, 0, dn->dimage->image, 0) == 0)
            error(Edrawmem);

        di = drawlookup(cl, 0, false);
        if(di == nil)
            error("draw: cannot happen 1");

        /*s: [[drawopen()]] switch qid cases, when Qctl, set name */
        di->vers = dn->vers;
        di->name = smalloc(strlen(screenname)+1);
        strcpy(di->name, screenname);
        di->fromname = dn->dimage;
        di->fromname->ref++;
        /*e: [[drawopen()]] switch qid cases, when Qctl, set name */

        incref(&cl->r);
        break;
    /*e: [[drawopen()]] switch qid cases */
    }
    /*s: [[drawxxx()]] unlock */
    dunlock();
    poperror();
    /*e: [[drawxxx()]] unlock */

    c->mode = openmode(omode);
    c->flag |= COPEN;
    c->offset = 0;
    c->iounit = IOUNIT;

    return c;
}
/*e: function drawopen */

/*s: function drawclose */
static void
drawclose(Chan *c)
{
    Client *cl;
    /*s: [[drawclose()]] other locals */
    DImage *d, **dp;
    int i;
    /*x: [[drawclose()]] other locals */
    Refresh *r;
    /*e: [[drawclose()]] other locals */

    if(QID(c->qid) < Qctl) /* Qtopdir, Qnew, Q3rd, Q2nd have no client */
        return;

    /*s: [[drawxxx()]] lock */
    dlock();
    if(waserror()){
        dunlock();
        nexterror();
    }
    /*e: [[drawxxx()]] lock */

    cl = drawclient(c);
    /*s: [[drawclose()]] if Qctl */
    if(QID(c->qid) == Qctl)
        cl->busy = false;
    /*e: [[drawclose()]] if Qctl */

    if((c->flag&COPEN) && (decref(&cl->r)==0)){
        /*s: [[drawclose()]] free refresh */
        while(r = cl->refresh){ /* assign = */
            cl->refresh = r->next;
            free(r);
        }
        /*e: [[drawclose()]] free refresh */
        /*s: [[drawclose()]] free names */
        /* free names */
        for(i=0; i<sdraw.nname; )
            if(sdraw.name[i].client == cl)
                drawdelname(sdraw.name+i);
            else
                i++;
        /*e: [[drawclose()]] free names */
        /*s: [[drawclose()]] free screens */
        while(cl->cscreen)
            drawuninstallscreen(cl, cl->cscreen);
        /*e: [[drawclose()]] free screens */
        /* all screens are freed, so now we can free images */
        /*s: [[drawclose()]] free dimages */
        dp = cl->dimage;
        for(i=0; i<NHASH; i++){
            while((d = *dp) != nil){
                *dp = d->next;
                drawfreedimage(d);
            }
            dp++;
        }
        /*e: [[drawclose()]] free dimages */

        sdraw.client[cl->slot] = nil;

        drawflush();    /* to erase visible, now dead windows */
        free(cl);
    }
    /*s: [[drawxxx()]] unlock */
    dunlock();
    poperror();
    /*e: [[drawxxx()]] unlock */
}
/*e: function drawclose */

/*s: function drawread */
long
drawread(Chan *c, void *a, long n, vlong off)
{
    Client *cl;
    /*s: [[drawread()]] other locals */
    Memimage *i;
    char buf[16];
    /*x: [[drawread()]] other locals */
    DImage *di;
    /*x: [[drawread()]] other locals */
    uchar *p;
    int index, m;
    ulong red, green, blue;
    ulong offset = off;
    /*x: [[drawread()]] other locals */
    Refresh *r;
    /*e: [[drawread()]] other locals */

    if(c->qid.type & QTDIR)
        return devdirread(c, a, n, 0, 0, drawgen);
    /*s: [[drawread()]] if Qwinname */
    if(QID(c->qid) == Qwinname)
        return readstr(off, a, n, screenname);
    /*e: [[drawread()]] if Qwinname */
    // else

    cl = drawclient(c);

    /*s: [[drawxxx()]] lock */
    dlock();
    if(waserror()){
        dunlock();
        nexterror();
    }
    /*e: [[drawxxx()]] lock */
    switch(QID(c->qid)){
    /*s: [[drawread()]] switch qid cases */
    case Qctl:
        if(n < 12*12) // NINFO
            error(Eshortread);

        /*s: [[drawread()]] switch qid cases, when Qctl, set i */
        /*s: [[drawread()]] switch qid cases, when Qctl, sanity check infoid */
        if(cl->infoid < 0)
            error(Enodrawimage);
        /*e: [[drawread()]] switch qid cases, when Qctl, sanity check infoid */
        if(cl->infoid == 0){
            i = screenimage;
            /*s: [[drawread()]] switch qid cases, when Qctl, sanity check i */
            if(i == nil)
                error(Enodrawimage);
            /*e: [[drawread()]] switch qid cases, when Qctl, sanity check i */
        }else{
            di = drawlookup(cl, cl->infoid, true);
            /*s: [[drawread()]] switch qid cases, when Qctl, sanity check di */
            if(di == nil)
                error(Enodrawimage);
            /*e: [[drawread()]] switch qid cases, when Qctl, sanity check di */
            i = di->image;
        }
        /*e: [[drawread()]] switch qid cases, when Qctl, set i */

        // mostly for initdisplay!
        n = snprint(a, n,
            "%11d %11d %11s %11d %11d %11d %11d %11d %11d %11d %11d %11d ",
            cl->clientid, cl->infoid, chantostr(buf, i->chan),
            (i->flags&Frepl)==Frepl,
            i->r.min.x, i->r.min.y, i->r.max.x, i->r.max.y,
            i->clipr.min.x, i->clipr.min.y, i->clipr.max.x, i->clipr.max.y
            );
        cl->infoid = -1;
        break;
    /*x: [[drawread()]] switch qid cases */
    case Qdata:
        /*s: [[drawread()]] switch qid cases, when Qdata, sanity checks */
        if(cl->readdata == nil)
            error("no draw data");
        if(n < cl->nreaddata)
            error(Eshortread);
        /*e: [[drawread()]] switch qid cases, when Qdata, sanity checks */
        n = cl->nreaddata;
        memmove(a, cl->readdata, cl->nreaddata);
        free(cl->readdata);
        cl->readdata = nil;
        break;
    /*x: [[drawread()]] switch qid cases */
    case Qcolormap:
        drawactive(true);  /* to restore map from backup */
        p = malloc(4*12*256+1);
        if(p == nil)
            error(Enomem);
        m = 0;
        for(index = 0; index < 256; index++){
            arch_getcolor(index, &red, &green, &blue);
            m += snprint((char*)p+m, 4*12*256+1 - m,
                "%11d %11lud %11lud %11lud\n", index,
                red>>24, green>>24, blue>>24);
        }
        n = readstr(offset, a, n, (char*)p);
        free(p);
        break;
    /*x: [[drawread()]] switch qid cases */
    case Qrefresh:
        if(n < 5*4)
            error(Ebadarg);
        for(;;){
            if(cl->refreshme || cl->refresh)
                break;

            dunlock();
            if(waserror()){
                dlock();    /* restore lock for waserror() above */
                nexterror();
            }

            sleep(&cl->refrend, drawrefactive, cl);

            poperror();
            dlock();
        }
        p = a;
        while(cl->refresh && n>=5*4){
            r = cl->refresh;
            BPLONG(p+0*4, r->dimage->id);
            BPLONG(p+1*4, r->r.min.x);
            BPLONG(p+2*4, r->r.min.y);
            BPLONG(p+3*4, r->r.max.x);
            BPLONG(p+4*4, r->r.max.y);
            cl->refresh = r->next;
            free(r);
            p += 5*4;
            n -= 5*4;
        }
        cl->refreshme = 0;
        n = p-(uchar*)a;
        break;
    /*e: [[drawread()]] switch qid cases */
    }
    /*s: [[drawxxx()]] unlock */
    dunlock();
    poperror();
    /*e: [[drawxxx()]] unlock */

    return n;
}
/*e: function drawread */

/*s: function drawwakeall */
void
drawwakeall(void)
{
    Client *cl;
    int i;

    for(i=0; i<sdraw.nclient; i++){
        cl = sdraw.client[i];
        if(cl && (cl->refreshme || cl->refresh))
            wakeup(&cl->refrend);
    }
}
/*e: function drawwakeall */

/*s: function drawwrite */
static long
drawwrite(Chan *c, void *a, long n, vlong)
{
    Client *cl;
    /*s: [[drawwrite()]] other locals */
    char buf[128], *fields[4], *q;
    int i, m, red, green, blue, x;
    /*e: [[drawwrite()]] other locals */

    /*s: [[drawwrite()]] sanity check c */
    if(c->qid.type & QTDIR)
        error(Eisdir);
    /*e: [[drawwrite()]] sanity check c */
    cl = drawclient(c);

    /*s: [[drawwrite()]] lock */
    dlock();
    if(waserror()){

        drawwakeall();

        dunlock();
        nexterror();
    }
    /*e: [[drawwrite()]] lock */
    switch(QID(c->qid)){
    /*s: [[drawwrite()]] switch qid cases */
    case Qctl:
        if(n != 4)
            error("unknown draw control request");
        cl->infoid = BGLONG((uchar*)a);
        break;
    /*x: [[drawwrite()]] switch qid cases */
    case Qdata:
        // The big dispatch!!
        drawmesg(cl, a, n);

        drawwakeall();
        break;
    /*x: [[drawwrite()]] switch qid cases */
    case Qcolormap:
        drawactive(true);  /* to restore map from backup */
        m = n;
        n = 0;
        while(m > 0){
            x = m;
            if(x > sizeof(buf)-1)
                x = sizeof(buf)-1;
            q = memccpy(buf, a, '\n', x);
            if(q == 0)
                break;
            i = q-buf;
            n += i;
            a = (char*)a + i;
            m -= i;
            *q = 0;
            if(tokenize(buf, fields, nelem(fields)) != 4)
                error(Ebadarg);
            i = strtoul(fields[0], 0, 0);
            red = strtoul(fields[1], 0, 0);
            green = strtoul(fields[2], 0, 0);
            blue = strtoul(fields[3], &q, 0);
            if(fields[3] == q)
                error(Ebadarg);
            if(red>255 || green>255 || blue>255 || i<0 || i>255)
                error(Ebadarg);
            red |= red<<8;
            red |= red<<16;
            green |= green<<8;
            green |= green<<16;
            blue |= blue<<8;
            blue |= blue<<16;
            arch_setcolor(i, red, green, blue);
        }
        break;
    /*e: [[drawwrite()]] switch qid cases */
    default:
        error(Ebadusefd);
    }
    /*s: [[drawxxx()]] unlock */
    dunlock();
    poperror();
    /*e: [[drawxxx()]] unlock */

    return n;
}
/*e: function drawwrite */


/*s: global drawdevtab */
Dev drawdevtab = {
    .dc       =    'i',
    .name     =    "draw",

    .attach   =    drawattach,
    .walk     =    drawwalk,
    .stat     =    drawstat,

    .open     =    drawopen,
    .close    =    drawclose,
    .read     =    drawread,
    .write    =    drawwrite,

    // generic               
    .create   =    devcreate,
    .remove   =    devremove,
    .bread    =    devbread,
    .bwrite   =    devbwrite,
    .wstat    =    devwstat,
    .reset    =    devreset,
    .init     =    devinit,
    .shutdown =    devshutdown,
};
/*e: global drawdevtab */

/*e: kernel/devices/screen/devdraw.c */
