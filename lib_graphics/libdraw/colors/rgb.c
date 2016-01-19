/*s: lib_graphics/libdraw/rgb.c */
#include <u.h>
#include <libc.h>
#include <draw.h>


/*s: function rgb2cmap */
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
/*e: function rgb2cmap */

/*s: function cmap2rgb */
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
/*e: function cmap2rgb */

/*s: function cmap2rgba */
int
cmap2rgba(int c)
{
    return (cmap2rgb(c)<<8)|0xFF;
}
/*e: function cmap2rgba */
/*e: lib_graphics/libdraw/rgb.c */
