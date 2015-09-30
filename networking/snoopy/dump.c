/*s: networking/ip/snoopy/dump.c */
#include <u.h>
#include <libc.h>
#include <ip.h>
#include <ctype.h>
#include "dat.h"
#include "protos.h"

/*s: function p_compile (networking/ip/snoopy/dump.c) */
static void
p_compile(Filter *)
{
}
/*e: function p_compile (networking/ip/snoopy/dump.c) */

/*s: global tohex */
static char tohex[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f'
};
/*e: global tohex */

/*s: function p_seprint (networking/ip/snoopy/dump.c) */
static int
p_seprint(Msg *m)
{
    int c, i, n, isstring;
    uchar *ps = m->ps;
    char *p = m->p;
    char *e = m->e;

    n = m->pe - ps;
    if(n > Nflag)
        n = Nflag;

    isstring = 1;
    for(i = 0; i < n; i++){
        c = ps[i];
        if(!isprint(c) && !isspace(c)){
            isstring = 0;
            break;
        }
    }

    if(isstring){
        for(i = 0; i < n && p+1<e; i++){
            c = ps[i];
            switch(c){
            case '\t':
                *p++ = '\\';
                *p++ = 't';
                break;
            case '\r':
                *p++ = '\\';
                *p++ = 'r';
                break;
            case '\n':
                *p++ = '\\';
                *p++ = 'n';
                break;
            default:
                *p++ = c;
            }
        }
    } else {
        for(i = 0; i < n && p+1<e; i++){
            c = ps[i];
            *p++ = tohex[c>>4];
            *p++ = tohex[c&0xf]; 
        }
    }

    m->pr = nil;
    m->p = p;
    m->ps = ps;

    return 0;
}
/*e: function p_seprint (networking/ip/snoopy/dump.c) */

/*s: global dump */
Proto dump =
{
    "dump",
    p_compile,
    nil,
    p_seprint,
    nil,
    nil,
    nil,
    defaultframer,
};
/*e: global dump */
/*e: networking/ip/snoopy/dump.c */