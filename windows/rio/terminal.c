/*s: windows/rio/terminal.c */
#include <u.h>
#include <libc.h>

// for dat.h
#include <draw.h>
#include <mouse.h>
#include <cursor.h>
#include <keyboard.h>
#include <frame.h>
#include <fcall.h>
#include <thread.h>

#include <plumb.h>
#include <complete.h>


#include "dat.h"
#include "fns.h"

/*s: enum _anon_ (windows/rio/wind.c) */
enum
{
    HiWater	= 640000,	/* max size of history */
    LoWater	= 400000,	/* min size of history after max'ed */
    MinWater	= 20000,	/* room to leave available when reallocating */
};
/*e: enum _anon_ (windows/rio/wind.c) */

/*s: enum _anon_ (windows/rio/rio.c)2 */
enum
{
    Cut,
    Paste,
    Snarf,
    Plumb,
    Send,
    Scroll,
};
/*e: enum _anon_ (windows/rio/rio.c)2 */

/*s: global menu2str */
char*		menu2str[] = {
 [Cut]		"cut",
 [Paste]	"paste",
 [Snarf]	"snarf",
 [Plumb]	"plumb",
 [Send]		"send",
 [Scroll]	"scroll",
 nil
};
/*e: global menu2str */

/*s: global menu2 */
Menu menu2 =
{
    menu2str
};
/*e: global menu2 */

/*s: global clickwin */
static Window	*clickwin;
/*e: global clickwin */
/*s: global clickmsec */
static uint	clickmsec;
/*e: global clickmsec */
/*s: global selectwin */
static Window	*selectwin;
/*e: global selectwin */
/*s: global selectq */
static uint	selectq;
/*e: global selectq */


//----------------------------------------------------------------------------
// Completion
//----------------------------------------------------------------------------


/*s: function windfilewidth */
int
windfilewidth(Window *w, uint q0, int oneelement)
{
    uint q;
    Rune r;

    q = q0;
    while(q > 0){
        r = w->r[q-1];
        if(r<=' ')
            break;
        if(oneelement && r=='/')
            break;
        --q;
    }
    return q0-q;
}
/*e: function windfilewidth */

/*s: function showcandidates */
void
showcandidates(Window *w, Completion *c)
{
    int i;
    Fmt f;
    Rune *rp;
    uint nr, qline, q0;
    char *s;

    runefmtstrinit(&f);
    if (c->nmatch == 0)
        s = "[no matches in ";
    else
        s = "[";
    if(c->nfile > 32)
        fmtprint(&f, "%s%d files]\n", s, c->nfile);
    else{
        fmtprint(&f, "%s", s);
        for(i=0; i<c->nfile; i++){
            if(i > 0)
                fmtprint(&f, " ");
            fmtprint(&f, "%s", c->filename[i]);
        }
        fmtprint(&f, "]\n");
    }
    /* place text at beginning of line before host point */
    qline = w->qh;
    while(qline>0 && w->r[qline-1] != '\n')
        qline--;

    rp = runefmtstrflush(&f);
    nr = runestrlen(rp);

    q0 = w->q0;
    q0 += winsert(w, rp, runestrlen(rp), qline) - qline;
    free(rp);
    wsetselect(w, q0+nr, q0+nr);
}
/*e: function showcandidates */

/*s: function namecomplete */
Rune*
namecomplete(Window *w)
{
    int nstr, npath;
    Rune *rp, *path, *str;
    Completion *c;
    char *s, *dir, *root;

    /* control-f: filename completion; works back to white space or / */
    if(w->q0<w->nr && w->r[w->q0]>' ')	/* must be at end of word */
        return nil;
    nstr = windfilewidth(w, w->q0, true);
    str = runemalloc(nstr);
    runemove(str, w->r+(w->q0-nstr), nstr);
    npath = windfilewidth(w, w->q0-nstr, false);
    path = runemalloc(npath);
    runemove(path, w->r+(w->q0-nstr-npath), npath);
    rp = nil;

    /* is path rooted? if not, we need to make it relative to window path */
    if(npath>0 && path[0]=='/'){
        dir = malloc(UTFmax*npath+1);
        sprint(dir, "%.*S", npath, path);
    }else{
        if(strcmp(w->dir, "") == 0)
            root = ".";
        else
            root = w->dir;
        dir = malloc(strlen(root)+1+UTFmax*npath+1);
        sprint(dir, "%s/%.*S", root, npath, path);
    }
    dir = cleanname(dir);

    s = smprint("%.*S", nstr, str);
    c = complete(dir, s);
    free(s);
    if(c == nil)
        goto Return;

    if(!c->advance)
        showcandidates(w, c);

    if(c->advance)
        rp = runesmprint("%s", c->string);

  Return:
    freecompletion(c);
    free(dir);
    free(path);
    free(str);
    return rp;
}
/*e: function namecomplete */


//----------------------------------------------------------------------------
// Scrolling
//----------------------------------------------------------------------------

/*s: function wframescroll */
void
wframescroll(Window *w, int dl)
{
    uint q0;

    if(dl == 0){
        wscrsleep(w, 100);
        return;
    }
    if(dl < 0){
        q0 = wbacknl(w, w->org, -dl);
        if(selectq > w->org+w->p0)
            wsetselect(w, w->org+w->p0, selectq);
        else
            wsetselect(w, selectq, w->org+w->p0);
    }else{
        if(w->org+w->nchars == w->nr)
            return;
        q0 = w->org+frcharofpt(w, Pt(w->Frame.r.min.x, w->Frame.r.min.y+dl*w->font->height));
        if(selectq >= w->org+w->p1)
            wsetselect(w, w->org+w->p1, selectq);
        else
            wsetselect(w, selectq, w->org+w->p1);
    }
    wsetorigin(w, q0, true);
}
/*e: function wframescroll */

/*s: function framescroll */
/*
 * called from frame library
 */
void
framescroll(Frame *f, int dl)
{
    if(f != &selectwin->Frame)
        error("frameselect not right frame");
    wframescroll(selectwin, dl);
}
/*e: function framescroll */


//----------------------------------------------------------------------------
// Selection
//----------------------------------------------------------------------------

/*s: function wclickmatch */
int
wclickmatch(Window *w, int cl, int cr, int dir, uint *q)
{
    Rune c;
    int nest;

    nest = 1;
    for(;;){
        if(dir > 0){
            if(*q == w->nr)
                break;
            c = w->r[*q];
            (*q)++;
        }else{
            if(*q == 0)
                break;
            (*q)--;
            c = w->r[*q];
        }
        if(c == cr){
            if(--nest==0)
                return 1;
        }else if(c == cl)
            nest++;
    }
    return cl=='\n' && nest==1;
}
/*e: function wclickmatch */


/*s: global left1 */
static Rune left1[] =  { L'{', L'[', L'(', L'<', L'«', 0 };
/*e: global left1 */
/*s: global right1 */
static Rune right1[] = { L'}', L']', L')', L'>', L'»', 0 };
/*e: global right1 */
/*s: global left2 */
static Rune left2[] =  { L'\n', 0 };
/*e: global left2 */
/*s: global left3 */
static Rune left3[] =  { L'\'', L'"', L'`', 0 };
/*e: global left3 */

/*s: global left */
Rune *left[] = {
    left1,
    left2,
    left3,
    nil
};
/*e: global left */
/*s: global right */
Rune *right[] = {
    right1,
    left2,
    left3,
    nil
};
/*e: global right */

/*s: function wdoubleclick */
void
wdoubleclick(Window *w, uint *q0, uint *q1)
{
    int c, i;
    Rune *r, *l, *p;
    uint q;

    for(i=0; left[i]!=nil; i++){
        q = *q0;
        l = left[i];
        r = right[i];
        /* try matching character to left, looking right */
        if(q == 0)
            c = '\n';
        else
            c = w->r[q-1];
        p = strrune(l, c);
        if(p != nil){
            if(wclickmatch(w, c, r[p-l], 1, &q))
                *q1 = q-(c!='\n');
            return;
        }
        /* try matching character to right, looking left */
        if(q == w->nr)
            c = '\n';
        else
            c = w->r[q];
        p = strrune(r, c);
        if(p != nil){
            if(wclickmatch(w, c, l[p-r], -1, &q)){
                *q1 = *q0+(*q0<w->nr && c=='\n');
                *q0 = q;
                if(c!='\n' || q!=0 || w->r[0]=='\n')
                    (*q0)++;
            }
            return;
        }
    }
    /* try filling out word to right */
    while(*q1<w->nr && isalnum(w->r[*q1]))
        (*q1)++;
    /* try filling out word to left */
    while(*q0>0 && isalnum(w->r[*q0-1]))
        (*q0)--;
}
/*e: function wdoubleclick */




/*s: function wselect */
void
wselect(Window *w)
{
    uint q0, q1;
    int b, x, y, first;

    first = 1;
    selectwin = w;
    /*
     * Double-click immediately if it might make sense.
     */
    b = w->mc.buttons;
    q0 = w->q0;
    q1 = w->q1;
    selectq = w->org+frcharofpt(w, w->mc.xy);
    if(clickwin==w && w->mc.msec-clickmsec<500)
    if(q0==q1 && selectq==w->q0){
        wdoubleclick(w, &q0, &q1);
        wsetselect(w, q0, q1);
        flushimage(display, 1);
        x = w->mc.xy.x;
        y = w->mc.xy.y;
        /* stay here until something interesting happens */
        do
            readmouse(&w->mc);
        while(w->mc.buttons==b && abs(w->mc.xy.x-x)<3 && abs(w->mc.xy.y-y)<3);
        w->mc.xy.x = x;	/* in case we're calling frselect */
        w->mc.xy.y = y;
        q0 = w->q0;	/* may have changed */
        q1 = w->q1;
        selectq = q0;
    }
    if(w->mc.buttons == b){
        w->scroll = framescroll;
        frselect(w, &w->mc);
        /* horrible botch: while asleep, may have lost selection altogether */
        if(selectq > w->nr)
            selectq = w->org + w->p0;
        w->Frame.scroll = nil;
        if(selectq < w->org)
            q0 = selectq;
        else
            q0 = w->org + w->p0;
        if(selectq > w->org+w->nchars)
            q1 = selectq;
        else
            q1 = w->org+w->p1;
    }
    if(q0 == q1){
        if(q0==w->q0 && clickwin==w && w->mc.msec-clickmsec<500){
            wdoubleclick(w, &q0, &q1);
            clickwin = nil;
        }else{
            clickwin = w;
            clickmsec = w->mc.msec;
        }
    }else
        clickwin = nil;
    wsetselect(w, q0, q1);
    flushimage(display, 1);
    while(w->mc.buttons){
        w->mc.msec = 0;
        b = w->mc.buttons;
        if(b & 6){
            if(b & 2){
                wsnarf(w);
                wcut(w);
            }else{
                if(first){
                    first = 0;
                    getsnarf();
                }
                wpaste(w);
            }
        }
        wscrdraw(w);
        flushimage(display, 1);
        while(w->mc.buttons == b)
            readmouse(&w->mc);
        clickwin = nil;
    }
}
/*e: function wselect */

//----------------------------------------------------------------------------
// Clicking
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// Cut/copy/paste
//----------------------------------------------------------------------------

/*s: function wsnarf */
void
wsnarf(Window *w)
{
    if(w->q1 == w->q0)
        return;
    nsnarf = w->q1 - w->q0;
    snarf = runerealloc(snarf, nsnarf);
    snarfversion++;	/* maybe modified by parent */
    runemove(snarf, w->r+w->q0, nsnarf);
    putsnarf();
}
/*e: function wsnarf */

/*s: function wcut */
void
wcut(Window *w)
{
    if(w->q1 == w->q0)
        return;
    wdelete(w, w->q0, w->q1);
    wsetselect(w, w->q0, w->q0);
}
/*e: function wcut */

/*s: function wpaste */
void
wpaste(Window *w)
{
    uint q0;

    if(nsnarf == 0)
        return;
    wcut(w);
    q0 = w->q0;
    if(w->rawing && q0==w->nr){
        waddraw(w, snarf, nsnarf);
        wsetselect(w, q0, q0);
    }else{
        q0 = winsert(w, snarf, nsnarf, w->q0);
        wsetselect(w, q0, q0+nsnarf);
    }
}
/*e: function wpaste */

//----------------------------------------------------------------------------
// Plumb
//----------------------------------------------------------------------------

/*s: function wplumb */
void
wplumb(Window *w)
{
    Plumbmsg *m;
    static int fd = -2;
    char buf[32];
    uint p0, p1;
    Cursor *c;

    if(fd == -2)
        fd = plumbopen("send", OWRITE|OCEXEC);
    if(fd < 0)
        return;
    m = emalloc(sizeof(Plumbmsg));
    m->src = estrdup("rio");
    m->dst = nil;
    m->wdir = estrdup(w->dir);
    m->type = estrdup("text");
    p0 = w->q0;
    p1 = w->q1;
    if(w->q1 > w->q0)
        m->attr = nil;
    else{
        while(p0>0 && w->r[p0-1]!=' ' && w->r[p0-1]!='\t' && w->r[p0-1]!='\n')
            p0--;
        while(p1<w->nr && w->r[p1]!=' ' && w->r[p1]!='\t' && w->r[p1]!='\n')
            p1++;
        sprint(buf, "click=%d", w->q0-p0);
        m->attr = plumbunpackattr(buf);
    }
    if(p1-p0 > messagesize-1024){
        plumbfree(m);
        return;	/* too large for 9P */
    }
    m->data = runetobyte(w->r+p0, p1-p0, &m->ndata);
    if(plumbsend(fd, m) < 0){
        c = lastcursor;
        riosetcursor(&query, 1);
        sleep(300);
        riosetcursor(c, 1);
    }
    plumbfree(m);
}
/*e: function wplumb */



//----------------------------------------------------------------------------
// Middle click
//----------------------------------------------------------------------------


/*s: function button2menu */
void
button2menu(Window *w)
{
    /*s: [[button2menu()]] return if window was deleted */
    if(w->deleted)
        return;
    /*e: [[button2menu()]] return if window was deleted */

    incref(w);
    /*s: [[button2menu()]] menu2str adjustments for scrolling */
    if(w->scrolling)
        menu2str[Scroll] = "noscroll";
    else
        menu2str[Scroll] = "scroll";
    /*e: [[button2menu()]] menu2str adjustments for scrolling */
    switch(menuhit(2, mousectl, &menu2, wscreen)){
    /*s: [[button2menu()]] cases */
    case Scroll:
        if(w->scrolling ^= 1)
            wshow(w, w->nr);
        break;
    /*x: [[button2menu()]] cases */
    case Cut:
        wsnarf(w);
        wcut(w);
        wscrdraw(w);
        break;

    case Snarf:
        wsnarf(w);
        break;

    case Paste:
        getsnarf();
        wpaste(w);
        wscrdraw(w);
        break;

    case Send:
        getsnarf();
        wsnarf(w);
        if(nsnarf == 0)
            break;
        if(w->rawing){
            waddraw(w, snarf, nsnarf);
            if(snarf[nsnarf-1]!='\n' && snarf[nsnarf-1]!='\004')
                          waddraw(w, L"\n", 1);
        }else{
            winsert(w, snarf, nsnarf, w->nr);
            if(snarf[nsnarf-1]!='\n' && snarf[nsnarf-1]!='\004')
                winsert(w, L"\n", 1, w->nr);
        }
        wsetselect(w, w->nr, w->nr);
        wshow(w, w->nr);
        break;
    /*x: [[button2menu()]] cases */
    case Plumb:
        wplumb(w);
        break;
    /*e: [[button2menu()]] cases */
    }
    wclose(w); // decref

    wsendctlmesg(w, Wakeup, ZR, nil);
    flushimage(display, true);
}
/*e: function button2menu */

//----------------------------------------------------------------------------
// Editor
//----------------------------------------------------------------------------

/*s: function wbswidth */
int
wbswidth(Window *w, Rune c)
{
    uint q, eq, stop;
    Rune r;
    int skipping;

    /* there is known to be at least one character to erase */
    if(c == 0x08)	/* ^H: erase character */
        return 1;
    q = w->q0;
    stop = 0;
    if(q > w->qh)
        stop = w->qh;
    skipping = true;
    while(q > stop){
        r = w->r[q-1];
        if(r == '\n'){		/* eat at most one more character */
            if(q == w->q0)	/* eat the newline */
                --q;
            break; 
        }
        if(c == 0x17){
            eq = isalnum(r);
            if(eq && skipping)	/* found one; stop skipping */
                skipping = false;
            else if(!eq && !skipping)
                break;
        }
        --q;
    }
    return w->q0-q;
}
/*e: function wbswidth */

/*s: function wfill */
void
wfill(Window *w)
{
    Rune *rp;
    int i, n, m, nl;

    if(w->lastlinefull)
        return;
    rp = malloc(messagesize);
    do{
        n = w->nr-(w->org+w->nchars);
        if(n == 0)
            break;
        if(n > 2000)	/* educated guess at reasonable amount */
            n = 2000;
        runemove(rp, w->r+(w->org+w->nchars), n);
        /*
         * it's expensive to frinsert more than we need, so
         * count newlines.
         */
        nl = w->maxlines-w->nlines;
        m = 0;
        for(i=0; i<n; ){
            if(rp[i++] == '\n'){
                m++;
                if(m >= nl)
                    break;
            }
        }
        frinsert(w, rp, rp+i, w->nchars);
    } while(w->lastlinefull == false);
    free(rp);
}
/*e: function wfill */


/*s: function wdelete */
void
wdelete(Window *w, uint q0, uint q1)
{
    uint n, p0, p1;

    n = q1-q0;
    if(n == 0)
        return;
    runemove(w->r+q0, w->r+q1, w->nr-q1);
    w->nr -= n;
    if(q0 < w->q0)
        w->q0 -= min(n, w->q0-q0);
    if(q0 < w->q1)
        w->q1 -= min(n, w->q1-q0);
    if(q1 < w->qh)
        w->qh -= n;
    else if(q0 < w->qh)
        w->qh = q0;
    if(q1 <= w->org)
        w->org -= n;
    else if(q0 < w->org+w->nchars){
        p1 = q1 - w->org;
        if(p1 > w->nchars)
            p1 = w->nchars;
        if(q0 < w->org){
            w->org = q0;
            p0 = 0;
        }else
            p0 = q0 - w->org;
        frdelete(w, p0, p1);
        wfill(w);
    }
}
/*e: function wdelete */

/*s: function wbacknl */
uint
wbacknl(Window *w, uint p, uint n)
{
    int i, j;

    /* look for start of this line if n==0 */
    if(n==0 && p>0 && w->r[p-1]!='\n')
        n = 1;
    i = n;
    while(i-->0 && p>0){
        --p;	/* it's at a newline now; back over it */
        if(p == 0)
            break;
        /* at 128 chars, call it a line anyway */
        for(j=128; --j>0 && p>0; p--)
            if(w->r[p-1]=='\n')
                break;
    }
    return p;
}
/*e: function wbacknl */


/*s: function wshow */
void
wshow(Window *w, uint q0)
{
    int qe;
    int nl;
    uint q;

    qe = w->org + w->nchars;
    if(w->org <= q0 && (q0 < qe || (q0 == qe && qe == w->nr)))
        wscrdraw(w);
    /*s: [[wshow()]] else, when q0 is out of scope */
    else{
        nl = 4 * w->maxlines / 5;
        q = wbacknl(w, q0, nl);
        /* avoid going backwards if trying to go forwards - long lines! */
        if(!(q0 > w->org && q < w->org))
            wsetorigin(w, q, true);
        while(q0 > w->org + w->nchars)
            wsetorigin(w, w->org+1, false);
    }
    /*e: [[wshow()]] else, when q0 is out of scope */
}
/*e: function wshow */

/*s: function wsetorigin */
void
wsetorigin(Window *w, uint org, int exact)
{
    int i, a, fixup;
    Rune *r;
    uint n;

    if(org>0 && !exact){
        /* org is an estimate of the char posn; find a newline */
        /* don't try harder than 256 chars */
        for(i=0; i<256 && org<w->nr; i++){
            if(w->r[org] == '\n'){
                org++;
                break;
            }
            org++;
        }
    }
    a = org - w->org;
    fixup = 0;
    if(a>=0 && a<w->nchars){
        frdelete(w, 0, a);
        fixup = 1;	/* frdelete can leave end of last line in wrong selection mode; it doesn't know what follows */
    }else if(a<0 && -a<w->nchars){
        n = w->org - org;
        r = runemalloc(n);
        runemove(r, w->r+org, n);
        frinsert(w, r, r+n, 0);
        free(r);
    }else
        frdelete(w, 0, w->nchars);
    w->org = org;
    wfill(w);
    wscrdraw(w);
    wsetselect(w, w->q0, w->q1);
    if(fixup && w->p1 > w->p0)
        frdrawsel(w, frptofchar(w, w->p1-1), w->p1-1, w->p1, 1);
}
/*e: function wsetorigin */

/*s: function wsetselect */
void
wsetselect(Window *w, uint q0, uint q1)
{
    int p0, p1;

    /* w->p0 and w->p1 are always right; w->q0 and w->q1 may be off */
    w->q0 = q0;
    w->q1 = q1;
    /* compute desired p0,p1 from q0,q1 */
    p0 = q0-w->org;
    p1 = q1-w->org;
    if(p0 < 0)
        p0 = 0;
    if(p1 < 0)
        p1 = 0;
    if(p0 > w->nchars)
        p0 = w->nchars;
    if(p1 > w->nchars)
        p1 = w->nchars;
    if(p0==w->p0 && p1==w->p1)
        return;

    /* screen disagrees with desired selection */
    if(w->p1<=p0 || p1<=w->p0 || p0==p1 || w->p1==w->p0){
        /* no overlap or too easy to bother trying */
        frdrawsel(w, frptofchar(w, w->p0), w->p0, w->p1, 0);
        frdrawsel(w, frptofchar(w, p0), p0, p1, 1);
        goto Return;
    }
    /* overlap; avoid unnecessary painting */
    if(p0 < w->p0){
        /* extend selection backwards */
        frdrawsel(w, frptofchar(w, p0), p0, w->p0, 1);
    }else if(p0 > w->p0){
        /* trim first part of selection */
        frdrawsel(w, frptofchar(w, w->p0), w->p0, p0, 0);
    }
    if(p1 > w->p1){
        /* extend selection forwards */
        frdrawsel(w, frptofchar(w, w->p1), w->p1, p1, 1);
    }else if(p1 < w->p1){
        /* trim last part of selection */
        frdrawsel(w, frptofchar(w, p1), p1, w->p1, 0);
    }

    Return:
    w->p0 = p0;
    w->p1 = p1;
}
/*e: function wsetselect */

/*s: function winsert */
uint
winsert(Window *w, Rune *r, int n, uint q0)
{
    uint m;

    if(n == 0)
        return q0;
    /*s: [[winsert()]] if size of rune array is getting really big */
    if(w->nr + n > HiWater && q0 >= w->org && q0 >= w->qh){
        m = min(HiWater-LoWater, min(w->org, w->qh));
        w->org -= m;
        w->qh -= m;
        if(w->q0 > m)
            w->q0 -= m;
        else
            w->q0 = 0;
        if(w->q1 > m)
            w->q1 -= m;
        else
            w->q1 = 0;
        w->nr -= m;
        runemove(w->r, w->r+m, w->nr);
        q0 -= m;
    }
    /*e: [[winsert()]] if size of rune array is getting really big */
    /*s: [[winsert()]] grow rune array if reach maxr */
    if(w->nr+n > w->maxr){
        /*
         * Minimize realloc breakage:
         *	Allocate at least MinWater
         * 	Double allocation size each time
         *	But don't go much above HiWater
         */
        m = max(min(2*(w->nr+n), HiWater), w->nr+n)+MinWater;
        if(m > HiWater)
            m = max(HiWater+MinWater, w->nr+n);
        if(m > w->maxr){
            w->r = runerealloc(w->r, m);
            w->maxr = m;
        }
    }
    /*e: [[winsert()]] grow rune array if reach maxr */

    // move to the right the runes after the cursor q0 to make some space
    runemove(w->r + q0 + n, w->r + q0, w->nr - q0);
    // fill the space
    runemove(w->r + q0, r, n);
    w->nr += n;

    /* if output touches, advance selection, not qh; works best for keyboard and output */
    if(q0 <= w->q0)
        w->q0 += n; // move the q0 cursor
    if(q0 <= w->q1)
        w->q1 += n;
    if(q0 < w->qh)
        w->qh += n;

    /*s: [[winsert()]] update visible text */
    if(q0 < w->org)
        w->org += n;
    else if(q0 <= w->org + w->nchars)
        frinsert(w, r, r+n, q0 - w->org); // echo back
    /*e: [[winsert()]] update visible text */
    return q0;
}
/*e: function winsert */


/*s: function wcontents */
char*
wcontents(Window *w, int *ip)
{
    return runetobyte(w->r, w->nr, ip);
}
/*e: function wcontents */


//----------------------------------------------------------------------------
// Mouse dispatch
//----------------------------------------------------------------------------

/*s: function wmousectl */
void
wmousectl(Window *w)
{
    int but;

    if(w->mc.buttons == 1)
        but = 1;
    else if(w->mc.buttons == 2)
        but = 2;
    else if(w->mc.buttons == 4)
        but = 3;
    else{
        if(w->mc.buttons == 8)
            wkeyctl(w, Kscrolloneup);
        if(w->mc.buttons == 16)
            wkeyctl(w, Kscrollonedown);
        return;
    }

    incref(w);		/* hold up window while we track */
    /*s: [[wmousectl()]] goto Return if window was deleted */
    if(w->deleted)
        goto Return;
    /*e: [[wmousectl()]] goto Return if window was deleted */

    /*s: [[wmousectl()]] if pt in scrollbar */
    if(ptinrect(w->mc.xy, w->scrollr)){
        if(but)
            wscroll(w, but);
        goto Return;
    }
    /*e: [[wmousectl()]] if pt in scrollbar */
    if(but == 1)
        wselect(w);

    /* else all is handled by main process */
   Return:
    wclose(w);
}
/*e: function wmousectl */

//----------------------------------------------------------------------------
// Key dispatch
//----------------------------------------------------------------------------


/*s: function interruptproc */
/*
 * Need to do this in a separate proc because if process we're interrupting
 * is dying and trying to print tombstone, kernel is blocked holding p->debug lock.
 */
void
interruptproc(void *v)
{
    int *notefd;

    notefd = v;
    write(*notefd, "interrupt", 9);
    free(notefd);
}
/*e: function interruptproc */



/*s: function wkeyctl */
void
wkeyctl(Window *w, Rune r)
{
    /*s: [[wkeyctl()]] locals */
    uint q0;
    /*x: [[wkeyctl()]] locals */
    uint q1;
    int n, nb;
    /*x: [[wkeyctl()]] locals */
    int nr;
    Rune *rp;
    int *notefd;
    /*e: [[wkeyctl()]] locals */

    if(r == 0)
        return;
    /*s: [[wkeyctl()]] return if window was deleted */
    if(w->deleted)
        return;
    /*e: [[wkeyctl()]] return if window was deleted */

    /* navigation keys work only when mouse is not open */
    /*s: [[wkeyctl()]] when mouse not opened and navigation keys */
    if(!w->mouseopen)
    switch(r){
    /*s: [[wkeyctl()]] when mouse not opened, switch key cases */
    case Khome:
        wshow(w, 0);
        return;
    /*x: [[wkeyctl()]] when mouse not opened, switch key cases */
    case Kend:
        wshow(w, w->nr);
        return;
    /*x: [[wkeyctl()]] when mouse not opened, switch key cases */
    case Kdown:
        n = w->maxlines / 3;
        goto case_Down;
    /*x: [[wkeyctl()]] when mouse not opened, switch key cases */
    case Kscrollonedown:
        n = mousescrollsize(w->maxlines);
        if(n <= 0)
            n = 1;
        goto case_Down;
    /*x: [[wkeyctl()]] when mouse not opened, switch key cases */
    case Kpgdown:
        n = 2 * w->maxlines / 3;
        // Fallthrough
    case_Down:
        q0 = w->org +
            frcharofpt(w, Pt(w->Frame.r.min.x, 
                             w->Frame.r.min.y + n * w->font->height));
        wsetorigin(w, q0, true);
        return;
    /*x: [[wkeyctl()]] when mouse not opened, switch key cases */
    case Kup:
        n = w->maxlines/3;
        goto case_Up;
    /*x: [[wkeyctl()]] when mouse not opened, switch key cases */
    case Kscrolloneup:
        n = mousescrollsize(w->maxlines);
        if(n <= 0)
            n = 1;
        goto case_Up;
    /*x: [[wkeyctl()]] when mouse not opened, switch key cases */
    case Kpgup:
        n = 2*w->maxlines/3;
        // Fallthrough
    case_Up:
        q0 = wbacknl(w, w->org, n);
        wsetorigin(w, q0, true);
        return;
    /*x: [[wkeyctl()]] when mouse not opened, switch key cases */
    case Kleft:
        if(w->q0 > 0){
            q0 = w->q0 - 1;
            wsetselect(w, q0, q0);
            wshow(w, q0);
        }
        return;
    /*x: [[wkeyctl()]] when mouse not opened, switch key cases */
    case Kright:
        if(w->q1 < w->nr){
            q1 = w->q1+1;
            wsetselect(w, q1, q1);
            wshow(w, q1);
        }
        return;
    /*x: [[wkeyctl()]] when mouse not opened, switch key cases */
    case 0x05:	/* ^E: end of line */
        q0 = w->q0;
        while(q0 < w->nr && w->r[q0] != '\n')
            q0++;
        wsetselect(w, q0, q0);
        wshow(w, w->q0);
        return;
    /*x: [[wkeyctl()]] when mouse not opened, switch key cases */
    case 0x01:	/* ^A: beginning of line */
        if(w->q0==0 || w->q0 == w->qh || w->r[w->q0 - 1]=='\n')
            return;
        nb = wbswidth(w, 0x15 /* ^U */);
        wsetselect(w, w->q0 - nb, w->q0 - nb);
        wshow(w, w->q0);
        return;
    /*e: [[wkeyctl()]] when mouse not opened, switch key cases */
    default:
        ; // no return! fallthrough
    }
    /*e: [[wkeyctl()]] when mouse not opened and navigation keys */

    /*s: [[wkeyctl()]] if rawing */
    if(w->rawing && (w->mouseopen || w->q0 == w->nr)){
        waddraw(w, &r, 1);
        return;
    }
    /*e: [[wkeyctl()]] if rawing */
    /*s: [[wkeyctl()]] if holding */
    if(r==0x1B || (w->holding && r==0x7F)){	/* toggle hold */
        if(w->holding)
            --w->holding;
        else
            w->holding++;
        wrepaint(w);
        if(r == 0x1B)
            return;
    }
    /*e: [[wkeyctl()]] if holding */

    /*s: [[wkeyctl()]] when not rawing */
    // here when no navigation key, no rawing, no 0x1B holding

    /*s: [[wkeyctl()]] snarf and cut if not interrupt key */
    if(r != 0x7F){ // 0x7F = Interrupt key
        wsnarf(w);
        wcut(w);
    }
    /*e: [[wkeyctl()]] snarf and cut if not interrupt key */
    switch(r){
    /*s: [[wkeyctl()]] special key cases and no special mode */
    case 0x08:	/* ^H: erase character */
    case 0x15:	/* ^U: erase line */
    case 0x17:	/* ^W: erase word */
        if(w->q0==0 || w->q0==w->qh)
            return;
        nb = wbswidth(w, r);
        q1 = w->q0;
        q0 = q1-nb;
        if(q0 < w->org){
            q0 = w->org;
            nb = q1-q0;
        }
        if(nb > 0){
            wdelete(w, q0, q0+nb);
            wsetselect(w, q0, q0);
        }
        return;
    /*x: [[wkeyctl()]] special key cases and no special mode */
    case 0x7F:		/* send interrupt */
        w->qh = w->nr;
        wshow(w, w->qh);
        notefd = emalloc(sizeof(int));
        *notefd = w->notefd;
        proccreate(interruptproc, notefd, 4096);
        return;
    /*x: [[wkeyctl()]] special key cases and no special mode */
    case 0x06:	/* ^F: file name completion */
    case Kins:		/* Insert: file name completion */
        rp = namecomplete(w);
        if(rp == nil)
            return;
        nr = runestrlen(rp);
        q0 = w->q0;
        q0 = winsert(w, rp, nr, q0);
        wshow(w, q0+nr);
        free(rp);
        return;
    /*e: [[wkeyctl()]] special key cases and no special mode */
    }
    // else

    /* otherwise ordinary character; just insert */
    /*s: [[wkeyctl()]] ordinary character */
    q0 = w->q0;
    q0 = winsert(w, &r, 1, q0);
    wshow(w, q0+1);
    /*e: [[wkeyctl()]] ordinary character */
    /*e: [[wkeyctl()]] when not rawing */
}
/*e: function wkeyctl */

/*e: windows/rio/terminal.c */
