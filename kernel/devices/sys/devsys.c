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

    /*s: devsys.c enum Qxxx cases */
        Qosversion,
    /*x: devsys.c enum Qxxx cases */
    /*x: devsys.c enum Qxxx cases */
        Qsysname,
    /*x: devsys.c enum Qxxx cases */
    /*e: devsys.c enum Qxxx cases */
};
/*e: devsys.c enum Qxxx */

/*s: global sysdir */
static Dirtab sysdir[]={
    ".",    {Qdir, 0, QTDIR},   0,      DMDIR|0555,

    /*s: [[sysdir]] fields */
        "osversion",    {Qosversion},   0,      0444,
    /*x: [[sysdir]] fields */
    /*x: [[sysdir]] fields */
        "sysname",  {Qsysname}, 0,      0664,
    /*x: [[sysdir]] fields */
    /*e: [[sysdir]] fields */
};
/*e: global sysdir */

/*s: global sysname */
char    *sysname;
/*e: global sysname */

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
    char tmp[256];

    if(n <= 0)
        return n;

    switch((ulong)c->qid.path){
    case Qdir:
        return devdirread(c, buf, n, sysdir, nelem(sysdir), devgen);

    /*s: [[sysread()]] cases */
        case Qosversion:
            snprint(tmp, sizeof tmp, "pad's version");
            n = readstr((ulong)offset, buf, n, tmp);
            return n;
    /*x: [[sysread()]] cases */
    /*x: [[sysread()]] cases */
        case Qsysname:
            if(sysname == nil)
                return 0;
            return readstr((ulong)offset, buf, n, sysname);
    /*x: [[sysread()]] cases */
    /*e: [[sysread()]] cases */

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
    char *a;
    char buf[256];

    offset = off;
    a = va;

    switch((ulong)c->qid.path){

    /*s: [[syswrite()]] cases */
    /*x: [[syswrite()]] cases */
    /*x: [[syswrite()]] cases */
        case Qsysname:
            if(offset != 0)
                error(Ebadarg);
            if(n <= 0 || n >= sizeof buf)
                error(Ebadarg);
            strncpy(buf, a, n);
            buf[n] = 0;
            if(buf[n-1] == '\n')
                buf[n-1] = 0;
            kstrdup(&sysname, buf);
            break;
    /*x: [[syswrite()]] cases */
    /*e: [[syswrite()]] cases */

    case Qosversion:

        error(Eperm);
        break;

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
