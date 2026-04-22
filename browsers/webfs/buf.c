/*s: webfs/buf.c */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ip.h>
#include <plumb.h>
#include <thread.h>
#include <fcall.h>
#include <9p.h>
#include "dat.h"
#include "fns.h"

/*s: function [[initibuf]](webfs) */
void
initibuf(Ibuf *b, Ioproc *io, int fd)
{
    b->fd = fd;
    b->io = io;
    b->rp = b->wp = b->buf;
}
/*e: function [[initibuf]](webfs) */

/*s: function [[readibuf]](webfs) */
int
readibuf(Ibuf *b, char *buf, int len)
{
    int n;

    n = b->wp - b->rp;
    if(n > 0){
        if(n > len)
            n = len;
        memmove(buf, b->rp, n);
        b->rp += n;
        return n;
    }
    return ioreadn(b->io, b->fd, buf, len);
}
/*e: function [[readibuf]](webfs) */

/*s: function [[unreadline]](webfs) */
void
unreadline(Ibuf *b, char *line)
{
    int i, n;

    i = strlen(line);
    n = b->wp - b->rp;
    memmove(&b->buf[i+1], b->rp, n);
    memmove(b->buf, line, i);
    b->buf[i] = '\n';
    b->rp = b->buf;
    b->wp = b->rp+i+1+n;
}
/*e: function [[unreadline]](webfs) */

/*s: function [[readline]](webfs) */
int
readline(Ibuf *b, char *buf, int len)
{
    int n;
    char *p;

    len--;

    for(p = buf;;){
        if(b->rp >= b->wp){
            /* claude: reset on drain -- same upstream ring-buffer bug as
             * hget's readline. wp only advances; without resetting to
             * b->buf after the ring drains, cumulative reads push wp
             * past b->buf+sizeof(b->buf) and the next read() writes off
             * the end of the allocation. Fat-header sites
             * (en.wikipedia.org etc.) trip it. unreadline() already
             * re-anchors rp/wp at b->buf when it fires, so the two
             * mutators stay consistent. */
            b->rp = b->wp = b->buf;
            n = ioread(b->io, b->fd, b->wp, sizeof(b->buf)/2);
            if(n < 0)
                return -1;
            if(n == 0)
                break;
            b->wp += n;
        }
        n = *b->rp++;
        if(len > 0){
            *p++ = n;
            len--;
        }
        if(n == '\n')
            break;
    }

    /* drop trailing white */
    for(;;){
        if(p <= buf)
            break;
        n = *(p-1);
        if(n != ' ' && n != '\t' && n != '\r' && n != '\n')
            break;
        p--;
    }

    *p = 0;
    return p-buf;
}
/*e: function [[readline]](webfs) */

/*e: webfs/buf.c */
