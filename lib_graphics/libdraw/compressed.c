/*s: lib_graphics/libdraw/compressed.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>

/*s: function _twiddlecompressed */
/*
 * compressed data are seuences of byte codes.  
 * if the first byte b has the 0x80 bit set, the next (b^0x80)+1 bytes
 * are data.  otherwise, it's two bytes specifying a previous string to repeat.
 */
void
_twiddlecompressed(uchar *buf, int n)
{
    uchar *ebuf;
    int j, k, c;

    ebuf = buf+n;
    while(buf < ebuf){
        c = *buf++;
        if(c >= 128){
            k = c-128+1;
            for(j=0; j<k; j++, buf++)
                *buf ^= 0xFF;
        }else
            buf++;
    }
}
/*e: function _twiddlecompressed */

/*s: function _compblocksize */
int
_compblocksize(Rectangle r, int depth)
{
    int bpl;

    bpl = bytesperline(r, depth);
    bpl = 2*bpl;	/* add plenty extra for blocking, etc. */
    if(bpl < NCBLOCK)
        return NCBLOCK;
    return bpl;
}
/*e: function _compblocksize */

/*s: function cloadimage */
int
cloadimage(Image *i, Rectangle r, uchar *data, int ndata)
{
    int m, nb, miny, maxy, ncblock;
    uchar *a;

    if(!rectinrect(r, i->r)){
        werrstr("cloadimage: bad rectangle");
        return -1;
    }

    miny = r.min.y;
    m = 0;
    ncblock = _compblocksize(r, i->depth);
    while(miny != r.max.y){
        maxy = atoi((char*)data+0*12);
        nb = atoi((char*)data+1*12);
        if(maxy<=miny || r.max.y<maxy){
            werrstr("creadimage: bad maxy %d", maxy);
            return -1;
        }
        data += 2*12;
        ndata -= 2*12;
        m += 2*12;
        if(nb<=0 || ncblock<nb || nb>ndata){
            werrstr("creadimage: bad count %d", nb);
            return -1;
        }
        a = bufimage(i->display, 21+nb);
        if(a == nil)
            return -1;
        a[0] = 'Y';
        BPLONG(a+1, i->id);
        BPLONG(a+5, r.min.x);
        BPLONG(a+9, miny);
        BPLONG(a+13, r.max.x);
        BPLONG(a+17, maxy);
        memmove(a+21, data, nb);
        miny = maxy;
        data += nb;
        ndata += nb;
        m += nb;
    }
    return m;
}
/*e: function cloadimage */

/*s: function creadimage */
Image *
creadimage(Display *d, int fd, int dolock)
{
    char hdr[5*12+1];
    Rectangle r;
    int m, nb, miny, maxy, new, ldepth, ncblock;
    uchar *buf, *a;
    Image *i;
    ulong chan;

    if(readn(fd, hdr, 5*12) != 5*12)
        return nil;

    /*
     * distinguish new channel descriptor from old ldepth.
     * channel descriptors have letters as well as numbers,
     * while ldepths are a single digit formatted as %-11d.
     */
    new = 0;
    for(m=0; m<10; m++){
        if(hdr[m] != ' '){
            new = 1;
            break;
        }
    }
    if(hdr[11] != ' '){
        werrstr("creadimage: bad format");
        return nil;
    }
    if(new){
        hdr[11] = '\0';
        if((chan = strtochan(hdr)) == 0){
            werrstr("creadimage: bad channel string %s", hdr);
            return nil;
        }
    }else{
        ldepth = ((int)hdr[10])-'0';
        if(ldepth<0 || ldepth>3){
            werrstr("creadimage: bad ldepth %d", ldepth);
            return nil;
        }
        chan = drawld2chan[ldepth];
    }
    r.min.x=atoi(hdr+1*12);
    r.min.y=atoi(hdr+2*12);
    r.max.x=atoi(hdr+3*12);
    r.max.y=atoi(hdr+4*12);
    if(r.min.x>r.max.x || r.min.y>r.max.y){
        werrstr("creadimage: bad rectangle");
        return nil;
    }

    if(d){
        if(dolock)
            lockdisplay(d);
        i = allocimage(d, r, chan, 0, 0);
        /*s: [[creadimage()]] set malloc tag for debug */
        setmalloctag(i, getcallerpc(&d));
        /*e: [[creadimage()]] set malloc tag for debug */
        if(dolock)
            unlockdisplay(d);
        if(i == nil)
            return nil;
    }else{
        i = mallocz(sizeof(Image), 1);
        if(i == nil)
            return nil;
    }
    ncblock = _compblocksize(r, chantodepth(chan));
    buf = malloc(ncblock);
    if(buf == nil)
        goto Errout;
    miny = r.min.y;
    while(miny != r.max.y){
        if(readn(fd, hdr, 2*12) != 2*12){
        Errout:
            if(dolock)
                lockdisplay(d);
        Erroutlock:
            freeimage(i);
            if(dolock)
                unlockdisplay(d);
            free(buf);
            return nil;
        }
        maxy = atoi(hdr+0*12);
        nb = atoi(hdr+1*12);
        if(maxy<=miny || r.max.y<maxy){
            werrstr("creadimage: bad maxy %d", maxy);
            goto Errout;
        }
        if(nb<=0 || ncblock<nb){
            werrstr("creadimage: bad count %d", nb);
            goto Errout;
        }
        if(readn(fd, buf, nb)!=nb)
            goto Errout;
        if(d){
            if(dolock)
                lockdisplay(d);
            a = bufimage(i->display, 21+nb);
            if(a == nil)
                goto Erroutlock;
            a[0] = 'Y';
            BPLONG(a+1, i->id);
            BPLONG(a+5, r.min.x);
            BPLONG(a+9, miny);
            BPLONG(a+13, r.max.x);
            BPLONG(a+17, maxy);
            if(!new)	/* old image: flip the data bits */
                _twiddlecompressed(buf, nb);
            memmove(a+21, buf, nb);
            if(dolock)
                unlockdisplay(d);
        }
        miny = maxy;
    }
    free(buf);
    return i;
}
/*e: function creadimage */

/*s: constant HSHIFT */
#define	HSHIFT	3	/* HSHIFT==5 runs slightly faster, but hash table is 64x bigger */
/*e: constant HSHIFT */
/*s: constant NHASH */
#define	NHASH	(1<<(HSHIFT*NMATCH))
/*e: constant NHASH */
/*s: constant HMASK */
#define	HMASK	(NHASH-1)
/*e: constant HMASK */
/*s: function hupdate */
#define	hupdate(h, c)	((((h)<<HSHIFT)^(c))&HMASK)
/*e: function hupdate */

typedef struct Hlist Hlist;
/*s: struct Hlist */
struct Hlist{
    uchar *s;
    Hlist *next, *prev;
};
/*e: struct Hlist */

/*s: function writeimage */
int
writeimage(fdt fd, Image *i, bool dolock)
{
    uchar *outbuf, *outp, *eout;		/* encoded data, pointer, end */
    uchar *loutp;				/* start of encoded line */
    Hlist *hash;				/* heads of hash chains of past strings */
    Hlist *chain, *hp;			/* hash chain members, pointer */
    Hlist *cp;				/* next Hlist to fall out of window */
    int h;					/* hash value */
    uchar *line, *eline;			/* input line, end pointer */
    uchar *data, *edata;			/* input buffer, end pointer */
    ulong n;				/* length of input buffer */
    ulong nb;				/* # of bytes returned by unloadimage */
    int bpl;				/* input line length */
    int offs, runlen;			/* offset, length of consumed data */
    uchar dumpbuf[NDUMP];			/* dump accumulator */
    int ndump;				/* length of dump accumulator */
    int miny, dy;				/* y values while unloading input */
    int chunk, ncblock;
    Rectangle r;
    uchar *p, *q, *s, *es, *t;
    char hdr[11+5*12+1];
    char cbuf[20];

    chunk = i->display->bufsize - 32;	/* a little room for header */
    r = i->r;
    bpl = bytesperline(r, i->depth);
    n = Dy(r)*bpl;
    data = malloc(n);

    ncblock = _compblocksize(r, i->depth);
    outbuf = malloc(ncblock);
    hash = malloc(NHASH*sizeof(Hlist));
    chain = malloc(NMEM*sizeof(Hlist));
    if(data == 0 || outbuf == 0 || hash == 0 || chain == 0){
    ErrOut:
        free(data);
        free(outbuf);
        free(hash);
        free(chain);
        return -1;
    }

    for(miny = r.min.y; miny != r.max.y; miny += dy){
        dy = r.max.y-miny;
        if(dy*bpl > chunk)
            dy = chunk/bpl;

        if(dolock)
            lockdisplay(i->display);

        nb = unloadimage(i, Rect(r.min.x, miny, r.max.x, miny+dy),
            data+(miny-r.min.y)*bpl, dy*bpl);

        if(dolock)
            unlockdisplay(i->display);

        if(nb != dy*bpl)
            goto ErrOut;
    }

    sprint(hdr, "compressed\n%11s %11d %11d %11d %11d ",
        chantostr(cbuf, i->chan), r.min.x, r.min.y, r.max.x, r.max.y);
    if(write(fd, hdr, 11+5*12) != 11+5*12)
        goto ErrOut;
    edata = data+n;
    eout = outbuf+ncblock;
    line = data;
    r.max.y = r.min.y;
    while(line != edata){
        memset(hash, 0, NHASH*sizeof(Hlist));
        memset(chain, 0, NMEM*sizeof(Hlist));
        cp = chain;
        h = 0;
        outp = outbuf;
        for(n = 0; n != NMATCH; n++)
            h = hupdate(h, line[n]);
        loutp = outbuf;
        while(line != edata){
            ndump = 0;
            eline = line+bpl;
            for(p = line; p != eline; ){
                if(eline-p < NRUN)
                    es = eline;
                else
                    es = p+NRUN;
                q = 0;
                runlen = 0;
                for(hp = hash[h].next; hp; hp = hp->next){
                    s = p + runlen;
                    if(s >= es)
                        continue;
                    t = hp->s + runlen;
                    for(; s >= p; s--)
                        if(*s != *t--)
                            goto matchloop;
                    t += runlen+2;
                    s += runlen+2;
                    for(; s < es; s++)
                        if(*s != *t++)
                            break;
                    n = s-p;
                    if(n > runlen){
                        runlen = n;
                        q = hp->s;
                        if(n == NRUN)
                            break;
                    }
            matchloop: ;
                }
                if(runlen < NMATCH){
                    if(ndump == NDUMP){
                        if(eout-outp < ndump+1)
                            goto Bfull;
                        *outp++ = ndump-1+128;
                        memmove(outp, dumpbuf, ndump);
                        outp += ndump;
                        ndump = 0;
                    }
                    dumpbuf[ndump++] = *p;
                    runlen = 1;
                }
                else{
                    if(ndump != 0){
                        if(eout-outp < ndump+1)
                            goto Bfull;
                        *outp++ = ndump-1+128;
                        memmove(outp, dumpbuf, ndump);
                        outp += ndump;
                        ndump = 0;
                    }
                    offs = p-q-1;
                    if(eout-outp < 2)
                        goto Bfull;
                    *outp++ = ((runlen-NMATCH)<<2) + (offs>>8);
                    *outp++ = offs&255;
                }
                for(q = p+runlen; p != q; p++){
                    if(cp->prev)
                        cp->prev->next = 0;
                    cp->next = hash[h].next;
                    cp->prev = &hash[h];
                    if(cp->next)
                        cp->next->prev = cp;
                    cp->prev->next = cp;
                    cp->s = p;
                    if(++cp == &chain[NMEM])
                        cp = chain;
                    if(edata-p > NMATCH)
                        h = hupdate(h, p[NMATCH]);
                }
            }
            if(ndump != 0){
                if(eout-outp < ndump+1)
                    goto Bfull;
                *outp++ = ndump-1+128;
                memmove(outp, dumpbuf, ndump);
                outp += ndump;
            }
            line = eline;
            loutp = outp;
            r.max.y++;
        }
    Bfull:
        if(loutp == outbuf)
            goto ErrOut;
        n = loutp-outbuf;
        sprint(hdr, "%11d %11ld ", r.max.y, n);
        write(fd, hdr, 2*12);
        write(fd, outbuf, n);
        r.min.y = r.max.y;
    }
    free(data);
    free(outbuf);
    free(hash);
    free(chain);
    return 0;
}
/*e: function writeimage */

/*e: lib_graphics/libdraw/compressed.c */
