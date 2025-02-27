/*s: windows/libframe/frbox.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>
#include <mouse.h>
#include <frame.h>

/*s: constant [[SLOP]] */
#define	SLOP	25
/*e: constant [[SLOP]] */

/*s: function [[_fraddbox]] */
void
_fraddbox(Frame *f, int bn, int n)	/* add n boxes after bn, shift the rest up,
                 * box[bn+n]==box[bn] */
{
    int i;

    /*s: [[_fraddbox()]] sanity check bn */
    if(bn > f->nbox)
        drawerror(f->display, "_fraddbox");
    /*e: [[_fraddbox()]] sanity check bn */
    if(f->nbox+n > f->nalloc)
        _frgrowbox(f, n+SLOP);
    for(i=f->nbox; --i>=bn; )
        f->box[i+n] = f->box[i];
    f->nbox+=n;
}
/*e: function [[_fraddbox]] */

/*s: function [[_frclosebox]] */
void
_frclosebox(Frame *f, int n0, int n1)	/* inclusive */
{
    int i;

    /*s: [[_frclosebox()]] sanity check n0 and n1 */
    if(n0>=f->nbox || n1>=f->nbox || n1<n0)
        drawerror(f->display, "_frclosebox");
    /*e: [[_frclosebox()]] sanity check n0 and n1 */
    n1++;
    for(i=n1; i<f->nbox; i++)
        f->box[i-(n1-n0)] = f->box[i];
    f->nbox -= n1-n0;
}
/*e: function [[_frclosebox]] */

/*s: function [[_frdelbox]] */
void
_frdelbox(Frame *f, int n0, int n1)	/* inclusive */
{
    /*s: [[_frdelbox()]] sanity check n0 and n1 */
    if(n0>=f->nbox || n1>=f->nbox || n1<n0)
        drawerror(f->display, "_frdelbox");
    /*e: [[_frdelbox()]] sanity check n0 and n1 */
    _frfreebox(f, n0, n1);
    _frclosebox(f, n0, n1);
}
/*e: function [[_frdelbox]] */

/*s: function [[_frfreebox]] */
void
_frfreebox(Frame *f, int n0, int n1)	/* inclusive */
{
    int i;

    /*s: [[_frfreebox()]] sanity check n0 and n1 */
    if(n1<n0)
        return;
    if(n0>=f->nbox || n1>=f->nbox)
        drawerror(f->display, "_frfreebox");
    /*e: [[_frfreebox()]] sanity check n0 and n1 */
    n1++;
    for(i=n0; i<n1; i++)
        if(f->box[i].nrune >= 0)
            free(f->box[i].ptr);
}
/*e: function [[_frfreebox]] */

/*s: function [[_frgrowbox]] */
void
_frgrowbox(Frame *f, int delta)
{
    f->nalloc += delta;
    f->box = realloc(f->box, f->nalloc*sizeof(Frbox));
    /*s: [[_frgrowbox()]] sanity check box */
    if(f->box == nil)
        drawerror(f->display, "_frgrowbox");
    /*e: [[_frgrowbox()]] sanity check box */
}
/*e: function [[_frgrowbox]] */

/*s: function [[dupbox]] */
static
void
dupbox(Frame *f, int bn)
{
    uchar *p;

    if(f->box[bn].nrune < 0)
        drawerror(f->display, "dupbox");
    _fraddbox(f, bn, 1);
    if(f->box[bn].nrune >= 0){
        p = _frallocstr(f, NBYTE(&f->box[bn])+1);
        strcpy((char*)p, (char*)f->box[bn].ptr);
        f->box[bn+1].ptr = p;
    }
}
/*e: function [[dupbox]] */

/*s: function [[runeindex]] */
static
uchar*
runeindex(uchar *p, int n)
{
    int i, w;
    Rune rune;

    for(i=0; i<n; i++,p+=w)
        if(*p < Runeself)
            w = 1;
        else{
            w = chartorune(&rune, (char*)p);
            USED(rune);
        }
    return p;
}
/*e: function [[runeindex]] */

/*s: function [[truncatebox]] */
static
void
truncatebox(Frame *f, Frbox *b, int n)	/* drop last n chars; no allocation done */
{
    if(b->nrune<0 || b->nrune<n)
        drawerror(f->display, "truncatebox");
    b->nrune -= n;
    runeindex(b->ptr, b->nrune)[0] = 0;
    b->wid = stringwidth(f->font, (char *)b->ptr);
}
/*e: function [[truncatebox]] */

/*s: function [[chopbox]] */
static
void
chopbox(Frame *f, Frbox *b, int n)	/* drop first n chars; no allocation done */
{
    char *p;

    if(b->nrune<0 || b->nrune<n)
        drawerror(f->display, "chopbox");
    p = (char*)runeindex(b->ptr, n);
    memmove((char*)b->ptr, p, strlen(p)+1);
    b->nrune -= n;
    b->wid = stringwidth(f->font, (char *)b->ptr);
}
/*e: function [[chopbox]] */

/*s: function [[_frsplitbox]] */
void
_frsplitbox(Frame *f, int bn, int n)
{
    dupbox(f, bn);
    truncatebox(f, &f->box[bn], f->box[bn].nrune-n);
    chopbox(f, &f->box[bn+1], n);
}
/*e: function [[_frsplitbox]] */

/*s: function [[_frmergebox]] */
void
_frmergebox(Frame *f, int bn)		/* merge bn and bn+1 */
{
    Frbox *b;

    b = &f->box[bn];
    _frinsure(f, bn, NBYTE(&b[0])+NBYTE(&b[1])+1);
    strcpy((char*)runeindex(b[0].ptr, b[0].nrune), (char*)b[1].ptr);
    b[0].wid += b[1].wid;
    b[0].nrune += b[1].nrune;
    _frdelbox(f, bn+1, bn+1);
}
/*e: function [[_frmergebox]] */

/*s: function [[_frfindbox]] */
/* find box containing q and put q on a box boundary */
int
_frfindbox(Frame *f, int bn, ulong p, ulong q)	
{
    Frbox *b;

    for(b = &f->box[bn]; bn < f->nbox && p+NRUNE(b) <= q; bn++, b++)
        p += NRUNE(b);
    if(p != q)
        _frsplitbox(f, bn++, (int)(q-p));
    return bn;
}
/*e: function [[_frfindbox]] */
/*e: windows/libframe/frbox.c */
