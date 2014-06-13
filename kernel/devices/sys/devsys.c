/*s: devsys.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

/*s: devsys.c enum Qxxx */
enum{
    Qdir,

    Qmisc,
};
/*e: devsys.c enum Qxxx */

/*s: global sysdir */
static Dirtab sysdir[]={
    ".",    {Qdir, 0, QTDIR},   0,      DMDIR|0555,

    "misc", {Qmisc},    0,      0444,
};
/*e: global sysdir */

/*s: method sysinit */
static void
sysinit(void)
{
}
/*e: method sysinit */

/*s: method sysattach */
static Chan*
sysattach(char *spec)
{
    return devattach('k', spec);
}
/*e: method sysattach */

/*s: method syswalk */
static Walkqid*
syswalk(Chan *c, Chan *nc, char **name, int nname)
{
    return devwalk(c, nc, name, nname, sysdir, nelem(sysdir), devgen);
}
/*e: method syswalk */

/*s: method sysstat */
static int
sysstat(Chan *c, uchar *dp, int n)
{
    return devstat(c, dp, n, sysdir, nelem(sysdir), devgen);
}
/*e: method sysstat */

/*s: method sysopen */
static Chan*
sysopen(Chan *c, int omode)
{
    c->aux = nil;
    c = devopen(c, omode, sysdir, nelem(sysdir), devgen);
    switch((ulong)c->qid.path){
    }
    return c;
}
/*e: method sysopen */

/*s: method sysclose */
static void
sysclose(Chan *c)
{
    switch((ulong)c->qid.path){
    }
}
/*e: method sysclose */

/*s: method sysread */
static long
sysread(Chan *c, void *buf, long n, vlong off)
{
    vlong offset = off;

    if(n <= 0)
        return n;

    switch((ulong)c->qid.path){
    case Qdir:
        return devdirread(c, buf, n, sysdir, nelem(sysdir), devgen);

        
    case Qmisc:
        return readstr((ulong)offset, buf, n, up->user);


    default:
        print("sysread %#llux\n", c->qid.path);
        error(Egreg);
    }
    return -1;      /* never reached */
}
/*e: method sysread */

/*s: method syswrite */
static long
syswrite(Chan *c, void *va, long n, vlong off)
{
    ulong offset;

    offset = off;

    switch((ulong)c->qid.path){
    default:
        print("syswrite: %#llux\n", c->qid.path);
        error(Egreg);
    }
    return n;
}
/*e: method syswrite */

/*s: global sysdevtab */
Dev sysdevtab = {
    .dc       =    'k',
    .name     =    "sys",
               
    .reset    =    devreset,
    .init     =    sysinit,
    .shutdown =    devshutdown,
    .attach   =    sysattach,
    .walk     =    syswalk,
    .stat     =    sysstat,
    .open     =    sysopen,
    .create   =    devcreate,
    .close    =    sysclose,
    .read     =    sysread,
    .bread    =    devbread,
    .write    =    syswrite,
    .bwrite   =    devbwrite,
    .remove   =    devremove,
    .wstat    =    devwstat,
};
/*e: global sysdevtab */

/*e: devsys.c */
