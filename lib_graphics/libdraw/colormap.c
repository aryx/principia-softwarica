/*s: lib_graphics/libdraw/colormap.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <bio.h>

/*s: function [[getval]] */
static ulong
getval(char **p)
{
    ulong v;
    char *q;

    v = strtoul(*p, &q, 0);
    v |= v<<8;
    v |= v<<16;
    *p = q;
    return v;
}
/*e: function [[getval]] */

/*s: function [[readcolmap]] */
void
readcolmap(Display *d, RGB *colmap)
{
    int i;
    char *p, *q;
    Biobuf *b;
    char buf[128];

    sprint(buf, "/dev/draw/%d/colormap", d->dirno);
    b = Bopen(buf, OREAD);
    if(b == 0)
        drawerror(d, "rdcolmap: can't open colormap device");

    for(;;) {
        p = Brdline(b, '\n');
        if(p == 0)
            break;
        i = strtoul(p, &q, 0);
        if(i < 0 || i > 255) {
            fprint(2, "rdcolmap: bad index\n");
            exits("bad");
        }
        p = q;
        colmap[255-i].red = getval(&p);
        colmap[255-i].green = getval(&p);
        colmap[255-i].blue = getval(&p);
    }
    Bterm(b);
}
/*e: function [[readcolmap]] */

/*s: function [[writecolmap]] */
/*
 * This code (and the devdraw interface) will have to change
 * if we ever get bitmaps with ldepth > 3, because the
 * colormap will have to be written in chunks
 */
void
writecolmap(Display *d, RGB *m)
{
    int i, n, fd;
    char buf[64], *t;
    ulong r, g, b;

    sprint(buf, "/dev/draw/%d/colormap", d->dirno);
    fd = open(buf, OWRITE);
    if(fd < 0)
        drawerror(d, "writecolmap: open colormap failed");
    t = malloc(8192);
    if (t == nil)
        drawerror(d, "writecolmap: no memory");
    n = 0;
    for(i = 0; i < 256; i++) {
        r = m[i].red>>24;
        g = m[i].green>>24;
        b = m[i].blue>>24;
        n += sprint(t+n, "%d %lud %lud %lud\n", 255-i, r, g, b);
    }
    i = write(fd, t, n);
    free(t);
    close(fd);
    if(i != n)
        drawerror(d, "writecolmap: bad write");
}
/*e: function [[writecolmap]] */

/*s: function [[rgb2cmap]] */
int
rgb2cmap(int cr, int cg, int cb)
{
    int i, r, g, b, sq;
    ulong rgb;
    int best, bestsq;

    best = 0;
    bestsq = 0x7FFFFFFF;
    // find best candidate
    for(i=0; i<256; i++){
        rgb = cmap2rgb(i);
        r = (rgb>>16) & 0xFF;
        g = (rgb>>8) & 0xFF;
        b = (rgb>>0) & 0xFF;
        sq = (r-cr)*(r-cr)+(g-cg)*(g-cg)+(b-cb)*(b-cb);
        if(sq < bestsq){
            bestsq = sq;
            best = i;
        }
    }
    return best;
}
/*e: function [[rgb2cmap]] */

/*s: function [[cmap2rgb]] */
int
cmap2rgb(int c)
{
    int j, num, den, r, g, b, v, rgb;

    r = c>>6;
    v = (c>>4)&3;
    j = (c-v+r)&15;
    g = j>>2;
    b = j&3;
    den=r;
    if(g>den)
        den=g;
    if(b>den)
        den=b;
    if(den==0) {
        v *= 17;
        rgb = (v<<16)|(v<<8)|v;
    }
    else{
        num=17*(4*den+v);
        rgb = ((r*num/den)<<16)|((g*num/den)<<8)|(b*num/den);
    }
    return rgb;
}
/*e: function [[cmap2rgb]] */

/*s: function [[cmap2rgba]] */
int
cmap2rgba(int c)
{
    return (cmap2rgb(c)<<8)|0xFF;
}
/*e: function [[cmap2rgba]] */

/*e: lib_graphics/libdraw/colormap.c */
