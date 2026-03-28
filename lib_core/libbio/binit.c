/*s: libbio/binit.c */
/*s: libbio includes */
#include	<u.h>
#include	<libc.h>
#include	<bio.h>
/*e: libbio includes */
/*s: global [[wbufs]] */
static	Biobufhdr*	wbufs[20];
/*e: global [[wbufs]] */
/*s: global [[atexitflag]] */
static	bool		atexitflag;
/*e: global [[atexitflag]] */

/*s: function [[batexit]] */
static
void
batexit(void)
{
    Biobufhdr *bp;
    int i;

    for(i=0; i<nelem(wbufs); i++) {
        bp = wbufs[i];
        if(bp != nil) {
            wbufs[i] = nil;
            Bflush(bp);
        }
    }
}
/*e: function [[batexit]] */

/*s: function [[deinstall]] */
static
void
deinstall(Biobufhdr *bp)
{
    int i;

    for(i=0; i<nelem(wbufs); i++)
        if(wbufs[i] == bp)
            wbufs[i] = nil;
}
/*e: function [[deinstall]] */
/*s: function [[install]] */
static
void
install(Biobufhdr *bp)
{
    int i;

    deinstall(bp);
    for(i=0; i<nelem(wbufs); i++)
        if(wbufs[i] == nil) {
            wbufs[i] = bp;
            break;
        }
    if(atexitflag == false) {
        atexitflag = true;
        atexit(batexit);
    }
}
/*e: function [[install]] */

/*s: function [[Binits]] */
int
Binits(Biobufhdr *bp, fdt f, int mode, uchar *p, int size)
{

    p += Bungetsize;	/* make room for Bungets */
    size -= Bungetsize;

    switch(mode&~(OCEXEC|ORCLOSE|OTRUNC)) {
    case OREAD:
        bp->state = Bractive;
        bp->ocount = 0;
        break;
    case OWRITE:
        install(bp);
        bp->state = Bwactive;
        bp->ocount = -size;
        break;
    default:
        fprint(2, "Binits: unknown mode %d\n", mode);
        return Beof;

    }
    bp->bbuf = p;
    bp->ebuf = p+size;
    bp->bsize = size;
    bp->icount = 0;
    bp->gbuf = bp->ebuf;
    bp->fid = f;
    bp->flag = 0;
    bp->rdline = 0;
    bp->offset = 0;
    bp->runesize = 0;
    return 0;
}
/*e: function [[Binits]] */
/*s: function [[Binit]] */
int
Binit(Biobuf *bp, fdt f, int mode)
{
    return Binits(bp, f, mode, bp->b, sizeof(bp->b));
}
/*e: function [[Binit]] */
/*s: function [[Bopen]] */
Biobuf*
Bopen(char *name, int mode)
{
    Biobuf *bp;
    fdt f;

    switch(mode&~(OCEXEC|ORCLOSE|OTRUNC)) {
    case OREAD:
        f = open(name, mode);
        break;
    case OWRITE:
        f = create(name, mode, 0666);
        break;
    default:
        fprint(2, "Bopen: unknown mode %#x\n", mode);
        return 0;
    }
    if(f < 0)
        return nil;

    bp = malloc(sizeof(Biobuf));
    Binits(bp, f, mode, bp->b, sizeof(bp->b));
    bp->flag = Bmagic;			/* mark bp open & malloced */
    return bp;
}
/*e: function [[Bopen]] */

/*s: function [[Bterm]] */
int
Bterm(Biobufhdr *bp)
{
    int r;

    deinstall(bp);
    r = Bflush(bp);
    if(bp->flag == Bmagic) {
        bp->flag = 0;
        close(bp->fid);
        bp->fid = -1;			/* prevent accidents */
        free(bp);
    }
    /* otherwise opened with Binit(s) */
    return r;
}
/*e: function [[Bterm]] */
/*e: libbio/binit.c */
