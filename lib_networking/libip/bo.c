/*s: lib_networking/libip/bo.c */
#include <u.h>
#include <libc.h>
#include <ip.h>

/*s: function [[hnputv]] */
void
hnputv(void *p, uvlong v)
{
    uchar *a;

    a = p;
    a[0] = v>>56;
    a[1] = v>>48;
    a[2] = v>>40;
    a[3] = v>>32;
    a[4] = v>>24;
    a[5] = v>>16;
    a[6] = v>>8;
    a[7] = v;
}
/*e: function [[hnputv]] */

/*s: function [[hnputl]] */
void
hnputl(void *p, uint v)
{
    uchar *a;

    a = p;
    a[0] = v>>24;
    a[1] = v>>16;
    a[2] = v>>8;
    a[3] = v;
}
/*e: function [[hnputl]] */

/*s: function [[hnputs]] */
void
hnputs(void *p, ushort v)
{
    uchar *a;

    a = p;
    a[0] = v>>8;
    a[1] = v;
}
/*e: function [[hnputs]] */

/*s: function [[nhgetv]] */
uvlong
nhgetv(void *p)
{
    uchar *a;
    uvlong v;

    a = p;
    v = (uvlong)a[0]<<56;
    v |= (uvlong)a[1]<<48;
    v |= (uvlong)a[2]<<40;
    v |= (uvlong)a[3]<<32;
    v |= a[4]<<24;
    v |= a[5]<<16;
    v |= a[6]<<8;
    v |= a[7]<<0;
    return v;
}
/*e: function [[nhgetv]] */

/*s: function [[nhgetl]] */
uint
nhgetl(void *p)
{
    uchar *a;

    a = p;
    return (a[0]<<24)|(a[1]<<16)|(a[2]<<8)|(a[3]<<0);
}
/*e: function [[nhgetl]] */

/*s: function [[nhgets]] */
ushort
nhgets(void *p)
{
    uchar *a;

    a = p;
    return (a[0]<<8)|(a[1]<<0);
}
/*e: function [[nhgets]] */
/*e: lib_networking/libip/bo.c */
