/*s: windows/rio/xfid.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h> // just for BGLONG for Qcursor
#include <thread.h>
#include <cursor.h>
#include <mouse.h>
#include <keyboard.h>
#include <frame.h>
#include <fcall.h>
#include <plumb.h>

#include "dat.h"
#include "fns.h"

void		xfidctl(void*);

/*s: constant MAXSNARF */
#define	MAXSNARF	100*1024
/*e: constant MAXSNARF */

/*s: global Einuse */
char Einuse[] =		"file in use";
/*e: global Einuse */
/*s: global Edeleted */
char Edeleted[] =	"window deleted";
/*e: global Edeleted */
/*s: global Ebadreq */
char Ebadreq[] =	"bad graphics request";
/*e: global Ebadreq */
/*s: global Etooshort */
char Etooshort[] =	"buffer too small";
/*e: global Etooshort */
/*s: global Ebadtile */
char Ebadtile[] =	"unknown tile";
/*e: global Ebadtile */
/*s: global Eshort */
char Eshort[] =		"short i/o request";
/*e: global Eshort */
/*s: global Elong */
char Elong[] = 		"snarf buffer too long";
/*e: global Elong */
/*s: global Eunkid */
char Eunkid[] = 	"unknown id in attach";
/*e: global Eunkid */
/*s: global Ebadrect */
char Ebadrect[] = 	"bad rectangle in attach";
/*e: global Ebadrect */
/*s: global Ewindow */
char Ewindow[] = 	"cannot make window";
/*e: global Ewindow */
/*s: global Enowindow */
char Enowindow[] = 	"window has no image";
/*e: global Enowindow */
/*s: global Ebadmouse */
char Ebadmouse[] = 	"bad format on /dev/mouse";
/*e: global Ebadmouse */
/*s: global Ebadwrect */
char Ebadwrect[] = 	"rectangle outside screen";
/*e: global Ebadwrect */
/*s: global Ebadoffset */
char Ebadoffset[] = 	"window read not on scan line boundary";
/*e: global Ebadoffset */
extern char Eperm[];

/*s: global xfidfree */
// list<ref_own<Xfid>> (next = Xfid.free)
static	Xfid	*xfidfree;
/*e: global xfidfree */
/*s: global xfid */
// list<ref_own<Xfid>> (next = Xfid.next)
static	Xfid	*xfid;
/*e: global xfid */
/*s: global cxfidalloc */
// chan<ref<Xfid>> (listener = filsysproc, sender = xfidallocthread)
static	Channel	*cxfidalloc;	/* chan(Xfid*) */
/*e: global cxfidalloc */
/*s: global cxfidfree */
// chan<ref<Xfid>> (listner = ??, sender = ??)
static	Channel	*cxfidfree;	/* chan(Xfid*) */
/*e: global cxfidfree */

/*s: global tsnarf */
static	char	*tsnarf;
/*e: global tsnarf */
/*s: global ntsnarf */
static	int	ntsnarf;
/*e: global ntsnarf */

/*s: enum Xxxx */
enum { 
    Alloc, 
    Free, 

    N 
};
/*e: enum Xxxx */

/*s: function xfidallocthread */
void
xfidallocthread(void*)
{
    Xfid *x;
    static Alt alts[N+1];

    alts[Alloc].c = cxfidalloc;
    alts[Alloc].v = nil;
    alts[Alloc].op = CHANRCV;
    alts[Free].c = cxfidfree;
    alts[Free].v = &x;
    alts[Free].op = CHANRCV;
    alts[N].op = CHANEND;

    for(;;){
        // event loop
        switch(alt(alts)){
        case Alloc:
            x = xfidfree;
            if(x)
                xfidfree = x->free;
            else{
                x = emalloc(sizeof(Xfid));
                x->c = chancreate(sizeof(void(*)(Xfid*)), 0);
                /*s: [[xfidallocthread()]] create flushc channel */
                x->flushc = chancreate(sizeof(int), 0);	/* notification only; nodata */
                x->flushtag = -1;
                /*e: [[xfidallocthread()]] create flushc channel */

                // insert_list(x, xfid)
                x->next = xfid;
                xfid = x;

                // new Xfid threads!
                threadcreate(xfidctl, x, 16384);
            }
            /*s: [[xfidallocthread()]] sanity check x when Alloc */
            if(x->ref != 0){
                fprint(STDERR, "%p incref %ld\n", x, x->ref);
                error("incref");
            }
            if(x->flushtag != -1)
                error("flushtag in allocate");
            /*e: [[xfidallocthread()]] sanity check x when Alloc */
            incref(x);

            sendp(cxfidalloc, x);
            break;

        case Free:
            /*s: [[xfidallocthread()]] sanity check x when Free */
            if(x->ref != 0){
                fprint(STDERR, "%p decref %ld\n", x, x->ref);
                error("decref");
            }
            if(x->flushtag != -1)
                error("flushtag in free");
            /*e: [[xfidallocthread()]] sanity check x when Free */
            // insert_list(x, xfidfree)
            x->free = xfidfree;
            xfidfree = x;
            break;
        }
    }
}
/*e: function xfidallocthread */

/*s: function xfidinit */
Channel*
xfidinit(void)
{
    cxfidalloc = chancreate(sizeof(Xfid*), 0);
    cxfidfree = chancreate(sizeof(Xfid*), 0);
    threadcreate(xfidallocthread, nil, STACK);
    return cxfidalloc;
}
/*e: function xfidinit */

/*s: function xfidctl */
void
xfidctl(void *arg)
{
    Xfid *x = arg;
    void (*f)(Xfid*);
    char buf[64];

    snprint(buf, sizeof buf, "xfid.%p", x);

    threadsetname(buf);

    for(;;){
        f = recvp(x->c);

        // Executing a xfidxxx()
        (*f)(x);

        if(decref(x) == 0)
            sendp(cxfidfree, x);
    }
}
/*e: function xfidctl */

/*s: function xfidflush */
void
xfidflush(Xfid *x)
{
    Fcall fc;
    Xfid *xf;

    for(xf=xfid; xf; xf=xf->next)
        if(xf->flushtag == x->oldtag){
            xf->flushtag = -1;
            xf->flushing = true;
            incref(xf);	/* to hold data structures up at tail of synchronization */
            if(xf->ref == 1)
                error("ref 1 in flush");
            if(canqlock(&xf->active)){
                qunlock(&xf->active);
                sendul(xf->flushc, 0);
            }else{
                qlock(&xf->active);	/* wait for him to finish */
                qunlock(&xf->active);
            }
            xf->flushing = false;

            if(decref(xf) == 0)
                sendp(cxfidfree, xf);
            break;
        }
    filsysrespond(x->fs, x, &fc, nil);
}
/*e: function xfidflush */

/*s: function xfidattach */
void
xfidattach(Xfid *x)
{
    Fcall fc;
    int id;
    Window *w = nil;
    bool newlymade = false;
    /*s: [[xfidattach()]] other locals */
    Rectangle r;
    int pid;
    bool hideit = false;
    bool scrollit;
    char *err = Eunkid;
    char *dir, errbuf[ERRMAX];
    Image *i;
    /*e: [[xfidattach()]] other locals */

    fc.qid = x->f->qid;
    qlock(&all);

    /*s: [[xfidattach()]] if mount "new ..." */
    if(strncmp(x->aname, "new", 3) == 0){	/* new -dx -dy - new syntax, as in wctl */
        pid = 0;
        if(parsewctl(nil, ZR, &r, &pid, nil, &hideit, &scrollit, &dir, x->aname, errbuf) < 0)
            err = errbuf;
        else {
            if(!goodrect(r))
                err = Ebadrect;
            else{
                if(hideit)
                    i = allocimage(display, r, view->chan, false, DWhite);
                else
                    i = allocwindow(wscreen, r, Refbackup, DWhite);
                if(i){
                    border(i, r, Selborder, display->black, ZP);
                    if(pid == 0)
                        pid = -1;	/* make sure we don't pop a shell! - UGH */
                    w = new(i, hideit, scrolling, pid, nil, nil, nil);
                    flushimage(display, 1);
                    newlymade = true;
                }else
                    err = Ewindow;
            }
       }
    }
    /*e: [[xfidattach()]] if mount "new ..." */
    else{
        // mount(fs->cfd, ..., "/mnt/wsys", ..., "2"), winid
        id = atoi(x->aname);
        w = wlookid(id);
    }

    x->f->w = w;
    /*s: [[xfidattach()]] sanity check w */
    if(w == nil){
        qunlock(&all);
        x->f->busy = false;
        filsysrespond(x->fs, x, &fc, err);
        return;
    }
    /*e: [[xfidattach()]] sanity check w */
    if(!newlymade)	/* counteract dec() in winshell() */
        incref(w);
    qunlock(&all);

    filsysrespond(x->fs, x, &fc, nil);
}
/*e: function xfidattach */

/*s: function xfidopen */
void
xfidopen(Xfid *x)
{
    Fcall fc;
    Window *w;

    w = x->f->w;
    /*s: [[xfidxxx()]] respond error if window was deleted */
    if(w->deleted){
        filsysrespond(x->fs, x, &fc, Edeleted);
        return;
    }
    /*e: [[xfidxxx()]] respond error if window was deleted */
    switch(FILE(x->f->qid)){
    /*s: [[xfidopen()]] cases */
    case Qconsctl:
        if(w->ctlopen){
            filsysrespond(x->fs, x, &fc, Einuse);
            return;
        }
        w->ctlopen = true;
        break;
    /*x: [[xfidopen()]] cases */
    case Qmouse:
        if(w->mouseopen){
            filsysrespond(x->fs, x, &fc, Einuse);
            return;
        }
        w->mouseopen = true;

        /*
         * Reshaped: there's a race if the appl. opens the
         * window, is resized, and then opens the mouse,
         * but that's rare.  The alternative is to generate
         * a resized event every time a new program starts
         * up in a window that has been resized since the
         * dawn of time.  We choose the lesser evil.
         */
        w->resized = false;
        break;
    /*x: [[xfidopen()]] cases */
    case Qwctl:
        if(x->mode==OREAD || x->mode==ORDWR){
            /*
             * It would be much nicer to implement fan-out for wctl reads,
             * so multiple people can see the resizings, but rio just isn't
             * structured for that.  It's structured for /dev/cons, which gives
             * alternate data to alternate readers.  So to keep things sane for
             * wctl, we compromise and give an error if two people try to
             * open it.  Apologies.
             */
            if(w->wctlopen){
                filsysrespond(x->fs, x, &fc, Einuse);
                return;
            }
            w->wctlopen = true;
            w->wctlready = true;
            wsendctlmesg(w, Wakeup, ZR, nil);
        }
        break;
    /*x: [[xfidopen()]] cases */
    case Qsnarf:
        if(x->mode==ORDWR || x->mode==OWRITE){
            if(tsnarf)
                free(tsnarf);	/* collision, but OK */
            ntsnarf = 0;
            tsnarf = malloc(1);
        }
        break;
    /*x: [[xfidopen()]] cases */
    case Qkbdin:
        if(w !=  wkeyboard){
            filsysrespond(x->fs, x, &fc, Eperm);
            return;
        }
        break;
    /*e: [[xfidopen()]] cases */
    }

    x->f->open = true;
    x->f->mode = x->mode;

    fc.qid = x->f->qid;
    fc.iounit = messagesize-IOHDRSZ;
    filsysrespond(x->fs, x, &fc, nil);
}
/*e: function xfidopen */

/*s: function xfidclose */
void
xfidclose(Xfid *x)
{
    Fcall fc;
    Window *w;
    /*s: [[xfidclose()]] other locals */
    int nb, nulls;
    /*e: [[xfidclose()]] other locals */

    w = x->f->w;
    switch(FILE(x->f->qid)){
    /*s: [[xfidclose()]] cases */
    case Qconsctl:
        /*s: [[xfidclose()]] Qconsctl case, if rawing */
        if(w->rawing){
            w->rawing = false;
            wsendctlmesg(w, Rawoff, ZR, nil);
        }
        /*e: [[xfidclose()]] Qconsctl case, if rawing */
        /*s: [[xfidclose()]] Qconsctl case, if holding */
        if(w->holding){
            w->holding = false;
            wsendctlmesg(w, Holdoff, ZR, nil);
        }
        /*e: [[xfidclose()]] Qconsctl case, if holding */
        w->ctlopen = false;
        break;
    /*x: [[xfidclose()]] cases */
    case Qcursor:
        w->cursorp = nil;
        wsetcursor(w, false);
        break;
    /*x: [[xfidclose()]] cases */
    case Qmouse:
        w->mouseopen = false;

        w->resized = false;
        if(w->i != nil)
            wsendctlmesg(w, Refresh, w->i->r, nil);
        break;
    /*x: [[xfidclose()]] cases */
    case Qwctl:
        if(x->f->mode==OREAD || x->f->mode==ORDWR)
            w->wctlopen = false;
    break;
    /*x: [[xfidclose()]] cases */
    /* odd behavior but really ok: replace snarf buffer when /dev/snarf is closed */
    case Qsnarf:
        if(x->f->mode==ORDWR || x->f->mode==OWRITE){
            snarf = runerealloc(snarf, ntsnarf+1);
            cvttorunes(tsnarf, ntsnarf, snarf, &nb, &nsnarf, &nulls);
            free(tsnarf);
            tsnarf = nil;
            ntsnarf = 0;
        }
        break;
    /*e: [[xfidclose()]] cases */
    }
    wclose(w);
    filsysrespond(x->fs, x, &fc, nil);
}
/*e: function xfidclose */

/*s: enum _anon_ (windows/rio/xfid.c)2 */
enum { CWdata, CWflush, NCW };
/*e: enum _anon_ (windows/rio/xfid.c)2 */

/*s: function xfidwrite */
void
xfidwrite(Xfid *x)
{
    Fcall fc;
    Window *w;
    uint qid;
    int off, cnt;
    char buf[256];
    /*s: [[xfidwrite()]] other locals */
    char *p;
    Point pt;
    /*x: [[xfidwrite()]] other locals */
    Rune *r;
    int nr; // nb runes
    int nb; // nb bytes
    /*x: [[xfidwrite()]] other locals */
    Alt alts[NCW+1];
    Conswritemesg cwm;
    Stringpair pair;
    /*x: [[xfidwrite()]] other locals */
    int c;
    /*e: [[xfidwrite()]] other locals */
    
    w = x->f->w;
    /*s: [[xfidxxx()]] respond error if window was deleted */
    if(w->deleted){
        filsysrespond(x->fs, x, &fc, Edeleted);
        return;
    }
    /*e: [[xfidxxx()]] respond error if window was deleted */
    qid = FILE(x->f->qid);
    cnt = x->count;
    off = x->offset;
    x->data[cnt] = 0;

    switch(qid){
    /*s: [[xfidwrite()]] cases */
    case Qmouse:
        if(w!=input || Dx(w->screenr)<=0)
            break;
        if(x->data[0] != 'm'){
            filsysrespond(x->fs, x, &fc, Ebadmouse);
            return;
        }
        p = nil;
        pt.x = strtoul(x->data+1, &p, 0);
        if(p == nil){
            filsysrespond(x->fs, x, &fc, Eshort);
            return;
        }
        pt.y = strtoul(p, nil, 0);
        if(w==input && wpointto(mouse->xy)==w)
            wsendctlmesg(w, Movemouse, Rpt(pt, pt), nil);
        break;
    /*x: [[xfidwrite()]] cases */
    case Qcons:

        /*s: [[xfidwrite()]] when Qcons, look for previous partial rune bytes */
        nr = x->f->nrpart;
        if(nr > 0){
            memmove(x->data+nr, x->data, cnt);	/* there's room: see malloc in filsysproc */
            memmove(x->data, x->f->rpart, nr);
            cnt += nr;
            x->f->nrpart = 0;
        }
        /*e: [[xfidwrite()]] when Qcons, look for previous partial rune bytes */
        r = runemalloc(cnt);
        cvttorunes(x->data, cnt-UTFmax, r, &nb, &nr, nil);
        /*s: [[xfidwrite()]] when Qcons, look if more full runes */
        /* approach end of buffer */
        while(fullrune(x->data+nb, cnt-nb)){
            c = nb;
            nb += chartorune(&r[nr], x->data+c);
            if(r[nr])
                nr++;
        }
        /*e: [[xfidwrite()]] when Qcons, look if more full runes */
        /*s: [[xfidwrite()]] when Qcons, store remaining partial rune bytes */
        // assert(cnt-nb < UTFMAX);
        if(nb < cnt){
            memmove(x->f->rpart, x->data + nb, cnt-nb);
            x->f->nrpart = cnt-nb;
        }
        /*e: [[xfidwrite()]] when Qcons, store remaining partial rune bytes */

        /*s: [[xfidxxx()]] set flushtag */
        x->flushtag = x->tag;
        /*e: [[xfidxxx()]] set flushtag */

        alts[CWdata].c = w->conswrite;
        alts[CWdata].v = &cwm;
        alts[CWdata].op = CHANRCV;
        /*s: [[xfidwrite()]] when Qcons, set alts for flush */
        alts[CWflush].c = x->flushc;
        alts[CWflush].v = nil;
        alts[CWflush].op = CHANRCV;
        /*e: [[xfidwrite()]] when Qcons, set alts for flush */
        alts[NCW].op = CHANEND;

        switch(alt(alts)){
        case CWdata:
            break;
        /*s: [[xfidwrite()]] when Qcons, switch alt flush case */
        case CWflush:
            filsyscancel(x);
            return;
        /*e: [[xfidwrite()]] when Qcons, switch alt flush case */
        }

        /* received data */
        /*s: [[xfidxxx()]] unset flushtag */
        x->flushtag = -1;
        /*e: [[xfidxxx()]] unset flushtag */
        /*s: [[xfidwrite()]] when Qcons, if flushing */
        if(x->flushing){
            recv(x->flushc, nil);	/* wake up flushing xfid */
            pair.s = runemalloc(1);
            pair.ns = 0;
            send(cwm.cw, &pair);		/* wake up window with empty data */
            filsyscancel(x);
            return;
        }
        /*e: [[xfidwrite()]] when Qcons, if flushing */

        qlock(&x->active);
        pair.s = r;
        pair.ns = nr;
        send(cwm.cw, &pair);
        fc.count = x->count;
        filsysrespond(x->fs, x, &fc, nil);
        qunlock(&x->active);
        return;
    /*x: [[xfidwrite()]] cases */
    case Qconsctl:
        /*s: [[xfidwrite()]] Qconsctl case */
        if(strncmp(x->data, "rawon", 5)==0){
            /*s: [[xfidwrite()]] Qconsctl case, if rawon message and holding mode */
            if(w->holding){
                w->holding = false;
                wsendctlmesg(w, Holdoff, ZR, nil);
            }
            /*e: [[xfidwrite()]] Qconsctl case, if rawon message and holding mode */
            if(w->rawing++ == 0)
                wsendctlmesg(w, Rawon, ZR, nil);
            break;
        }
        if(strncmp(x->data, "rawoff", 6)==0 && w->rawing){
            if(--w->rawing == 0)
                wsendctlmesg(w, Rawoff, ZR, nil);
            break;
        }
        /*x: [[xfidwrite()]] Qconsctl case */
        if(strncmp(x->data, "holdon", 6)==0){
            if(w->holding++ == 0)
                wsendctlmesg(w, Holdon, ZR, nil);
            break;
        }
        if(strncmp(x->data, "holdoff", 7)==0 && w->holding){
            if(--w->holding == false)
                wsendctlmesg(w, Holdoff, ZR, nil);
            break;
        }
        /*e: [[xfidwrite()]] Qconsctl case */
        // else
        filsysrespond(x->fs, x, &fc, "unknown control message");
        return;
    /*x: [[xfidwrite()]] cases */
    case Qcursor:
        if(cnt < 2*4+2*2*16)
            w->cursorp = nil;
        else{
            w->cursor.offset.x = BGLONG(x->data+0*4);
            w->cursor.offset.y = BGLONG(x->data+1*4);
            memmove(w->cursor.clr, x->data+2*4, 2*2*16);
            w->cursorp = &w->cursor;
        }
        wsetcursor(w, !sweeping);
        break;
    /*x: [[xfidwrite()]] cases */
    case Qlabel:
        if(off != 0){
            filsysrespond(x->fs, x, &fc, "non-zero offset writing label");
            return;
        }
        free(w->label);
        w->label = emalloc(cnt+1);
        memmove(w->label, x->data, cnt);
        w->label[cnt] = '\0';
        break;
    /*x: [[xfidwrite()]] cases */
    case Qwdir:
        if(cnt == 0)
            break;
        if(x->data[cnt-1] == '\n'){
            if(cnt == 1)
                break;
            x->data[cnt-1] = '\0';
        }
        /* assume data comes in a single write */
        /*
          * Problem: programs like dossrv, ftp produce illegal UTF;
          * we must cope by converting it first.
          */
        snprint(buf, sizeof buf, "%.*s", cnt, x->data);
        if(buf[0] == '/'){
            free(w->dir);
            w->dir = estrdup(buf);
        }else{
            p = emalloc(strlen(w->dir) + 1 + strlen(buf) + 1);
            sprint(p, "%s/%s", w->dir, buf);
            free(w->dir);
            w->dir = cleanname(p);
        }
        break;
    /*x: [[xfidwrite()]] cases */
    case Qwctl:
        if(writewctl(x, buf) < 0){
            filsysrespond(x->fs, x, &fc, buf);
            return;
        }
        flushimage(display, true);
        break;
    /*x: [[xfidwrite()]] cases */
    case Qsnarf:
        /* always append only */
        if(ntsnarf > MAXSNARF){	/* avoid thrashing when people cut huge text */
            filsysrespond(x->fs, x, &fc, Elong);
            return;
        }
        tsnarf = erealloc(tsnarf, ntsnarf+cnt+1);	/* room for NUL */
        memmove(tsnarf+ntsnarf, x->data, cnt);
        ntsnarf += cnt;
        snarfversion++;
        break;
    /*x: [[xfidwrite()]] cases */
    case Qkbdin:
        keyboardsend(x->data, cnt);
        break;
    /*e: [[xfidwrite()]] cases */
    default:
        fprint(STDERR, buf, "unknown qid %d in write\n", qid);
        sprint(buf, "unknown qid in write");
        filsysrespond(x->fs, x, &fc, buf);
        return;
    }

    fc.count = cnt;
    filsysrespond(x->fs, x, &fc, nil);
}
/*e: function xfidwrite */

/*s: function readwindow */
int
readwindow(Image *i, char *t, Rectangle r, int offset, int n)
{
    int ww, y;

    offset -= 5*12;
    ww = bytesperline(r, view->depth);
    r.min.y += offset/ww;
    if(r.min.y >= r.max.y)
        return 0;
    y = r.min.y + n/ww;
    if(y < r.max.y)
        r.max.y = y;
    if(r.max.y <= r.min.y)
        return 0;
    return unloadimage(i, r, (uchar*)t, n);
}
/*e: function readwindow */

/*s: enum _anon_ (windows/rio/xfid.c)3 */
enum { CRdata, CRflush, NCR };
/*e: enum _anon_ (windows/rio/xfid.c)3 */
/*s: enum _anon_ (windows/rio/xfid.c)4 */
enum { MRdata, MRflush, NMR };
/*e: enum _anon_ (windows/rio/xfid.c)4 */
/*s: enum _anon_ (windows/rio/xfid.c)5 */
enum { WCRdata, WCRflush, NWCR };
/*e: enum _anon_ (windows/rio/xfid.c)5 */

/*s: function xfidread */
void
xfidread(Xfid *x)
{
    Window *w;
    uint qid;
    int off, cnt;
    Fcall fc;
    /*s: [[xfidread()]] other locals */
    Alt alts[NCR+1];
    Mousereadmesg mrm;
    Mouse ms;
    int n, c;
    /*x: [[xfidread()]] other locals */
    Consreadmesg crm;
    Channel *c1, *c2;	/* chan (tuple(char*, int)) */
    char *t;
    Stringpair pair;
    /*x: [[xfidread()]] other locals */
    Image *i;
    Rectangle r;
    char buf[128];
    char cbuf[30];
    /*x: [[xfidread()]] other locals */
    Consreadmesg cwrm;
    /*e: [[xfidread()]] other locals */
    
    w = x->f->w;
    /*s: [[xfidxxx()]] respond error if window was deleted */
    if(w->deleted){
        filsysrespond(x->fs, x, &fc, Edeleted);
        return;
    }
    /*e: [[xfidxxx()]] respond error if window was deleted */
    qid = FILE(x->f->qid);
    off = x->offset;
    cnt = x->count;

    switch(qid){
    /*s: [[xfidread()]] cases */
    case Qmouse:
        /*s: [[xfidxxx()]] set flushtag */
        x->flushtag = x->tag;
        /*e: [[xfidxxx()]] set flushtag */

        alts[MRdata].c = w->mouseread;
        alts[MRdata].v = &mrm;
        alts[MRdata].op = CHANRCV;
        /*s: [[xfidread()]] when Qmouse, set alts for flush */
        alts[MRflush].c = x->flushc;
        alts[MRflush].v = nil;
        alts[MRflush].op = CHANRCV;
        /*e: [[xfidread()]] when Qmouse, set alts for flush */
        alts[NMR].op = CHANEND;

        switch(alt(alts)){
        case MRdata:
            break;
        /*s: [[xfidread()]] when Qmouse, switch alt flush case */
        case MRflush:
            filsyscancel(x);
            return;
        /*e: [[xfidread()]] when Qmouse, switch alt flush case */
        }
        /* received data */
        /*s: [[xfidxxx()]] unset flushtag */
        x->flushtag = -1;
        /*e: [[xfidxxx()]] unset flushtag */
        /*s: [[xfidread()]] when Qmouse, if flushing */
        if(x->flushing){
            recv(x->flushc, nil);		/* wake up flushing xfid */
            recv(mrm.cm, nil);			/* wake up window and toss data */
            filsyscancel(x);
            return;
        }
        /*e: [[xfidread()]] when Qmouse, if flushing */

        qlock(&x->active);
        recv(mrm.cm, &ms);
        c = 'm';
        /*s: [[xfidread()]] when Qmouse, adjust c for resize message if resized */
        if(w->resized)
            c = 'r';
        /*e: [[xfidread()]] when Qmouse, adjust c for resize message if resized */
        n = sprint(buf, "%c%11d %11d %11d %11ld ", c, ms.xy.x, ms.xy.y, ms.buttons, ms.msec);
        w->resized = false;

        fc.data = buf;
        fc.count = min(n, cnt);
        filsysrespond(x->fs, x, &fc, nil);
        qunlock(&x->active);
        break;
    /*x: [[xfidread()]] cases */
    case Qcons:
        /*s: [[xfidxxx()]] set flushtag */
        x->flushtag = x->tag;
        /*e: [[xfidxxx()]] set flushtag */

        alts[CRdata].c = w->consread;
        alts[CRdata].v = &crm;
        alts[CRdata].op = CHANRCV;
        /*s: [[xfidread()]] when Qcons, set alts for flush */
        alts[CRflush].c = x->flushc;
        alts[CRflush].v = nil;
        alts[CRflush].op = CHANRCV;
        /*e: [[xfidread()]] when Qcons, set alts for flush */
        alts[NCR].op = CHANEND;

        switch(alt(alts)){
        case CRdata:
            break;
        /*s: [[xfidread()]] when Qcons, switch alt flush case */
        case CRflush:
            filsyscancel(x);
            return;
        /*e: [[xfidread()]] when Qcons, switch alt flush case */
        }
        /* received data */
        /*s: [[xfidxxx()]] unset flushtag */
        x->flushtag = -1;
        /*e: [[xfidxxx()]] unset flushtag */

        c1 = crm.c1;
        c2 = crm.c2;
        t = malloc(cnt+UTFmax+1);	/* room to unpack partial rune plus */
        pair.s = t;
        pair.ns = cnt;
        send(c1, &pair);

        /*s: [[xfidread()]] when Qcons, if flushing */
        if(x->flushing){
            recv(x->flushc, nil);	/* wake up flushing xfid */
            recv(c2, nil);			/* wake up window and toss data */
            free(t);
            filsyscancel(x);
            return;
        }
        /*e: [[xfidread()]] when Qcons, if flushing */

        qlock(&x->active);
        recv(c2, &pair);
        fc.data = pair.s;
        fc.count = pair.ns;
        filsysrespond(x->fs, x, &fc, nil);
        free(t);
        qunlock(&x->active);
        break;
    /*x: [[xfidread()]] cases */
    case Qcursor:
        filsysrespond(x->fs, x, &fc, "cursor read not implemented");
        break;
    /*x: [[xfidread()]] cases */
    case Qwinname:
        n = strlen(w->name);
        if(n == 0){
            filsysrespond(x->fs, x, &fc, "window has no name");
            break;
        }
        t = estrdup(w->name);
        goto Text;
    /*x: [[xfidread()]] cases */
    case Qwindow:
        i = w->i;
        if(i == nil || Dx(w->screenr)<=0){
            filsysrespond(x->fs, x, &fc, Enowindow);
            return;
        }
        r = w->screenr;
        /* fall through */

    caseImage:
        if(off < 5*12){
            n = sprint(buf, "%11s %11d %11d %11d %11d ",
                chantostr(cbuf, view->chan),
                i->r.min.x, i->r.min.y, i->r.max.x, i->r.max.y);
            t = estrdup(buf);
            goto Text;
        }
        t = malloc(cnt);
        fc.data = t;
        n = readwindow(i, t, r, off, cnt);	/* careful; fc.count is unsigned */
        if(n < 0){
            buf[0] = 0;
            errstr(buf, sizeof buf);
            filsysrespond(x->fs, x, &fc, buf);
        }else{
            fc.count = n;
            filsysrespond(x->fs, x, &fc, nil);
        }
        free(t);
        return;
    /*x: [[xfidread()]] cases */
    case Qtext:
        t = wcontents(w, &n);
        goto Text;

    Text:
        if(off > n){
            off = n;
            cnt = 0;
        }
        if(off+cnt > n)
            cnt = n-off;

        fc.data = t + off;
        fc.count = cnt;
        filsysrespond(x->fs, x, &fc, nil);
        free(t);
        break;
    /*x: [[xfidread()]] cases */
    case Qwinid:
        n = sprint(buf, "%11d ", w->id);
        t = estrdup(buf);
        goto Text;
    /*x: [[xfidread()]] cases */
    case Qlabel:
        n = strlen(w->label);
        if(off > n)
            off = n;
        if(off+cnt > n)
            cnt = n - off;

        fc.data = w->label + off;
        fc.count = cnt;
        filsysrespond(x->fs, x, &fc, nil);
        break;
    /*x: [[xfidread()]] cases */
    case Qwdir:
        t = estrdup(w->dir);
        n = strlen(t);
        goto Text;
    /*x: [[xfidread()]] cases */
    case Qscreen:
        i = display->image;
        if(i == nil){
            filsysrespond(x->fs, x, &fc, "no top-level screen");
            break;
        }
        r = i->r;
        goto caseImage;
    /*x: [[xfidread()]] cases */
    case Qwctl:	/* read returns rectangle, hangs if not resized */
        if(cnt < 4*12){
            filsysrespond(x->fs, x, &fc, Etooshort);
            break;
        }
        /*s: [[xfidxxx()]] set flushtag */
        x->flushtag = x->tag;
        /*e: [[xfidxxx()]] set flushtag */

        alts[WCRdata].c = w->wctlread;
        alts[WCRdata].v = &cwrm;
        alts[WCRdata].op = CHANRCV;
        /*s: [[xfidread()]] when Qwctl, set alts for flush */
        alts[WCRflush].c = x->flushc;
        alts[WCRflush].v = nil;
        alts[WCRflush].op = CHANRCV;
        /*e: [[xfidread()]] when Qwctl, set alts for flush */
        alts[NMR].op = CHANEND;

        switch(alt(alts)){
        case WCRdata:
            break;
        /*s: [[xfidread()]] when Qwctl, switch alt flush case */
        case WCRflush:
            filsyscancel(x);
            return;
        /*e: [[xfidread()]] when Qwctl, switch alt flush case */
        }

        /* received data */
        /*s: [[xfidxxx()]] unset flushtag */
        x->flushtag = -1;
        /*e: [[xfidxxx()]] unset flushtag */
        c1 = cwrm.c1;
        c2 = cwrm.c2;
        t = malloc(cnt+1);	/* be sure to have room for NUL */
        pair.s = t;
        pair.ns = cnt+1;
        send(c1, &pair);
        /*s: [[xfidread()]] when Qwctl, if flushing */
        if(x->flushing){
            recv(x->flushc, nil);	/* wake up flushing xfid */
            recv(c2, nil);			/* wake up window and toss data */
            free(t);
            filsyscancel(x);
            return;
        }
        /*e: [[xfidread()]] when Qwctl, if flushing */

        qlock(&x->active);
        recv(c2, &pair);
        fc.data = pair.s;
        if(pair.ns > cnt)
            pair.ns = cnt;
        fc.count = pair.ns;
        filsysrespond(x->fs, x, &fc, nil);
        free(t);
        qunlock(&x->active);
        break;
    /*x: [[xfidread()]] cases */
    /* The algorithm for snarf and text is expensive but easy and rarely used */
    case Qsnarf:
        getsnarf();
        if(nsnarf)
            t = runetobyte(snarf, nsnarf, &n);
        else {
            t = nil;
            n = 0;
        }
        goto Text;
    /*e: [[xfidread()]] cases */
    default:
        fprint(STDERR, "unknown qid %d in read\n", qid);
        sprint(buf, "unknown qid in read");
        filsysrespond(x->fs, x, &fc, buf);
        break;
        }
}
/*e: function xfidread */
/*e: windows/rio/xfid.c */
