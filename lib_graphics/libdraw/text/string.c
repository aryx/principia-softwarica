/*s: lib_graphics/libdraw/string.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>

/*s: enum _anon_ (lib_graphics/libdraw/string.c) */
enum
{
/*s: constant Max */
Max = 100
/*e: constant Max */
};
/*e: enum _anon_ (lib_graphics/libdraw/string.c) */

/*s: function string */
Point
string(Image *dst, Point pt, Image *src, Point sp, Font *f, char *s)
{
    return _string(dst, pt, src, sp, f, s, nil, 1<<24, dst->clipr, nil, ZP, SoverD);
}
/*e: function string */

/*s: function stringop */
Point
stringop(Image *dst, Point pt, Image *src, Point sp, Font *f, char *s, Drawop op)
{
    return _string(dst, pt, src, sp, f, s, nil, 1<<24, dst->clipr, nil, ZP, op);
}
/*e: function stringop */

/*s: function stringn */
Point
stringn(Image *dst, Point pt, Image *src, Point sp, Font *f, char *s, int len)
{
    return _string(dst, pt, src, sp, f, s, nil, len, dst->clipr, nil, ZP, SoverD);
}
/*e: function stringn */

/*s: function stringnop */
Point
stringnop(Image *dst, Point pt, Image *src, Point sp, Font *f, char *s, int len, Drawop op)
{
    return _string(dst, pt, src, sp, f, s, nil, len, dst->clipr, nil, ZP, op);
}
/*e: function stringnop */

/*s: function runestring */
Point
runestring(Image *dst, Point pt, Image *src, Point sp, Font *f, Rune *r)
{
    return _string(dst, pt, src, sp, f, nil, r, 1<<24, dst->clipr, nil, ZP, SoverD);
}
/*e: function runestring */

/*s: function runestringop */
Point
runestringop(Image *dst, Point pt, Image *src, Point sp, Font *f, Rune *r, Drawop op)
{
    return _string(dst, pt, src, sp, f, nil, r, 1<<24, dst->clipr, nil, ZP, op);
}
/*e: function runestringop */

/*s: function runestringn */
Point
runestringn(Image *dst, Point pt, Image *src, Point sp, Font *f, Rune *r, int len)
{
    return _string(dst, pt, src, sp, f, nil, r, len, dst->clipr, nil, ZP, SoverD);
}
/*e: function runestringn */

/*s: function runestringnop */
Point
runestringnop(Image *dst, Point pt, Image *src, Point sp, Font *f, Rune *r, int len, Drawop op)
{
    return _string(dst, pt, src, sp, f, nil, r, len, dst->clipr, nil, ZP, op);
}
/*e: function runestringnop */

/*s: function _string */
Point
_string(Image *dst, Point pt, Image *src, Point sp, Font *f, char *s, Rune *r, int len, Rectangle clipr, Image *bg, Point bgp, Drawop op)
{
    Rune **rptr;
    char **sptr;
    int m, n, max;
    ushort cbuf[Max], *c, *ec;
    byte *b;
    int wid;
    /*s: [[_string()]] other locals */
    char *subfontname;
    Subfont *sf = nil;
    /*x: [[_string()]] other locals */
    Font *def;
    /*e: [[_string()]] other locals */

    /*s: [[_string()]] non unicode string handling, set sptr and rptr */
    if(s == nil){
        s = "";
        sptr = nil;
    }else
        sptr = &s;

    if(r == nil){
        r = (Rune*) L"";
        rptr = nil;
    }
    /*e: [[_string()]] non unicode string handling, set sptr and rptr */
    else
      rptr = &r;

    while((*s || *r) && len){
        /*s: [[_string()]] set max */
        max = Max;
        if(len < max)
            max = len;
        /*e: [[_string()]] set max */

        // will trigger many calls to loadchar() for the same subfont
        n = cachechars(f, sptr, rptr, cbuf, max, &wid, &subfontname);

        if(n > 0){
            _setdrawop(dst->display, op);

            // string: 's' dstid[4] srcid[4] fontid[4] P[2*4] clipr[4*4] sp[2*4] ni[2] ni*(index[2]) 

            m = 47+2*n;
            /*s: [[_string()]] if bg part1 */
            // stringbg: 'x' dstid[4] srcid[4] fontid[4] P[2*4] clipr[4*4] sp[2*4] ni[2] bgid[4] bgpt[2*4] ni*(index[2])
            if(bg)
                m += 4+2*4;
            /*e: [[_string()]] if bg part1 */
            b = bufimage(dst->display, m);
            /*s: [[_string()]] sanity check b */
            if(b == nil){
                fprint(2, "string: %r\n");
                break;
            }
            /*e: [[_string()]] sanity check b */
            /*s: [[_string()]] if bg part2 */
            if(bg)
                b[0] = 'x';
            /*e: [[_string()]] if bg part2 */
            else
                b[0] = 's';
            BPLONG(b+1, dst->id);
            BPLONG(b+5, src->id);
            BPLONG(b+9, f->cacheimage->id);
            BPLONG(b+13, pt.x);
            BPLONG(b+17, pt.y + f->ascent);
            BPLONG(b+21, clipr.min.x);
            BPLONG(b+25, clipr.min.y);
            BPLONG(b+29, clipr.max.x);
            BPLONG(b+33, clipr.max.y);
            BPLONG(b+37, sp.x);
            BPLONG(b+41, sp.y);
            BPSHORT(b+45, n);
            b += 47;
            /*s: [[_string()]] if bg part3 */
            if(bg){
                BPLONG(b, bg->id);
                BPLONG(b+4, bgp.x);
                BPLONG(b+8, bgp.y);
                b += 12;
            }
            /*e: [[_string()]] if bg part3 */
            // index of the set of characters to draw (hashcode of Rune)
            ec = &cbuf[n];
            for(c=cbuf; c<ec; c++, b+=2)
                BPSHORT(b, *c);

            pt.x += wid;
            bgp.x += wid;

            agefont(f);

            len -= n; // progress
        }
        /*s: [[_string()]] if subfontname */
        if(subfontname){
            /*s: [[_string()]] free previous sf */
            freesubfont(sf);
            /*e: [[_string()]] free previous sf */
            // populate Font.subf global so next loadchar will find the subfont
            sf=_getsubfont(f->display, subfontname);
            /*s: [[_string()]] sanity check sf and possibly adjust f */
            if(sf == nil){
                def = f->display ? f->display->defaultfont : nil;
                if(def && f != def)
                    f = def;
                else
                    break;
            }
            /*e: [[_string()]] sanity check sf and possibly adjust f */
            /* 
             * must not free sf until cachechars has found it in the cache
             * and picked up its own reference.
             */
        }
        /*e: [[_string()]] if subfontname */
    }
    return pt;
}
/*e: function _string */
/*e: lib_graphics/libdraw/string.c */
