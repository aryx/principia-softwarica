/*s: kernel/network/ip/inferno.c */
#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"
#include    "../port/error.h"

/*s: function commonuser */
/*
 *  some hacks for commonality twixt inferno and plan9
 */

char*
commonuser(void)
{
    return up->user;
}
/*e: function commonuser */

/*s: function commonerror */
char*
commonerror(void)
{
    return up->errstr;
}
/*e: function commonerror */

/*s: function bootpread */
int
bootpread(char*, ulong, int)
{
    return  0;
}
/*e: function bootpread */
/*e: kernel/network/ip/inferno.c */
