/*s: libbio/bgetd.c */
#include <u.h>
#include <libc.h>
#include <bio.h>

struct	bgetd
{
    Biobufhdr*	b;
    int		eof;
};

/*s: function [[Bgetdf]] */
static int
Bgetdf(void *vp)
{
    int c;
    struct bgetd *bg = vp;

    c = Bgetc(bg->b);
    if(c == Beof)
        bg->eof = 1;
    return c;
}
/*e: function [[Bgetdf]] */

/*s: function [[Bgetd]] */
int
Bgetd(Biobufhdr *bp, double *dp)
{
    double d;
    struct bgetd b;

    b.b = bp;
    b.eof = 0;
    d = charstod(Bgetdf, &b);
    if(b.eof)
        return -1;
    Bungetc(bp);
    *dp = d;
    return 1;
}
/*e: function [[Bgetd]] */
/*e: libbio/bgetd.c */
