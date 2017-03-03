/*s: devices/sys/arm/devarch.c */
#include "u.h"
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

#include "io.h"

/*s: enum _anon_ (devices/sys/arm/devarch.c)(arm) */
enum {
    Qdir = 0,
    Qbase,

    Qmax = 16,
};
/*e: enum _anon_ (devices/sys/arm/devarch.c)(arm) */

typedef long Rdwrfn(Chan*, void*, long, vlong);

/*s: global readfn(arm) */
static Rdwrfn *readfn[Qmax];
/*e: global readfn(arm) */
/*s: global writefn(arm) */
static Rdwrfn *writefn[Qmax];
/*e: global writefn(arm) */

/*s: global archdir(arm) */
static Dirtab archdir[Qmax] = {
    ".",        { Qdir, 0, QTDIR }, 0,  0555,
};
/*e: global archdir(arm) */

/*s: global archwlock(arm) */
Lock archwlock; /* the lock is only for changing archdir */
/*e: global archwlock(arm) */
/*s: global narchdir(arm) */
int narchdir = Qbase;
/*e: global narchdir(arm) */

/*s: function addarchfile(arm) */
/*
 * Add a file to the #P listing.  Once added, you can't delete it.
 * You can't add a file with the same name as one already there,
 * and you get a pointer to the Dirtab entry so you can do things
 * like change the Qid version.  Changing the Qid path is disallowed.
 */
Dirtab*
addarchfile(char *name, int perm, Rdwrfn *rdfn, Rdwrfn *wrfn)
{
    int i;
    Dirtab d;
    Dirtab *dp;

    memset(&d, 0, sizeof d);
    strcpy(d.name, name);
    d.perm = perm;

    lock(&archwlock);
    if(narchdir >= Qmax){
        unlock(&archwlock);
        return nil;
    }

    for(i=0; i<narchdir; i++)
        if(strcmp(archdir[i].name, name) == 0){
            unlock(&archwlock);
            return nil;
        }

    d.qid.path = narchdir;
    archdir[narchdir] = d;
    readfn[narchdir] = rdfn;
    writefn[narchdir] = wrfn;
    dp = &archdir[narchdir++];
    unlock(&archwlock);

    return dp;
}
/*e: function addarchfile(arm) */

/*s: function archattach(arm) */
static Chan*
archattach(char* spec)
{
    return devattach('P', spec);
}
/*e: function archattach(arm) */

/*s: function archwalk(arm) */
Walkqid*
archwalk(Chan* c, Chan *nc, char** name, int nname)
{
    return devwalk(c, nc, name, nname, archdir, narchdir, devgen);
}
/*e: function archwalk(arm) */

/*s: function archstat(arm) */
static int
archstat(Chan* c, uchar* dp, int n)
{
    return devstat(c, dp, n, archdir, narchdir, devgen);
}
/*e: function archstat(arm) */

/*s: function archopen(arm) */
static Chan*
archopen(Chan* c, int omode)
{
    return devopen(c, omode, archdir, narchdir, devgen);
}
/*e: function archopen(arm) */

/*s: function archclose(arm) */
static void
archclose(Chan*)
{
}
/*e: function archclose(arm) */

/*s: function archread(arm) */
static long
archread(Chan *c, void *a, long n, vlong offset)
{
    Rdwrfn *fn;

    switch((ulong)c->qid.path){
    case Qdir:
        return devdirread(c, a, n, archdir, narchdir, devgen);

    default:
        if(c->qid.path < narchdir && (fn = readfn[c->qid.path]))
            return fn(c, a, n, offset);
        error(Eperm);
        break;
    }

    return 0;
}
/*e: function archread(arm) */

/*s: function archwrite(arm) */
static long
archwrite(Chan *c, void *a, long n, vlong offset)
{
    Rdwrfn *fn;

    if(c->qid.path < narchdir && (fn = writefn[c->qid.path]))
        return fn(c, a, n, offset);
    error(Eperm);

    return 0;
}
/*e: function archwrite(arm) */

void archinit(void);

/*s: global archdevtab(arm) */
Dev archdevtab = {
    .dc = 'P',
    .name = "arch",

    .reset = devreset,
    .init = archinit,
    .shutdown = devshutdown,
    .attach = archattach,
    .walk = archwalk,
    .stat = archstat,
    .open = archopen,
    .create = devcreate,
    .close = archclose,
    .read = archread,
    .bread = devbread,
    .write = archwrite,
    .bwrite = devbwrite,
    .remove = devremove,
    .wstat = devwstat,
};
/*e: global archdevtab(arm) */

/*s: function cputyperead(arm) */
static long
cputyperead(Chan*, void *a, long n, vlong offset)
{
    char name[64], str[128];

    cputype2name(name, sizeof name);
    snprint(str, sizeof str, "ARM %s %d\n", name, cpu->cpumhz);
    return readstr(offset, a, n, str);
}
/*e: function cputyperead(arm) */

/*s: function cputempread(arm) */
static long
cputempread(Chan*, void *a, long n, vlong offset)
{
    char str[16];

    snprint(str, sizeof str, "%ud\n", (getcputemp()+500)/1000);
    return readstr(offset, a, n, str);
}
/*e: function cputempread(arm) */

/*s: function archinit(arm) */
void
archinit(void)
{
    addarchfile("cputype", 0444, cputyperead, nil);
    addarchfile("cputemp", 0444, cputempread, nil);
}
/*e: function archinit(arm) */
/*e: devices/sys/arm/devarch.c */
