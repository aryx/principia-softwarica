/*s: lib_graphics/libdraw/chan.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: global channames */
static char channames[] = "rgbkamx";
/*e: global channames */
/*s: function chantostr */
char*
chantostr(char *buf, ulong cc)
{
    ulong c, rc;
    char *p;

    if(chantodepth(cc) == 0)
        return nil;

    /* reverse the channel descriptor so we can easily generate the string in the right order */
    rc = 0;
    for(c=cc; c; c>>=8){
        rc <<= 8;
        rc |= c&0xFF;
    }

    p = buf;
    for(c=rc; c; c>>=8) {
        *p++ = channames[TYPE(c)];
        *p++ = '0'+NBITS(c);
    }
    *p = 0;

    return buf;
}
/*e: function chantostr */

/*s: function isspace */
/* avoid pulling in ctype when using with drawterm etc. */
static int
isspace(char c)
{
    return c==' ' || c== '\t' || c=='\r' || c=='\n';
}
/*e: function isspace */

/*s: function strtochan */
ulong
strtochan(char *s)
{
    char *p, *q;
    ulong c;
    int t, n, d;

    c = 0;
    d = 0;
    p=s;
    while(*p && isspace(*p))
        p++;

    while(*p && !isspace(*p)){
        if((q = strchr(channames, p[0])) == nil) 
            return 0;
        t = q-channames;
        if(p[1] < '0' || p[1] > '9')
            return 0;
        n = p[1]-'0';
        d += n;
        c = (c<<8) | __DC(t, n);
        p += 2;
    }
    if(d==0 || (d>8 && d%8) || (d<8 && 8%d))
        return 0;
    return c;
}
/*e: function strtochan */

/*s: function chantodepth */
int
chantodepth(ulong c)
{
    int n;

    for(n=0; c; c>>=8){
        if(TYPE(c) >= NChan || NBITS(c) > 8 || NBITS(c) <= 0)
            return 0; // warning?
        n += NBITS(c);
    }
    if(n==0 || (n>8 && n%8) || (n<8 && 8%n))
        return 0; // warning?
    return n;
}
/*e: function chantodepth */
/*e: lib_graphics/libdraw/chan.c */
