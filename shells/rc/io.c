/*s: rc/io.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

/*s: enum [[MiscConstants]] */
enum { Stralloc = 100, };
/*e: enum [[MiscConstants]] */

// forward decls
int emptybuf(io*);
int fullbuf(io*, int);

/*s: function [[rchr]] */
int
rchr(io *b)
{
    if(b->bufp==b->ebuf)
        return emptybuf(b);
    return *b->bufp++;
}
/*e: function [[rchr]] */

/*s: function [[rutf]] */
int
rutf(io *b, char *buf, Rune *r)
{
    int n, i, c;

    c = rchr(b);
    if(c == EOF)
        return EOF;
    *buf = c;
    if(c < Runesync){
        *r = c;
        return 1;
    }
    for(i = 1; (c = rchr(b)) != EOF; ){
        buf[i++] = c;
        buf[i] = 0;
        if(fullrune(buf, i)){
            n = chartorune(r, buf);
            b->bufp -= i - n;	/* push back unconsumed bytes */
            assert(b->fd == -1 || b->bufp > b->buf);
            return n;
        }
    }
    /* at eof */
    b->bufp -= i - 1;			/* consume 1 byte */
    *r = Runeerror;
    return runetochar(buf, r);
}
/*e: function [[rutf]] */

/*s: function [[fullbuf]] */
int
fullbuf(io *f, int c)
{
    flush(f);
    return *f->bufp++=c;
}
/*e: function [[fullbuf]] */

/*s: function [[flush]] */
void
flush(io *f)
{
    int n;

    if(f->strp){
        n = f->ebuf - f->strp;
        f->strp = realloc(f->strp, n+Stralloc+1);
        if(f->strp==0)
            panic("Can't realloc %d bytes in flush!", n+Stralloc+1);
        f->bufp = f->strp + n;
        f->ebuf = f->bufp + Stralloc;
        memset(f->bufp, '\0', Stralloc+1);
    }
    else{
        n = f->bufp-f->buf;
        if(n && Write(f->fd, f->buf, n) != n){
            Write(2, "Write error\n", 12);
            if(ntrap)
                dotrap();
        }
        f->bufp = f->buf;
        f->ebuf = f->buf+NBUF;
    }
}
/*e: function [[flush]] */

/*s: function [[openfd]] */
io*
openfd(fdt fd)
{
    io *f = new(struct Io);
    f->fd = fd;
    f->bufp = f->ebuf = f->buf;
    f->strp = nil;
    return f;
}
/*e: function [[openfd]] */

/*s: function [[openstr]] */
io*
openstr(void)
{
    io *f = new(struct Io);

    f->fd = -1;
    f->bufp = f->strp = emalloc(Stralloc+1);
    f->ebuf = f->bufp + Stralloc;
    memset(f->bufp, '\0', Stralloc+1);
    return f;
}
/*e: function [[openstr]] */

/*s: function [[opencore]] */
/*
 * Open a corebuffer to read.  EOF occurs after reading len
 * characters from buf.
 */

io*
opencore(char *s, int len)
{
    io *f = new(struct Io);
    uchar *buf = emalloc(len);

    f->fd = -1 /*open("/dev/null", 0)*/;
    f->bufp = f->strp = buf;
    f->ebuf = buf+len;
    Memcpy(buf, s, len);
    return f;
}
/*e: function [[opencore]] */

/*s: function [[closeio]] */
void
closeio(io *io)
{
    if(io->fd>=0)
        close(io->fd);
    if(io->strp)
        efree(io->strp);
    efree(io);
}
/*e: function [[closeio]] */

/*s: function [[emptybuf]] */
int
emptybuf(io *f)
{
    int n;
    if(f->fd==-1 || (n = Read(f->fd, f->buf, NBUF))<=0) return EOF;
    f->bufp = f->buf;
    f->ebuf = f->buf + n;
    return *f->bufp++;
}
/*e: function [[emptybuf]] */
/*e: rc/io.c */
