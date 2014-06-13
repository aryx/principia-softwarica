/*s: devsys.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

#include    <authsrv.h>

/*s: devsys.c enum Qxxx */
enum{
    Qdir,

    /*s: devsys.c enum Qxxx cases */
        Qosversion,
    /*x: devsys.c enum Qxxx cases */
        Qconfig,
    /*x: devsys.c enum Qxxx cases */
        Qhostowner,
        Qhostdomain,
    /*x: devsys.c enum Qxxx cases */
        Qsysname,
    /*x: devsys.c enum Qxxx cases */
        Qdrivers,
    /*x: devsys.c enum Qxxx cases */
        Qreboot,
    /*x: devsys.c enum Qxxx cases */
        Qsysstat,
    /*e: devsys.c enum Qxxx cases */
};
/*e: devsys.c enum Qxxx */

/*s: global sysdir */
static Dirtab sysdir[]={
    ".",    {Qdir, 0, QTDIR},   0,      DMDIR|0555,

    /*s: [[sysdir]] fields */
        "osversion",    {Qosversion},   0,      0444,
    /*x: [[sysdir]] fields */
        "config",   {Qconfig},  0,      0444,
    /*x: [[sysdir]] fields */
        "hostowner",    {Qhostowner},   0,      0664,
        "hostdomain",   {Qhostdomain},  DOMLEN,     0664,
    /*x: [[sysdir]] fields */
        "sysname",  {Qsysname}, 0,      0664,
    /*x: [[sysdir]] fields */
        "drivers",  {Qdrivers}, 0,      0444,
    /*x: [[sysdir]] fields */
        "reboot",   {Qreboot},  0,      0660,
    /*x: [[sysdir]] fields */
        "sysstat",  {Qsysstat}, 0,      0666,
    /*e: [[sysdir]] fields */
};
/*e: global sysdir */

/*s: devsys.c decls and globals */
extern uchar configfile[]; // in $CONF.c
/*x: devsys.c decls and globals */
enum
{
    CMhalt,
    CMreboot,
    CMpanic,
};

Cmdtab rebootmsg[] =
{
    CMhalt,     "halt",     1,
    CMreboot,   "reboot",   0,
    CMpanic,    "panic",    0,
};
/*e: devsys.c decls and globals */
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
    /*s: [[sysread()]] locals */
        char *b, *bp;
        int i, k;
        Cpu *mp;
        int id;
    /*e: [[sysread()]] locals */

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
        case Qconfig:
            return readstr((ulong)offset, buf, n, (char*) configfile);
    /*x: [[sysread()]] cases */
        case Qhostowner:
            return readstr((ulong)offset, buf, n, eve);

        case Qhostdomain:
            return readstr((ulong)offset, buf, n, hostdomain);
    /*x: [[sysread()]] cases */
        case Qsysname:
            if(sysname == nil)
                return 0;
            return readstr((ulong)offset, buf, n, sysname);
    /*x: [[sysread()]] cases */
        case Qdrivers:
            b = malloc(READSTR);
            if(b == nil)
                error(Enomem);
            k = 0;
            for(i = 0; devtab[i] != nil; i++)
                k += snprint(b+k, READSTR-k, "#%C %s\n",
                    devtab[i]->dc, devtab[i]->name);
            if(waserror()){
                free(b);
                nexterror();
            }
            n = readstr((ulong)offset, buf, n, b);
            free(b);
            poperror();
            return n;
    /*x: [[sysread()]] cases */
        case Qsysstat:
            b = smalloc(conf.ncpu*(NUMSIZE*11+1) + 1); /* +1 for NUL */
            bp = b;
            for(id = 0; id < MAXCPUS; id++) {
                if(active.cpus & (1<<id)) {
                    mp = CPUS(id);
                    readnum(0, bp, NUMSIZE, id, NUMSIZE);
                    bp += NUMSIZE;
                    readnum(0, bp, NUMSIZE, mp->cs, NUMSIZE);
                    bp += NUMSIZE;
                    readnum(0, bp, NUMSIZE, mp->intr, NUMSIZE);
                    bp += NUMSIZE;
                    readnum(0, bp, NUMSIZE, mp->syscall, NUMSIZE);
                    bp += NUMSIZE;
                    readnum(0, bp, NUMSIZE, mp->pfault, NUMSIZE);
                    bp += NUMSIZE;
                    readnum(0, bp, NUMSIZE, mp->tlbfault, NUMSIZE);
                    bp += NUMSIZE;
                    readnum(0, bp, NUMSIZE, mp->tlbpurge, NUMSIZE);
                    bp += NUMSIZE;
                    readnum(0, bp, NUMSIZE, mp->load, NUMSIZE);
                    bp += NUMSIZE;
                    readnum(0, bp, NUMSIZE,
                        (mp->perf.avg_inidle*100)/mp->perf.period,
                        NUMSIZE);
                    bp += NUMSIZE;
                    readnum(0, bp, NUMSIZE,
                        (mp->perf.avg_inintr*100)/mp->perf.period,
                        NUMSIZE);
                    bp += NUMSIZE;
                    *bp++ = '\n';
                }
            }
            if(waserror()){
                free(b);
                nexterror();
            }
            n = readstr((ulong)offset, buf, n, b);
            free(b);
            poperror();
            return n;
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
    /*s: [[syswrite()]] locals */
        char buf[256];
        Cpu *mp;
        int id;
    /*x: [[syswrite()]] locals */
        Cmdbuf *cb;
        Cmdtab *ct;
    /*e: [[syswrite()]] locals */

    offset = off;
    a = va;

    switch((ulong)c->qid.path){

    /*s: [[syswrite()]] cases */
        case Qhostowner:
            return hostownerwrite(a, n);

        case Qhostdomain:
            return hostdomainwrite(a, n);
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
        case Qreboot:
            if(!iseve())
                error(Eperm);
            cb = parsecmd(a, n);

            if(waserror()) {
                free(cb);
                nexterror();
            }
            ct = lookupcmd(cb, rebootmsg, nelem(rebootmsg));
            switch(ct->index) {
            case CMhalt:
                reboot(nil, 0, 0);
                break;
            case CMreboot:
                rebootcmd(cb->nf-1, cb->f+1);
                break;
            case CMpanic:
                *(ulong*)0=0;
                panic("/dev/reboot");
            }
            poperror();
            free(cb);
            break;
    /*x: [[syswrite()]] cases */
        case Qsysstat:
            for(id = 0; id < 32; id++) {
                if(active.cpus & (1<<id)) {
                    mp = CPUS(id);
                    mp->cs = 0;
                    mp->intr = 0;
                    mp->syscall = 0;
                    mp->pfault = 0;
                    mp->tlbfault = 0;
                    mp->tlbpurge = 0;
                }
            }
            break;

    /*e: [[syswrite()]] cases */

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
