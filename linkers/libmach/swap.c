/*s: linkers/libmach/swap.c */
#include <u.h>

/*s: function beswab */
/*
 * big-endian short
 */
ushort
beswab(ushort s)
{
    uchar *p;

    p = (uchar*)&s;
    return (p[0]<<8) | p[1];
}
/*e: function beswab */

/*s: function beswal */
/*
 * big-endian long
 */
ulong
beswal(ulong l)
{
    uchar *p;

    p = (uchar*)&l;
    return (p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3];
}
/*e: function beswal */

/*s: function beswav */
/*
 * big-endian vlong
 */
uvlong
beswav(uvlong v)
{
    uchar *p;

    p = (uchar*)&v;
    return ((uvlong)p[0]<<56) | ((uvlong)p[1]<<48) | ((uvlong)p[2]<<40)
                  | ((uvlong)p[3]<<32) | ((uvlong)p[4]<<24)
                  | ((uvlong)p[5]<<16) | ((uvlong)p[6]<<8)
                  | (uvlong)p[7];
}
/*e: function beswav */

/*s: function leswab */
/*
 * little-endian short
 */
ushort
leswab(ushort s)
{
    uchar *p;

    p = (uchar*)&s;
    return (p[1]<<8) | p[0];
}
/*e: function leswab */

/*s: function leswal */
/*
 * little-endian long
 */
ulong
leswal(ulong l)
{
    uchar *p;

    p = (uchar*)&l;
    return (p[3]<<24) | (p[2]<<16) | (p[1]<<8) | p[0];
}
/*e: function leswal */

/*s: function leswav */
/*
 * little-endian vlong
 */
uvlong
leswav(uvlong v)
{
    uchar *p;

    p = (uchar*)&v;
    return ((uvlong)p[7]<<56) | ((uvlong)p[6]<<48) | ((uvlong)p[5]<<40)
                  | ((uvlong)p[4]<<32) | ((uvlong)p[3]<<24)
                  | ((uvlong)p[2]<<16) | ((uvlong)p[1]<<8)
                  | (uvlong)p[0];
}
/*e: function leswav */
/*e: linkers/libmach/swap.c */
