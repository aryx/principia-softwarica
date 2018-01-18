/*s: lib_graphics/libmemdraw/io.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>
#include <memdraw.h>

/*s: function [[readmemimage]] */
Memimage*
readmemimage(fdt fd)
{
    char hdr[5*12+1];
    int dy;
    ulong chan;
    uint l, n;
    int m, j;
    int new, miny, maxy;
    Rectangle r;
    uchar *tmp;
    int ldepth, chunk;
    Memimage *i;

    if(readn(fd, hdr, 11) != 11){
        werrstr("readimage: short header");
        return nil;
    }
    if(memcmp(hdr, "compressed\n", 11) == 0)
        return creadmemimage(fd);

    if(readn(fd, hdr+11, 5*12-11) != 5*12-11){
        werrstr("readimage: short header (2)");
        return nil;
    }

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
        werrstr("readimage: bad format");
        return nil;
    }
    if(new){
        hdr[11] = '\0';
        if((chan = strtochan(hdr)) == 0){
            werrstr("readimage: bad channel string %s", hdr);
            return nil;
        }
    }else{
        ldepth = ((int)hdr[10])-'0';
        if(ldepth<0 || ldepth>3){
            werrstr("readimage: bad ldepth %d", ldepth);
            return nil;
        }
        chan = drawld2chan[ldepth];
    }

    r.min.x = atoi(hdr+1*12);
    r.min.y = atoi(hdr+2*12);
    r.max.x = atoi(hdr+3*12);
    r.max.y = atoi(hdr+4*12);
    if(r.min.x>r.max.x || r.min.y>r.max.y){
        werrstr("readimage: bad rectangle");
        return nil;
    }

    miny = r.min.y;
    maxy = r.max.y;

    l = bytesperline(r, chantodepth(chan));
    i = allocmemimage(r, chan);
    if(i == nil)
        return nil;
    chunk = 32*1024;
    if(chunk < l)
        chunk = l;
    tmp = malloc(chunk);
    if(tmp == nil)
        goto Err;
    while(maxy > miny){
        dy = maxy - miny;
        if(dy*l > chunk)
            dy = chunk/l;
        if(dy <= 0){
            werrstr("readmemimage: image too wide for buffer");
            goto Err;
        }
        n = dy*l;
        m = readn(fd, tmp, n);
        if(m != n){
            werrstr("readmemimage: read count %d not %d: %r", m, n);
   Err:
    freememimage(i);
            free(tmp);
            return nil;
        }
        if(!new)	/* an old image: must flip all the bits */
            for(j=0; j<chunk; j++)
                tmp[j] ^= 0xFF;

        if(loadmemimage(i, Rect(r.min.x, miny, r.max.x, miny+dy), tmp, chunk) <= 0)
            goto Err;
        miny += dy;
    }
    free(tmp);
    return i;
}
/*e: function [[readmemimage]] */

/*s: function [[creadmemimage]] */
Memimage*
creadmemimage(int fd)
{
    char hdr[5*12+1];
    Rectangle r;
    int m, nb, miny, maxy, new, ldepth, ncblock;
    uchar *buf;
    Memimage *i;
    ulong chan;

    if(readn(fd, hdr, 5*12) != 5*12){
        werrstr("readmemimage: short header (2)");
        return nil;
    }

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

    i = allocmemimage(r, chan);
    if(i == nil)
        return nil;
    ncblock = _compblocksize(r, i->depth);
    buf = malloc(ncblock);
    if(buf == nil)
        goto Errout;
    miny = r.min.y;
    while(miny != r.max.y){
        if(readn(fd, hdr, 2*12) != 2*12){
        Shortread:
            werrstr("readmemimage: short read");
        Errout:
            freememimage(i);
            free(buf);
            return nil;
        }
        maxy = atoi(hdr+0*12);
        nb = atoi(hdr+1*12);
        if(maxy<=miny || r.max.y<maxy){
            werrstr("readimage: bad maxy %d", maxy);
            goto Errout;
        }
        if(nb<=0 || ncblock<nb){
            werrstr("readimage: bad count %d", nb);
            goto Errout;
        }
        if(readn(fd, buf, nb)!=nb)
            goto Shortread;
        if(!new)	/* old image: flip the data bits */
            _twiddlecompressed(buf, nb);
        cloadmemimage(i, Rect(r.min.x, miny, r.max.x, maxy), buf, nb);
        miny = maxy;
    }
    free(buf);
    return i;
}
/*e: function [[creadmemimage]] */



/*s: constant [[CHUNK]] */
#define	CHUNK	8000
/*e: constant [[CHUNK]] */

/*s: constant [[HSHIFT]]([[(lib_graphics/libmemdraw/write.c)]]) */
#define	HSHIFT	3	/* HSHIFT==5 runs slightly faster, but hash table is 64x bigger */
/*e: constant [[HSHIFT]]([[(lib_graphics/libmemdraw/write.c)]]) */
/*s: constant [[NHASH]]([[(lib_graphics/libmemdraw/write.c)]]) */
#define	NHASH	(1<<(HSHIFT*NMATCH))
/*e: constant [[NHASH]]([[(lib_graphics/libmemdraw/write.c)]]) */
/*s: constant [[HMASK]]([[(lib_graphics/libmemdraw/write.c)]]) */
#define	HMASK	(NHASH-1)
/*e: constant [[HMASK]]([[(lib_graphics/libmemdraw/write.c)]]) */
/*s: function [[hupdate]]([[(lib_graphics/libmemdraw/write.c)]]) */
#define	hupdate(h, c)	((((h)<<HSHIFT)^(c))&HMASK)
/*e: function [[hupdate]]([[(lib_graphics/libmemdraw/write.c)]]) */
typedef struct Hlist Hlist;
/*s: struct [[Hlist]]([[(lib_graphics/libmemdraw/write.c)]]) */
struct Hlist{
    uchar *s;
    Hlist *next, *prev;
};
/*e: struct [[Hlist]]([[(lib_graphics/libmemdraw/write.c)]]) */

/*s: function [[writememimage]] */
int
writememimage(int fd, Memimage *i)
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
    int ncblock;				/* size of compressed blocks */
    Rectangle r;
    uchar *p, *q, *s, *es, *t;
    char hdr[11+5*12+1];
    char cbuf[20];

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
        if(dy*bpl > CHUNK)
            dy = CHUNK/bpl;
        nb = unloadmemimage(i, Rect(r.min.x, miny, r.max.x, miny+dy),
            data+(miny-r.min.y)*bpl, dy*bpl);
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
/*e: function [[writememimage]] */

/*e: lib_graphics/libmemdraw/io.c */
