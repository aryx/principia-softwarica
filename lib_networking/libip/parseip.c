/*s: lib_networking/libip/parseip.c */
#include <u.h>
#include <libc.h>
#include <ctype.h>
#include <ip.h>

/*s: function v4parseip */
char*
v4parseip(ipv4 to, char *from)
{
    int i;
    char *p;

    p = from;
    for(i = 0; i < IPv4addrlen && *p; i++){
        to[i] = strtoul(p, &p, 0);
        if(*p == '.')
            p++;
    }
    switch(CLASS(to)){
    case 0:	/* class A - 1 uchar net */
    case 1:
        if(i == 3){
            to[3] = to[2];
            to[2] = to[1];
            to[1] = 0;
        } else if (i == 2){
            to[3] = to[1];
            to[1] = 0;
        }
        break;
    case 2:	/* class B - 2 uchar net */
        if(i == 3){
            to[3] = to[2];
            to[2] = 0;
        }
        break;
    }
    return p;
}
/*e: function v4parseip */

/*s: function ipcharok */
static int
ipcharok(int c)
{
    return c == '.' || c == ':' || isascii(c) && isxdigit(c);
}
/*e: function ipcharok */

/*s: function delimchar */
static bool
delimchar(int c)
{
    if(c == '\0')
        return true;
    if(c == '.' || c == ':' || isascii(c) && isalnum(c))
        return false;
    return true;
}
/*e: function delimchar */

/*s: function parseip */
/*
 * `from' may contain an address followed by other characters,
 * at least in /boot, so we permit whitespace (and more) after the address.
 * we do ensure that "delete" cannot be parsed as "de::".
 *
 * some callers don't check the return value for errors, so
 * set `to' to something distinctive in the case of a parse error.
 */
vlong
parseip(ipaddr to, char *from)
{
    int i, elipsis = 0;
    bool v4 = true;
    ulong x;
    char *p, *op;

    memset(to, 0, IPaddrlen);
    p = from;
    for(i = 0; i < IPaddrlen && ipcharok(*p); i+=2){
        op = p;
        x = strtoul(p, &p, 16);
        if((*p == '.' && i <= IPaddrlen-4) || (*p == '\0' && i == 0)){
            /* ends with v4 */
            p = v4parseip(to+i, op);
            i += 4;
            break;
        }

        /* v6: at most 4 hex digits, followed by colon or delim */
        if(x != (ushort)x || *p != ':' && !delimchar(*p)) {
            memset(to, 0, IPaddrlen);
            return -1;			/* parse error */
        }
        to[i] = x>>8;
        to[i+1] = x;
        if(*p == ':'){
            v4 = false;
            if(*++p == ':'){	/* :: is elided zero short(s) */
                if (elipsis) {
                    memset(to, 0, IPaddrlen);
                    return -1;	/* second :: */
                }
                elipsis = i+2;
                p++;
            }
        } else if (p == op)		/* strtoul made no progress? */
            break;
    }
    if (p == from || !delimchar(*p)) {
        memset(to, 0, IPaddrlen);
        return -1;				/* parse error */
    }
    if(i < IPaddrlen){
        memmove(&to[elipsis+IPaddrlen-i], &to[elipsis], i-elipsis);
        memset(&to[elipsis], 0, IPaddrlen-i);
    }

    if(v4){
        to[10] = to[11] = 0xff;
        return nhgetl(to + IPv4off);
    } else
        return 6;
}
/*e: function parseip */

/*s: function parseipmask */
/*
 *  hack to allow ip v4 masks to be entered in the old
 *  style
 */
vlong
parseipmask(uchar *to, char *from)
{
    int i, w;
    vlong x;
    uchar *p;

    if(*from == '/'){
        /* as a number of prefix bits */
        i = atoi(from+1);
        if(i < 0)
            i = 0;
        if(i > 128)
            i = 128;
        w = i;
        memset(to, 0, IPaddrlen);
        for(p = to; i >= 8; i -= 8)
            *p++ = 0xff;
        if(i > 0)
            *p = ~((1<<(8-i))-1);
        x = nhgetl(to+IPv4off);
        /*
         * identify as ipv6 if the mask is inexpressible as a v4 mask
         * (because it has too few mask bits).  Arguably, we could
         * always return 6 here.
         */
        if (w < 8*(IPaddrlen-IPv4addrlen))
            return 6;
    } else {
        /* as a straight v4 bit mask */
        x = parseip(to, from);
        if (x != -1)
            x = (ulong)nhgetl(to + IPv4off);
        if(memcmp(to, v4prefix, IPv4off) == 0)
            memset(to, 0xff, IPv4off);
    }
    return x;
}
/*e: function parseipmask */

/*e: lib_networking/libip/parseip.c */
