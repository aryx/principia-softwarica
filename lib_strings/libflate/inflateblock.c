/*s: libflate/inflateblock.c */
/*s: libflate includes */
#include <u.h>
#include <libc.h>
#include <flate.h>
/*e: libflate includes */
typedef struct Block	Block;

struct Block
{
    uchar	*pos;
    uchar	*limit;
};

/*s: function [[blgetc]] */
static int
blgetc(void *vb)
{
    Block *b;

    b = vb;
    if(b->pos >= b->limit)
        return -1;
    return *b->pos++;
}
/*e: function [[blgetc]] */

/*s: function [[blwrite (libflate/inflateblock.c)]] */
static int
blwrite(void *vb, void *buf, int n)
{
    Block *b;

    b = vb;

    if(n > b->limit - b->pos)
        n = b->limit - b->pos;
    memmove(b->pos, buf, n);
    b->pos += n;
    return n;
}
/*e: function [[blwrite (libflate/inflateblock.c)]] */

/*s: function [[inflateblock]] */
int
inflateblock(uchar *dst, int dsize, uchar *src, int ssize)
{
    Block bd, bs;
    int ok;

    bs.pos = src;
    bs.limit = src + ssize;

    bd.pos = dst;
    bd.limit = dst + dsize;

    ok = inflate(&bd, blwrite, &bs, blgetc);
    if(ok != FlateOk)
        return ok;
    return bd.pos - dst;
}
/*e: function [[inflateblock]] */
/*e: libflate/inflateblock.c */
