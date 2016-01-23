/*s: kernel/devices/screen/devdraw.c */
#include    "u.h"
#include    "../port/lib.h"
#include    "../port/error.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"

#include    <draw.h>
#include    <memdraw.h>
#include    <memlayer.h>
#include    <cursor.h>

#include    "screen.h"

/*s: enum _anon_ (kernel/devices/screen/devdraw.c) */
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
    Qctl, // used in x < Qctl so must be the first!
    Qdata, // all the operations, drawmesg()

    Qcolormap, 
    Qrefresh,
};
/*e: enum _anon_ (kernel/devices/screen/devdraw.c) */

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
/*s: constant NHASH bis */
#define NHASH       (1<<5)
/*e: constant NHASH bis */
/*s: constant HASHMASK */
#define HASHMASK    (NHASH-1)
/*e: constant HASHMASK */
/*s: constant IOUNIT */
#define IOUNIT      (64*1024)
/*e: constant IOUNIT */

typedef struct Client Client;
typedef struct KDraw KDraw;
typedef struct DImage DImage;
typedef struct DScreen DScreen;
typedef struct CScreen CScreen;
typedef struct FChar FChar;
typedef struct Refresh Refresh;
typedef struct Refx Refx;
typedef struct DName DName;

/*s: global blanktime */
ulong blanktime = 30;   /* in minutes; a half hour */
/*e: global blanktime */

/*s: struct KDraw */
struct KDraw
{
    // growing_array<option<Client>>, size in KDraw.nclient
    Client**    client;
    int     nclient;

    // gensym
    int     clientid;
    
    /*s: [[KDraw]] other fields */
    bool     softscreen;
    /*x: [[KDraw]] other fields */
    DName*  name;
    int     nname;
    /*x: [[KDraw]] other fields */
    int     vers;
    /*x: [[KDraw]] other fields */
    bool	blanked;    /* screen turned off */
    /*x: [[KDraw]] other fields */
    ulong   savemap[3*256];
    /*x: [[KDraw]] other fields */
    ulong   blanktime;  /* time of last operation */
    /*e: [[KDraw]] other fields */
};
/*e: struct KDraw */

/*s: struct Client */
struct Client
{
    int     clientid; // /dev/draw/x/

    // hash<Image.id, ref_own<Dimage>> (next = Dimage.next in bucket)
    DImage*     dimage[NHASH];

    /*s: [[Client]] drawing state fields */
    // enum<Drawop>
    int     op;
    /*e: [[Client]] drawing state fields */
    /*s: [[Client]] layer fields */
    CScreen*    cscreen;
    /*e: [[Client]] layer fields */
    /*s: [[Client]] other fields */
    // index in KDraw.clients[]
    int     slot;
    /*x: [[Client]] other fields */
    int     infoid;
    /*x: [[Client]] other fields */
    byte*   readdata;
    int     nreaddata;
    /*x: [[Client]] other fields */
    Refresh*    refresh;
    Rendez      refrend;
    int     refreshme;
    /*e: [[Client]] other fields */

    // Extra
    /*s: [[Client]] concurrency fields */
    bool     busy;
    /*e: [[Client]] concurrency fields */
    Ref     r;
};
/*e: struct Client */

/*s: struct Refresh */
struct Refresh
{
    DImage*     dimage;
    Rectangle   r;
    Refresh*    next;
};
/*e: struct Refresh */

/*s: struct Refx */
struct Refx
{
    Client*     client;
    DImage*     dimage;
};
/*e: struct Refx */

/*s: struct DName */
struct DName
{
    char        *name;
    Client      *client;
    DImage*     dimage;
    /*s: [[DName]] other fields */
    int     vers;
    /*e: [[DName]] other fields */
};
/*e: struct DName */

/*s: struct FChar */
struct FChar
{
    int     minx;   /* left edge of bits */
    int     maxx;   /* right edge of bits */
    uchar       miny;   /* first non-zero scan-line */
    uchar       maxy;   /* last non-zero scan-line + 1 */
    schar       left;   /* offset of baseline */
    uchar       width;  /* width of baseline */
};
/*e: struct FChar */

/*s: struct DImage */
/*
 * Reference counts in DImages:
 *  one per open by original client
 *  one per screen image or fill
 *  one per image derived from this one by name
 */
struct DImage
{
    int     id; // same than Image.id?
    Memimage*   image;

    /*s: [[Dimage]] layer fields */
    DScreen*    dscreen;    /* 0 if not a window */
    /*e: [[Dimage]] layer fields */
    /*s: [[Dimage]] other fields */
    int     ascent;
    int     nfchar;
    FChar*      fchar;

    /*x: [[Dimage]] other fields */
    char    *name;
    DImage*     fromname;   /* image this one is derived from, by name */
    /*x: [[Dimage]] other fields */
    int     vers;
    /*e: [[Dimage]] other fields */

    // Extra
    /*s: [[Dimage]] extra fields */
    DImage*     next;
    /*e: [[Dimage]] extra fields */
    int     ref;
};
/*e: struct DImage */

/*s: struct CScreen */
struct CScreen
{
    DScreen*    dscreen;
    CScreen*    next;
};
/*e: struct CScreen */

/*s: struct DScreen */
struct DScreen
{
    int     id;

    int     public;
    int     ref;

    DImage      *dimage;
    DImage      *dfill;

    Memscreen*  screen;

    Client*     owner;
    DScreen*    next;
};
/*e: struct DScreen */

/*s: global sdraw */
static  KDraw        sdraw;
/*e: global sdraw */
/*s: global drawlock */
QLock   drawlock;
/*e: global drawlock */

/*s: global screenimage */
static  Memimage    *screenimage;
/*e: global screenimage */
/*s: global screendimage */
static  DImage* screendimage;
/*e: global screendimage */
/*s: global screenname */
static  char    screenname[40];
/*e: global screenname */
/*s: global screennameid */
// gensym
static  int screennameid;
/*e: global screennameid */

/*s: global flushrect */
static  Rectangle   flushrect;
/*e: global flushrect */
/*s: global waste */
static  int     waste;
/*e: global waste */
/*s: global dscreen */
static  DScreen*    dscreen;
/*e: global dscreen */

// forward decls
void        drawmesg(Client*, void*, int);
void        drawuninstall(Client*, int);
void        drawfreedimage(DImage*);
Client*     drawclientofpath(ulong);
DImage* 	allocdimage(Memimage*);

/*s: global Enodrawimage */
static  char Enodrawimage[] =   "unknown id for draw image";
/*e: global Enodrawimage */
/*s: global Enodrawscreen */
static  char Enodrawscreen[] =  "unknown id for draw screen";
/*e: global Enodrawscreen */
/*s: global Eshortdraw */
static  char Eshortdraw[] = "short draw message";
/*e: global Eshortdraw */
/*s: global Eshortread */
static  char Eshortread[] = "draw read too short";
/*e: global Eshortread */
/*s: global Eimageexists */
static  char Eimageexists[] =   "image id in use";
/*e: global Eimageexists */
/*s: global Escreenexists */
static  char Escreenexists[] =  "screen id in use";
/*e: global Escreenexists */
/*s: global Edrawmem */
static  char Edrawmem[] =   "image memory allocation failed";
/*e: global Edrawmem */
/*s: global Ereadoutside */
static  char Ereadoutside[] =   "readimage outside image";
/*e: global Ereadoutside */
/*s: global Ewriteoutside */
static  char Ewriteoutside[] =  "writeimage outside image";
/*e: global Ewriteoutside */
/*s: global Enotfont */
static  char Enotfont[] =   "image not a font";
/*e: global Enotfont */
/*s: global Eindex */
static  char Eindex[] =     "character index out of range";
/*e: global Eindex */
/*s: global Enoclient */
static  char Enoclient[] =  "no such draw client";
/*e: global Enoclient */
/*s: global Enameused */
//static    char Edepth[] =     "image has bad depth";
static  char Enameused[] =  "image name in use";
/*e: global Enameused */
/*s: global Enoname */
static  char Enoname[] =    "no image with that name";
/*e: global Enoname */
/*s: global Eoldname */
static  char Eoldname[] =   "named image no longer valid";
/*e: global Eoldname */
/*s: global Enamed */
static  char Enamed[] =     "image already has name";
/*e: global Enamed */
/*s: global Ewrongname */
static  char Ewrongname[] =     "wrong name for image";
/*e: global Ewrongname */

/*s: function dlock */
static void
dlock(void)
{
    qlock(&drawlock);
}
/*e: function dlock */

/*s: function candlock */
static int
candlock(void)
{
    return canqlock(&drawlock);
}
/*e: function candlock */

/*s: function dunlock */
static void
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

    if(s == DEVDOTDOT){
        switch(QID(c->qid)){
        case Qtopdir:
        case Q2nd:
            mkqid(&q, Qtopdir, 0, QTDIR);
            devdir(c, q, "#i", 0, eve, 0500, dp);
            break;
        case Q3rd:
            cl = drawclientofpath(c->qid.path);
            if(cl == nil)
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
    // else

    /*
     * Top level directory contains the name of the device.
     */
    t = QID(c->qid);
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

    /*
     * Second level contains "new" plus all the clients.
     */
    if(t == Q2nd || t == Qnew){
        if(s == 0){
            mkqid(&q, Qnew, 0, QTFILE);
            devdir(c, q, "new", 0, eve, 0666, dp);
        }
        else if(s <= sdraw.nclient){
            cl = sdraw.client[s-1];
            if(cl == nil)
                return 0;
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

    /*
     * Third level.
     */
    path = c->qid.path&~((1<<QSHIFT)-1);    /* slot component */
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
}
/*e: function drawgen */

/*s: function drawrefactive */
static
int
drawrefactive(void *a)
{
    Client *c;

    c = a;
    return c->refreshme || c->refresh != nil;
}
/*e: function drawrefactive */

/*s: function drawrefreshscreen */
static
void
drawrefreshscreen(DImage *l, Client *client)
{
    while(l != nil && l->dscreen == nil)
        l = l->fromname;
    if(l != nil && l->dscreen->owner != client)
        l->dscreen->owner->refreshme = 1;
}
/*e: function drawrefreshscreen */

/*s: function drawrefresh */
static
void
drawrefresh(Memimage*, Rectangle r, void *v)
{
    Refx *x;
    DImage *d;
    Client *c;
    Refresh *ref;

    if(v == 0)
        return;
    x = v;
    c = x->client;
    d = x->dimage;
    for(ref=c->refresh; ref; ref=ref->next)
        if(ref->dimage == d){
            combinerect(&ref->r, r);
            return;
        }
    ref = malloc(sizeof(Refresh));
    if(ref){
        ref->dimage = d;
        ref->r = r;
        ref->next = c->refresh;
        c->refresh = ref;
    }
}
/*e: function drawrefresh */

/*s: function addflush */
static void
addflush(Rectangle r)
{
    int abb, ar, anbb;
    Rectangle nbb;

    if(sdraw.softscreen==false || !rectclip(&r, screenimage->r))
        return;

    if(flushrect.min.x >= flushrect.max.x){
        flushrect = r;
        waste = 0;
        return;
    }
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
        flushmemscreen(flushrect);
    flushrect = r;
    waste = 0;
}
/*e: function addflush */

/*s: function dstflush */
static
void
dstflush(int dstid, Memimage *dst, Rectangle r)
{
    Memlayer *l;

    if(dstid == 0){
        combinerect(&flushrect, r);
        return;
    }
    /* how can this happen? -rsc, dec 12 2002 */
    if(dst == nil){
        print("nil dstflush\n");
        return;
    }

    /*s: [[dstflush()]] if layer */
    l = dst->layer;
    if(l == nil)
        return;
    do{
        if(l->screen->image->data != screenimage->data)
            return;
        r = rectaddpt(r, l->delta);
        l = l->screen->image->layer;
    }while(l);
    /*e: [[dstflush()]] if layer */

    addflush(r);
}
/*e: function dstflush */

/*s: function drawflush */
void
drawflush(void)
{
    if(flushrect.min.x < flushrect.max.x)
        flushmemscreen(flushrect);
    flushrect = Rect(10000, 10000, -10000, -10000);
}
/*e: function drawflush */

/*s: function drawcmp */
static
int
drawcmp(char *a, char *b, int n)
{
    if(strlen(a) != n)
        return 1;
    return memcmp(a, b, n);
}
/*e: function drawcmp */

/*s: function drawlookupname */
DName*
drawlookupname(int n, char *str)
{
    DName *name, *ename;

    name = sdraw.name;
    ename = &name[sdraw.nname];
    for(; name<ename; name++)
        if(drawcmp(name->name, str, n) == 0)
            return name;
    return nil;
}
/*e: function drawlookupname */

/*s: function drawgoodname */
bool
drawgoodname(DImage *d)
{
    DName *n;

    /* if window, validate the screen's own images */
    if(d->dscreen)
        if(drawgoodname(d->dscreen->dimage) == false
        || drawgoodname(d->dscreen->dfill) == false)
            return false;
    if(d->name == nil)
        return true;
    n = drawlookupname(strlen(d->name), d->name);
    if(n==nil || n->vers!=d->vers)
        return false;
    return true;
}
/*e: function drawgoodname */

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

/*s: function drawlookupdscreen */
DScreen*
drawlookupdscreen(int id)
{
    DScreen *s;

    s = dscreen;
    while(s){
        if(s->id == id)
            return s;
        s = s->next;
    }
    return 0;
}
/*e: function drawlookupdscreen */

/*s: function drawlookupscreen */
DScreen*
drawlookupscreen(Client *client, int id, CScreen **cs)
{
    CScreen *s;

    s = client->cscreen;
    while(s){
        if(s->dscreen->id == id){
            *cs = s;
            return s->dscreen;
        }
        s = s->next;
    }
    error(Enodrawscreen);
    return 0;
}
/*e: function drawlookupscreen */

/*s: function allocdimage */
/// makescreenimage | drawinstall -> <>
DImage*
allocdimage(Memimage *i)
{
    DImage *d;

    d = malloc(sizeof(DImage));
    if(d == nil)
        return nil;
    d->image = i;
    d->ref = 1;

    d->name = nil;
    d->fromname = nil;
    d->vers = 0;
    d->nfchar = 0;
    d->fchar = 0;

    return d;
}
/*e: function allocdimage */

/*s: function drawinstall */
Memimage*
drawinstall(Client *client, int id, Memimage *i, DScreen *dscreen)
{
    DImage *d;

    d = allocdimage(i);
    if(d == nil)
        return nil;
    d->id = id;

    /*s: [[drawinstall()]] install dscreen */
    d->dscreen = dscreen;
    /*e: [[drawinstall()]] install dscreen */

    // insert_hash(d, client->dimage)
    d->next = client->dimage[id&HASHMASK];
    client->dimage[id&HASHMASK] = d;

    return i;
}
/*e: function drawinstall */

/*s: function drawinstallscreen */
Memscreen*
drawinstallscreen(Client *client, DScreen *d, int id, DImage *dimage, DImage *dfill, int public)
{
    Memscreen *s;
    CScreen *c;

    c = malloc(sizeof(CScreen));
    if(dimage && dimage->image && dimage->image->chan == 0)
        panic("bad image %p in drawinstallscreen", dimage->image);

    if(c == 0)
        return 0;
    if(d == 0){
        d = malloc(sizeof(DScreen));
        if(d == 0){
            free(c);
            return 0;
        }
        s = malloc(sizeof(Memscreen));
        if(s == 0){
            free(c);
            free(d);
            return 0;
        }
        s->frontmost = 0;
        s->rearmost = 0;
        d->dimage = dimage;
        if(dimage){
            s->image = dimage->image;
            dimage->ref++;
        }
        d->dfill = dfill;
        if(dfill){
            s->fill = dfill->image;
            dfill->ref++;
        }
        d->ref = 0;
        d->id = id;
        d->screen = s;
        d->public = public;
        d->next = dscreen;
        d->owner = client;
        dscreen = d;
    }
    c->dscreen = d;
    d->ref++;
    c->next = client->cscreen;
    client->cscreen = c;
    return d->screen;
}
/*e: function drawinstallscreen */

/*s: function drawdelname */
void
drawdelname(DName *name)
{
    int i;

    i =  name - sdraw.name;
    memmove(name, name+1, (sdraw.nname-(i+1))*sizeof(DName));
    sdraw.nname--;
}
/*e: function drawdelname */

/*s: function drawfreedscreen */
void
drawfreedscreen(DScreen *this)
{
    DScreen *ds, *next;

    this->ref--;
    if(this->ref < 0)
        print("negative ref in drawfreedscreen\n");
    if(this->ref > 0)
        return;
    ds = dscreen;
    if(ds == this){
        dscreen = this->next;
        goto Found;
    }
    while(next = ds->next){ /* assign = */
        if(next == this){
            ds->next = this->next;
            goto Found;
        }
        ds = next;
    }
    error(Enodrawimage);

    Found:
    if(this->dimage)
        drawfreedimage(this->dimage);
    if(this->dfill)
        drawfreedimage(this->dfill);
    free(this->screen);
    free(this);
}
/*e: function drawfreedscreen */

/*s: function drawfreedimage */
void
drawfreedimage(DImage *dimage)
{
    int i;
    Memimage *l;
    DScreen *ds;

    dimage->ref--;
    if(dimage->ref < 0)
        print("negative ref in drawfreedimage\n");
    if(dimage->ref > 0)
        return;

    /*s: [[drawfreedimage()]] free names */
    /* any names? */
    for(i=0; i<sdraw.nname; )
        if(sdraw.name[i].dimage == dimage)
            drawdelname(sdraw.name+i);
        else
            i++;

    if(dimage->fromname){   /* acquired by name; owned by someone else*/
        drawfreedimage(dimage->fromname);
        goto Return;
    }
    /*e: [[drawfreedimage()]] free names */

    /*s: [[drawfreedimage()]] if dscreen */
    ds = dimage->dscreen;
    if(ds){
        l = dimage->image;
        if(l->data == screenimage->data)
            addflush(l->layer->screenr);
        if(l->layer->refreshfn == drawrefresh)  /* else true owner will clean up */
            free(l->layer->refreshptr);
        l->layer->refreshptr = nil;
        if(drawgoodname(dimage))
            memldelete(l);
        else
            memlfree(l);
        drawfreedscreen(ds);
    }
    /*e: [[drawfreedimage()]] if dscreen */
    else
        freememimage(dimage->image);
    Return:
    free(dimage->fchar);
    free(dimage);
}
/*e: function drawfreedimage */

/*s: function drawuninstallscreen */
void
drawuninstallscreen(Client *client, CScreen *this)
{
    CScreen *cs, *next;

    cs = client->cscreen;
    if(cs == this){
        client->cscreen = this->next;
        drawfreedscreen(this->dscreen);
        free(this);
        return;
    }
    while(next = cs->next){ /* assign = */
        if(next == this){
            cs->next = this->next;
            drawfreedscreen(this->dscreen);
            free(this);
            return;
        }
        cs = next;
    }
}
/*e: function drawuninstallscreen */

/*s: function drawuninstall */
void
drawuninstall(Client *client, int id)
{
    DImage *d, *next;

    d = client->dimage[id&HASHMASK];
    if(d == nil)
        error(Enodrawimage);
    if(d->id == id){
        client->dimage[id&HASHMASK] = d->next;
        drawfreedimage(d);
        return;
    }
    while(next = d->next){  /* assign = */
        if(next->id == id){
            d->next = next->next;
            drawfreedimage(next);
            return;
        }
        d = next;
    }
    error(Enodrawimage);
}
/*e: function drawuninstall */

/*s: function drawaddname */
void
drawaddname(Client *client, DImage *di, int n, char *str)
{
    DName *name, *ename, *new, *t;

    name = sdraw.name;
    ename = &name[sdraw.nname];
    for(; name<ename; name++)
        if(drawcmp(name->name, str, n) == 0)
            error(Enameused);

    t = smalloc((sdraw.nname+1)*sizeof(DName));
    memmove(t, sdraw.name, sdraw.nname*sizeof(DName));
    free(sdraw.name);
    sdraw.name = t;
    new = &sdraw.name[sdraw.nname++];
    new->name = smalloc(n+1);
    memmove(new->name, str, n);
    new->name[n] = '\0';
    new->dimage = di;
    new->client = client;
    new->vers = ++sdraw.vers;
}
/*e: function drawaddname */

/*s: function drawnewclient */
Client*
drawnewclient(void)
{
    Client *cl, **cp;
    int i;

    // find free slot
    for(i=0; i<sdraw.nclient; i++){
        cl = sdraw.client[i];
        if(cl == nil)
            break;
    }

    /*s: [[drawnewclient()]] grow array if necessary */
    // growing array
    if(i == sdraw.nclient){
        cp = malloc((sdraw.nclient+1)*sizeof(Client*));
        if(cp == nil)
            return nil;
        memmove(cp, sdraw.client, sdraw.nclient*sizeof(Client*));
        free(sdraw.client);
        sdraw.client = cp;
        sdraw.nclient++;
        cp[i] = nil;
    }
    /*e: [[drawnewclient()]] grow array if necessary */

    cl = malloc(sizeof(Client));
    if(cl == nil)
        return nil;
    memset(cl, 0, sizeof(Client));
    cl->slot = i;
    cl->clientid = ++sdraw.clientid;
    cl->op = SoverD; // The classic
    sdraw.client[i] = cl;
    return cl;
}
/*e: function drawnewclient */

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
    if(cl==nil || cl->clientid==0)
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

/*s: function drawimage */
Memimage*
drawimage(Client *client, uchar *a)
{
    DImage *d;

    d = drawlookup(client, BGLONG(a), true);
    if(d == nil)
        error(Enodrawimage);
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

/*s: function drawchar */
Point
drawchar(Memimage *dst, Memimage *rdst, Point p, Memimage *src, Point *sp, DImage *font, int index, int op)
{
    FChar *fc;
    Rectangle r;
    Point sp1;
    static Memimage *tmp;

    fc = &font->fchar[index];
    r.min.x = p.x+fc->left;
    r.min.y = p.y-(font->ascent-fc->miny);
    r.max.x = r.min.x+(fc->maxx-fc->minx);
    r.max.y = r.min.y+(fc->maxy-fc->miny);
    sp1.x = sp->x+fc->left;
    sp1.y = sp->y+fc->miny;

    /*
     * If we're drawing greyscale fonts onto a VGA screen,
     * it's very costly to read the screen memory to do the
     * alpha blending inside memdraw.  If this is really a stringbg,
     * then rdst is the bg image (in main memory) which we can
     * refer to for the underlying dst pixels instead of reading dst
     * directly.
     */
    if(ishwimage(dst) && !ishwimage(rdst) && font->image->depth > 1){
        if(tmp == nil || tmp->chan != dst->chan || Dx(tmp->r) < Dx(r) || Dy(tmp->r) < Dy(r)){
            if(tmp)
                freememimage(tmp);
            tmp = allocmemimage(Rect(0,0,Dx(r),Dy(r)), dst->chan);
            if(tmp == nil)
                goto fallback;
        }
        memdraw(tmp, Rect(0,0,Dx(r),Dy(r)), rdst, r.min, memopaque, ZP, S);
        memdraw(tmp, Rect(0,0,Dx(r),Dy(r)), src, sp1, font->image, Pt(fc->minx, fc->miny), op);
        memdraw(dst, r, tmp, ZP, memopaque, ZP, S);
    }else{
    fallback:
        memdraw(dst, r, src, sp1, font->image, Pt(fc->minx, fc->miny), op);
    }

    p.x += fc->width;
    sp->x += fc->width;
    return p;
}
/*e: function drawchar */

/*s: function makescreenimage */
static DImage*
makescreenimage(void)
{
    /*s: [[makescreenimage()]] locals */
    Memdata *md;
    Memimage *i;
    DImage *di;
    /*x: [[makescreenimage()]] locals */
    int width, depth;
    ulong chan;
    Rectangle r;
    /*e: [[makescreenimage()]] locals */

    /*s: [[makescreenimage()]] allocate Memdata md */
    // allocate Memdata
    md = malloc(sizeof(Memdata));
    if(md == nil)
        return nil;
    md->allocd = true;

    md->bdata = attachscreen(&r, &chan, &depth, &width, &sdraw.softscreen);
    if(md->bdata == nil){
        free(md);
        return nil;
    }
    md->base = nil; // not allocated by poolalloc
    md->ref = 1;
    /*e: [[makescreenimage()]] allocate Memdata md */
    /*s: [[makescreenimage()]] allocate Memimage i */
    // allocate Memimage
    i = allocmemimaged(r, chan, md);
    if(i == nil){
        free(md);
        return nil;
    }
    /*e: [[makescreenimage()]] allocate Memimage i */
    /*s: [[makescreenimage()]] allocate DImage di */
    // allocate DImage
    di = allocdimage(i);
    if(di == nil){
        freememimage(i);    /* frees md */
        return nil;
    }
    /*e: [[makescreenimage()]] allocate DImage di */

    /*s: [[makescreenimage()]] drawaddname */
    if(!waserror()){
        snprint(screenname, sizeof screenname, "noborder.screen.%d", ++screennameid);
        drawaddname(nil, di, strlen(screenname), screenname);
        poperror();
    }
    /*e: [[makescreenimage()]] drawaddname */

    return di;
}
/*e: function makescreenimage */

/*s: function initscreenimage */
static error0
initscreenimage(void)
{
    if(screenimage != nil)
        return OK_1;

    screendimage = makescreenimage();
    if(screendimage == nil)
        return ERROR_0;
    screenimage = screendimage->image;

    //DBG1("initscreenimage %p %p\n", screendimage, screenimage);
    mouseresize();
    return OK_1;
}
/*e: function initscreenimage */

/*s: function deletescreenimage */
void
deletescreenimage(void)
{
    dlock();
    if(screenimage){
        /* will be freed via screendimage; disable */
        screenimage->clipr = ZR;
        screenimage = nil;
    }
    if(screendimage){
        drawfreedimage(screendimage);
        screendimage = nil;
    }
    dunlock();
}
/*e: function deletescreenimage */

/*s: function resetscreenimage */
void
resetscreenimage(void)
{
    dlock();
    initscreenimage();
    dunlock();
}
/*e: function resetscreenimage */

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

    dlock();
    if(waserror()){
        dunlock();
        nexterror();
    }

    /*s: [[drawopen()]] if Qnew */
    if(QID(c->qid) == Qnew){
        cl = drawnewclient();
        if(cl == nil)
            error(Enodev);
        c->qid.path = Qctl|((cl->slot+1)<<QSHIFT); // >>
    }
    /*e: [[drawopen()]] if Qnew */
    switch(QID(c->qid)){
    /*s: [[drawopen()]] switch qid cases */
    case Qnew:
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
    /*e: [[drawopen()]] switch qid cases */
    }

    dunlock();
    poperror();

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

    dlock();
    if(waserror()){
        dunlock();
        nexterror();
    }

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
        /*s: [[drawclose()]] free dimages */
        /* all screens are freed, so now we can free images */
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
    dunlock();
    poperror();
}
/*e: function drawclose */

/*s: function drawread */
long
drawread(Chan *c, void *a, long n, vlong off)
{
    Client *cl;
    /*s: [[drawread()]] other locals */
    DImage *di;
    Memimage *i;
    char buf[16];
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
    dlock();
    if(waserror()){
        dunlock();
        nexterror();
    }

    switch(QID(c->qid)){
    /*s: [[drawread()]] switch qid cases */
    case Qctl:
        if(n < 12*12)
            error(Eshortread);

        if(cl->infoid < 0)
            error(Enodrawimage);
        if(cl->infoid == 0){
            i = screenimage;
            if(i == nil)
                error(Enodrawimage);
        }else{
            di = drawlookup(cl, cl->infoid, true);
            if(di == nil)
                error(Enodrawimage);
            i = di->image;
        }

        // for initdisplay!
        n = snprint(a, n,
            "%11d %11d %11s %11d %11d %11d %11d %11d %11d %11d %11d %11d ",
            cl->clientid, cl->infoid, chantostr(buf, i->chan),
            (i->flags&Frepl)==Frepl,
            i->r.min.x, i->r.min.y, i->r.max.x, i->r.max.y,
            i->clipr.min.x, i->clipr.min.y, i->clipr.max.x,
            i->clipr.max.y);
        cl->infoid = -1;
        break;
    /*x: [[drawread()]] switch qid cases */
    case Qdata:
        if(cl->readdata == nil)
            error("no draw data");
        if(n < cl->nreaddata)
            error(Eshortread);
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
            getcolor(index, &red, &green, &blue);
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
    dunlock();
    poperror();
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

    if(c->qid.type & QTDIR)
        error(Eisdir);

    cl = drawclient(c);

    dlock();
    if(waserror()){

        drawwakeall();

        dunlock();
        nexterror();
    }

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
            setcolor(i, red, green, blue);
        }
        break;
    /*e: [[drawwrite()]] switch qid cases */
    default:
        error(Ebadusefd);
    }
    dunlock();
    poperror();
    return n;
}
/*e: function drawwrite */

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

/*s: function printmesg */
static void
printmesg(char *fmt, uchar *a, bool plsprnt)
{
    char buf[256];
    char *p, *q;
    int s, left;

    if(1|| plsprnt==false){
        SET(s,q,p);
        USED(fmt, a, buf, p, q, s);
        return;
    }

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
    *q = 0;
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
    int c, repl, y, dstid, scrnid, ni, ci, j, nw, e0, e1, op, ox, oy, oesize, esize, doflush;
    byte *u, refresh;
    ulong value, chan;
    Rectangle r, clipr;
    Point p, q, *pp, sp;
    Memimage *i, *bg, *dst, *src, *mask;
    Memimage *l, **lp;
    Memscreen *scrn;
    DImage *font, *ll, *di, *ddst, *dsrc;
    DName *dn;
    DScreen *dscrn;
    FChar *fc;
    CScreen *cs;
    Refreshfn reffn;
    Refx *refx;
    /*e: [[drawmesg()]] locals */

    a = av;
    if(waserror()){
        if(fmt) 
            printmesg(fmt, a, 1);
    /*  iprint("error: %s\n", up->errstr);  */
        nexterror();
    }
    while((n-=m) > 0){
        USED(fmt);
        a += m;
        switch(*a){
        /*s: [[drawmesg()]] cases */
        /* new allocate: 'b' id[4] screenid[4] refresh[1] chan[4] repl[1] R[4*4] clipR[4*4] rrggbbaa[4] */
        case 'b':
            printmesg(fmt="LLbLbRRL", a, 0);
            m = 1+4+4+1+4+1+4*4+4*4+4;
            if(n < m)
                error(Eshortdraw);
            dstid = BGLONG(a+1);
            scrnid = BGSHORT(a+5);
            refresh = a[9];
            chan = BGLONG(a+10);
            repl = a[14];
            drawrectangle(&r, a+15);
            drawrectangle(&clipr, a+31);
            value = BGLONG(a+47);
            if(drawlookup(client, dstid, false))
                error(Eimageexists);

            /*s: [[drawmesg()]] allocate image case, if screen id */
            if(scrnid){
                dscrn = drawlookupscreen(client, scrnid, &cs);
                scrn = dscrn->screen;
                if(repl || chan != scrn->image->chan)
                    error("image parameters incompatible with screen");

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
                l = memlalloc(scrn, r, reffn, 0, value);
                if(l == 0)
                    error(Edrawmem);

                addflush(l->layer->screenr);
                l->clipr = clipr;
                rectclip(&l->clipr, r);
                if(drawinstall(client, dstid, l, dscrn) == 0){
                    memldelete(l);
                    error(Edrawmem);
                }
                dscrn->ref++;
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
                continue;
            }
            /*e: [[drawmesg()]] allocate image case, if screen id */

            i = allocmemimage(r, chan); // The call

            if(i == nil)
                error(Edrawmem);
            if(repl)
                i->flags |= Frepl;
            i->clipr = clipr;
            if(!repl)
                rectclip(&i->clipr, r);

            if(drawinstall(client, dstid, i, 0) == 0){
                freememimage(i);
                error(Edrawmem);
            }
            memfillcolor(i, value); // The call
            continue;
        /*x: [[drawmesg()]] cases */
        /* free: 'f' id[4] */
        case 'f':
            printmesg(fmt="L", a, 1);
            m = 1+4;
            if(n < m)
                error(Eshortdraw);
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
            printmesg(fmt="LbR", a, 0);
            m = 1+4+1+4*4;
            if(n < m)
                error(Eshortdraw);
            ddst = drawlookup(client, BGLONG(a+1), true);
            if(ddst == nil)
                error(Enodrawimage);
            if(ddst->name)
                error("cannot change repl/clipr of shared image");
            dst = ddst->image;
            if(a[5])
                dst->flags |= Frepl;

            drawrectangle(&dst->clipr, a+6); // The call

            continue;

        /*x: [[drawmesg()]] cases */
        /* visible: 'v' */
        case 'v':
            printmesg(fmt="", a, 0);
            m = 1;
            drawflush();
            continue;

        /*x: [[drawmesg()]] cases */
        /* name an image: 'N' dstid[4] in[1] j[1] name[j] */
        case 'N':
            printmesg(fmt="Lbz", a, 0);
            m = 1+4+1+1;
            if(n < m)
                error(Eshortdraw);
            c = a[5];
            j = a[6];
            if(j == 0)  /* give me a non-empty name please */
                error(Eshortdraw);
            m += j;
            if(n < m)
                error(Eshortdraw);
            di = drawlookup(client, BGLONG(a+1), false);
            if(di == nil)
                error(Enodrawimage);
            if(di->name)
                error(Enamed);

            if(c)
                drawaddname(client, di, j, (char*)a+7); // The call?
            else{
                dn = drawlookupname(j, (char*)a+7);
                if(dn == nil)
                    error(Enoname);
                if(dn->dimage != di)
                    error(Ewrongname);
                drawdelname(dn);
            }
            continue;

        /*x: [[drawmesg()]] cases */
        /* attach to a named image: 'n' dstid[4] j[1] name[j] */
        case 'n':
            printmesg(fmt="Lz", a, 0);
            m = 1+4+1;
            if(n < m)
                error(Eshortdraw);
            j = a[5];
            if(j == 0)  /* give me a non-empty name please */
                error(Eshortdraw);
            m += j;
            if(n < m)
                error(Eshortdraw);
            dstid = BGLONG(a+1);
            if(drawlookup(client, dstid, false))
                error(Eimageexists);

            dn = drawlookupname(j, (char*)a+6); // The call?

            if(dn == nil)
                error(Enoname);
            if(drawinstall(client, dstid, dn->dimage->image, 0) == 0)
                error(Edrawmem);

            di = drawlookup(client, dstid, false);
            if(di == nil)
                error("draw: cannot happen");
            di->vers = dn->vers;
            di->name = smalloc(j+1);
            di->fromname = dn->dimage;
            di->fromname->ref++;
            memmove(di->name, a+6, j);
            di->name[j] = 0;
            client->infoid = dstid;
            continue;

        /*x: [[drawmesg()]] cases */
        /* draw: 'd' dstid[4] srcid[4] maskid[4] R[4*4] P[2*4] P[2*4] */
        case 'd':
            printmesg(fmt="LLLRPP", a, 0);
            m = 1+4+4+4+4*4+2*4+2*4;
            if(n < m)
                error(Eshortdraw);
            dst = drawimage(client, a+1);
            dstid = BGLONG(a+1);
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
            printmesg(fmt="b", a, 0);
            m = 1+1;
            if(n < m)
                error(Eshortdraw);
            client->op = a[1];
            continue;
        /*x: [[drawmesg()]] cases */
        /* draw line: 'L' dstid[4] p0[2*4] p1[2*4] end0[4] end1[4] radius[4] srcid[4] sp[2*4] */
        case 'L':
            printmesg(fmt="LPPlllLP", a, 0);
            m = 1+4+2*4+2*4+4+4+4+4+2*4;
            if(n < m)
                error(Eshortdraw);
            dst = drawimage(client, a+1);
            dstid = BGLONG(a+1);
            drawpoint(&p, a+5);
            drawpoint(&q, a+13);
            e0 = BGLONG(a+21);
            e1 = BGLONG(a+25);
            j = BGLONG(a+29);
            if(j < 0)
                error("negative line width");
            src = drawimage(client, a+33);
            drawpoint(&sp, a+37);
            op = drawclientop(client);

            memline(dst, p, q, e0, e1, j, src, sp, op); // The call

            /* avoid memlinebbox if possible */
            if(dstid==0 || dst->layer!=nil){
                /* BUG: this is terribly inefficient: update maximal containing rect*/
                r = memlinebbox(p, q, e0, e1, j);
                dstflush(dstid, dst, insetrect(r, -(1+1+j)));
            }
            continue;
        /*x: [[drawmesg()]] cases */
        /* filled polygon: 'P' dstid[4] n[2] wind[4] ignore[2*4] srcid[4] sp[2*4] p0[2*4] dp[2*2*n] */
        /* polygon: 'p' dstid[4] n[2] end0[4] end1[4] radius[4] srcid[4] sp[2*4] p0[2*4] dp[2*2*n] */
        case 'p':
        case 'P':
            printmesg(fmt="LslllLPP", a, 0);
            m = 1+4+2+4+4+4+4+2*4;
            if(n < m)
                error(Eshortdraw);
            dstid = BGLONG(a+1);
            dst = drawimage(client, a+1);
            ni = BGSHORT(a+5);
            if(ni < 0)
                error("negative count in polygon");
            e0 = BGLONG(a+7);
            e1 = BGLONG(a+11);
            j = 0;
            if(*a == 'p'){
                j = BGLONG(a+15);
                if(j < 0)
                    error("negative polygon line width");
            }
            src = drawimage(client, a+19);
            drawpoint(&sp, a+23);
            drawpoint(&p, a+31);
            ni++;
            pp = malloc(ni*sizeof(Point));
            if(pp == nil)
                error(Enomem);
            doflush = false;

            if(dstid==0 || (dst->layer && dst->layer->screen->image->data == screenimage->data))
                doflush = true;    /* simplify test in loop */
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
                    if(rectclip(&r, dst->clipr))        /* should perhaps be an arg to dstflush */
                        dstflush(dstid, dst, r);
                }
                pp[y] = p;
            }
            if(y == 1)
                dstflush(dstid, dst, Rect(p.x-esize, p.y-esize, p.x+esize+1, p.y+esize+1));
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
            printmesg(fmt="LLPlllPll", a, 0);
            m = 1+4+4+2*4+4+4+4+2*4+2*4;
            if(n < m)
                error(Eshortdraw);
            dst = drawimage(client, a+1);
            dstid = BGLONG(a+1);
            src = drawimage(client, a+5);
            drawpoint(&p, a+9);
            e0 = BGLONG(a+17);
            e1 = BGLONG(a+21);
            if(e0<0 || e1<0)
                error("invalid ellipse semidiameter");
            j = BGLONG(a+25);
            if(j < 0)
                error("negative ellipse thickness");
            drawpoint(&sp, a+29);
            c = j;
            if(*a == 'E')
                c = -1;
            ox = BGLONG(a+37);
            oy = BGLONG(a+41);
            op = drawclientop(client);
            /* high bit indicates arc angles are present */

            if(ox & (1<<31)){
                if((ox & (1<<30)) == 0)
                    ox &= ~(1<<31);
                memarc(dst, p, e0, e1, c, src, sp, ox, oy, op); // The call
            }else
                memellipse(dst, p, e0, e1, c, src, sp, op); // The call

            dstflush(dstid, dst, Rect(p.x-e0-j, p.y-e1-j, p.x+e0+j+1, p.y+e1+j+1));

            continue;
        /*x: [[drawmesg()]] cases */
        /* string: 's' dstid[4] srcid[4] fontid[4] P[2*4] clipr[4*4] sp[2*4] ni[2] ni*(index[2]) */
        /* stringbg: 'x' dstid[4] srcid[4] fontid[4] P[2*4] clipr[4*4] sp[2*4] ni[2] bgid[4] bgpt[2*4] ni*(index[2]) */
        case 's':
        case 'x':
            printmesg(fmt="LLLPRPs", a, 0);
            m = 1+4+4+4+2*4+4*4+2*4+2;
            if(*a == 'x')
                m += 4+2*4;
            if(n < m)
                error(Eshortdraw);

            dst = drawimage(client, a+1);
            dstid = BGLONG(a+1);
            src = drawimage(client, a+5);
            font = drawlookup(client, BGLONG(a+9), true);
            if(font == 0)
                error(Enodrawimage);
            if(font->nfchar == 0)
                error(Enotfont);
            drawpoint(&p, a+13);
            drawrectangle(&r, a+21);
            drawpoint(&sp, a+37);
            ni = BGSHORT(a+45);
            u = a+m;
            m += ni*2;
            if(n < m)
                error(Eshortdraw);
            clipr = dst->clipr;
            dst->clipr = r;
            op = drawclientop(client);
            bg = dst;
            if(*a == 'x'){
                /* paint background */
                bg = drawimage(client, a+47);
                drawpoint(&q, a+51);
                r.min.x = p.x;
                r.min.y = p.y-font->ascent;
                r.max.x = p.x;
                r.max.y = r.min.y+Dy(font->image->r);
                j = ni;
                while(--j >= 0){
                    ci = BGSHORT(u);
                    if(ci<0 || ci>=font->nfchar){
                        dst->clipr = clipr;
                        error(Eindex);
                    }
                    r.max.x += font->fchar[ci].width;
                    u += 2;
                }
                memdraw(dst, r, bg, q, memopaque, ZP, op);
                u -= 2*ni;
            }
            q = p;
            while(--ni >= 0){
                ci = BGSHORT(u);
                if(ci<0 || ci>=font->nfchar){
                    dst->clipr = clipr;
                    error(Eindex);
                }
                q = drawchar(dst, bg, q, src, &sp, font, ci, op);
                u += 2;
            }
            dst->clipr = clipr;
            p.y -= font->ascent;
            dstflush(dstid, dst, Rect(p.x, p.y, q.x, p.y+Dy(font->image->r)));
            continue;

        /*x: [[drawmesg()]] cases */
        /* initialize font: 'i' fontid[4] nchars[4] ascent[1] */
        case 'i':
            printmesg(fmt="Llb", a, 1);
            m = 1+4+4+1;
            if(n < m)
                error(Eshortdraw);
            dstid = BGLONG(a+1);
            if(dstid == 0)
                error("cannot use display as font");
            font = drawlookup(client, dstid, true);
            if(font == 0)
                error(Enodrawimage);
            if(font->image->layer)
                error("cannot use window as font");
            ni = BGLONG(a+5);
            if(ni<=0 || ni>4096)
                error("bad font size (4096 chars max)");
            free(font->fchar);  /* should we complain if non-zero? */
            font->fchar = malloc(ni*sizeof(FChar));
            if(font->fchar == 0)
                error("no memory for font");
            memset(font->fchar, 0, ni*sizeof(FChar));
            font->nfchar = ni;
            font->ascent = a[9];
            continue;

        /*x: [[drawmesg()]] cases */
        /* load character: 'l' fontid[4] srcid[4] index[2] R[4*4] P[2*4] left[1] width[1] */
        case 'l':
            printmesg(fmt="LLSRPbb", a, 0);
            m = 1+4+4+2+4*4+2*4+1+1;
            if(n < m)
                error(Eshortdraw);
            font = drawlookup(client, BGLONG(a+1), true);
            if(font == 0)
                error(Enodrawimage);
            if(font->nfchar == 0)
                error(Enotfont);
            src = drawimage(client, a+5);
            ci = BGSHORT(a+9);
            if(ci >= font->nfchar)
                error(Eindex);
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
        /* read: 'r' id[4] R[4*4] */
        case 'r':
            printmesg(fmt="LR", a, 0);
            m = 1+4+4*4;
            if(n < m)
                error(Eshortdraw);
            i = drawimage(client, a+1);
            drawrectangle(&r, a+5);
            if(!rectinrect(r, i->r))
                error(Ereadoutside);
            c = bytesperline(r, i->depth);
            c *= Dy(r);

            free(client->readdata);
            client->readdata = mallocz(c, 0);
            if(client->readdata == nil)
                error("readimage malloc failed");

            client->nreaddata = memunload(i, r, client->readdata, c); // The call

            if(client->nreaddata < 0){
                free(client->readdata);
                client->readdata = nil;
                error("bad readimage call");
            }
            continue;

        /*x: [[drawmesg()]] cases */
        /* write: 'y' id[4] R[4*4] data[x*1] */
        /* write from compressed data: 'Y' id[4] R[4*4] data[x*1] */
        case 'y':
        case 'Y':
            printmesg(fmt="LR", a, 0);
        //  iprint("load %c\n", *a);
            m = 1+4+4*4;
            if(n < m)
                error(Eshortdraw);
            dstid = BGLONG(a+1);
            dst = drawimage(client, a+1);
            drawrectangle(&r, a+5);
            if(!rectinrect(r, dst->r))
                error(Ewriteoutside);

            y = memload(dst, r, a+m, n-m, *a=='Y'); // The call

            if(y < 0)
                error("bad writeimage call");
            dstflush(dstid, dst, r);
            m += y;
            continue;
        /*x: [[drawmesg()]] cases */
        /* allocate screen: 'A' id[4] imageid[4] fillid[4] public[1] */
        case 'A':
            printmesg(fmt="LLLb", a, 1);
            m = 1+4+4+4+1;
            if(n < m)
                error(Eshortdraw);
            dstid = BGLONG(a+1);
            if(dstid == 0)
                error(Ebadarg);
            if(drawlookupdscreen(dstid))
                error(Escreenexists);
            ddst = drawlookup(client, BGLONG(a+5), true);
            dsrc = drawlookup(client, BGLONG(a+9), true);
            if(ddst==0 || dsrc==0)
                error(Enodrawimage);

            if(drawinstallscreen(client, 0, dstid, ddst, dsrc, a[13]) == 0) // The call
                error(Edrawmem);
            continue;

        /*x: [[drawmesg()]] cases */
        /* free screen: 'F' id[4] */
        case 'F':
            printmesg(fmt="L", a, 1);
            m = 1+4;
            if(n < m)
                error(Eshortdraw);
            drawlookupscreen(client, BGLONG(a+1), &cs);
            drawuninstallscreen(client, cs); // The call
            continue;

        /*x: [[drawmesg()]] cases */
        /* use public screen: 'S' id[4] chan[4] */
        case 'S':
            printmesg(fmt="Ll", a, 0);
            m = 1+4+4;
            if(n < m)
                error(Eshortdraw);
            dstid = BGLONG(a+1);
            if(dstid == 0)
                error(Ebadarg);
            dscrn = drawlookupdscreen(dstid);
            if(dscrn==0 || (dscrn->public==0 && dscrn->owner!=client))
                error(Enodrawscreen);
            if(dscrn->screen->image->chan != BGLONG(a+5))
                error("inconsistent chan");

            if(drawinstallscreen(client, dscrn, 0, 0, 0, 0) == 0) // The call
                error(Edrawmem);
            continue;

        /*x: [[drawmesg()]] cases */
        /* position window: 'o' id[4] r.min [2*4] screenr.min [2*4] */
        case 'o':
            printmesg(fmt="LPP", a, 0);
            m = 1+4+2*4+2*4;
            if(n < m)
                error(Eshortdraw);
            dst = drawimage(client, a+1);

            if(dst->layer){
                drawpoint(&p, a+5);
                drawpoint(&q, a+13);
                r = dst->layer->screenr;

                ni = memlorigin(dst, p, q); // The call

                if(ni < 0)
                    error("image origin failed");
                if(ni > 0){
                    addflush(r);
                    addflush(dst->layer->screenr);
                    ll = drawlookup(client, BGLONG(a+1), true);
                    drawrefreshscreen(ll, client);
                }
            }
            continue;
        /*x: [[drawmesg()]] cases */
        /* top or bottom windows: 't' top[1] nw[2] n*id[4] */
        case 't':
            printmesg(fmt="bsL", a, 0);
            m = 1+1+2;
            if(n < m)
                error(Eshortdraw);
            nw = BGSHORT(a+2);
            if(nw < 0)
                error(Ebadarg);
            if(nw == 0)
                continue;
            m += nw*4;
            if(n < m)
                error(Eshortdraw);
            lp = malloc(nw*sizeof(Memimage*));
            if(lp == 0)
                error(Enomem);
            if(waserror()){
                free(lp);
                nexterror();
            }
            for(j=0; j<nw; j++)
                lp[j] = drawimage(client, a+1+1+2+j*4);
            if(lp[0]->layer == 0)
                error("images are not windows");
            for(j=1; j<nw; j++)
                if(lp[j]->layer->screen != lp[0]->layer->screen)
                    error("images not on same screen");

            if(a[1])
                memltofrontn(lp, nw); // The call
            else
                memltorearn(lp, nw); // The call

            if(lp[0]->layer->screen->image->data == screenimage->data)
                for(j=0; j<nw; j++)
                    addflush(lp[j]->layer->screenr);
            ll = drawlookup(client, BGLONG(a+1+1+2), true);
            drawrefreshscreen(ll, client);
            poperror();
            free(lp);
            continue;

        /*x: [[drawmesg()]] cases */
        /* toggle debugging: 'D' val[1] */
        case 'D':
            printmesg(fmt="b", a, 0);
            m = 1+1;
            if(n < m)
                error(Eshortdraw);
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

/*s: function drawcmap */
/*
 * On 8 bit displays, load the default color map
 */
void
drawcmap(void)
{
    int r, g, b;
    int cr, cg, cb, v;
    int num, den;
    int i, j;

    drawactive(true);  /* to restore map from backup */
    for(r=0,i=0; r!=4; r++)
        for(v=0; v!=4; v++,i+=16){
        for(g=0,j=v-r; g!=4; g++)
            for(b=0;b!=4;b++,j++){
            den = r;
            if(g > den)
                den = g;
            if(b > den)
                den = b;
            if(den == 0)    /* divide check -- pick grey shades */
                cr = cg = cb = v*17;
            else{
                num = 17*(4*den+v);
                cr = r*num/den;
                cg = g*num/den;
                cb = b*num/den;
            }
            setcolor(i+(j&15),
                cr*0x01010101, cg*0x01010101, cb*0x01010101);
            }
    }
}
/*e: function drawcmap */

/*s: function drawblankscreen */
void
drawblankscreen(bool blank)
{
    int i, nc;
    ulong *p;

    if(blank == sdraw.blanked)
        return;
    if(!candlock())
        return;
    if(screenimage == nil){
        dunlock();
        return;
    }
    p = sdraw.savemap;
    nc = screenimage->depth > 8 ? 256 : 1<<screenimage->depth;

    /*
     * blankscreen uses the hardware to blank the screen
     * when possible.  to help in cases when it is not possible,
     * we set the color map to be all black.
     */
    if(blank == false){ /* turn screen on */
        for(i=0; i<nc; i++, p+=3)
            setcolor(i, p[0], p[1], p[2]);
        blankscreen(false);
    }else{  /* turn screen off */
        blankscreen(true);
        for(i=0; i<nc; i++, p+=3){
            getcolor(i, &p[0], &p[1], &p[2]);
            setcolor(i, 0, 0, 0);
        }
    }
    sdraw.blanked = blank;
    dunlock();
}
/*e: function drawblankscreen */

/*s: function drawactive */
/*
 * record activity on screen, changing blanking as appropriate
 */
void
drawactive(bool active)
{
    if(active){
        drawblankscreen(false);
        sdraw.blanktime = CPUS(0)->ticks;
    }else{
        if(blanktime && sdraw.blanktime 
           && TK2SEC(CPUS(0)->ticks - sdraw.blanktime)/60 >= blanktime)
            drawblankscreen(true);
    }
}
/*e: function drawactive */

/*s: function drawidletime */
int
drawidletime(void)
{
    return TK2SEC(CPUS(0)->ticks - sdraw.blanktime)/60;
}
/*e: function drawidletime */
/*e: kernel/devices/screen/devdraw.c */
