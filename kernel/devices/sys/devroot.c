/*s: devroot.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

//*****************************************************************************
// Constants and types
//*****************************************************************************

/*s: devroot enum Qxxx */
enum
{
    Qdir = 0,
    Qboot = 0x1000,

    Nrootfiles = 32,
    Nbootfiles = 32,
};
/*e: devroot enum Qxxx */

/*s: devroot.c forward decl */
typedef struct Dirlist Dirlist;
/*e: devroot.c forward decl */
/*s: struct Dirlist */
struct Dirlist
{
    uint base; // for unique qids
    Dirtab *dir;
    uchar **data;
    int ndir; // number of dir used
    int mdir; // max dir entries
};
/*e: struct Dirlist */

//*****************************************************************************
// Root
//*****************************************************************************

// note: directories have by convention 0 length

/*s: globals rootdir, rootdata, rootlist */
static Dirtab rootdir[Nrootfiles] = {
  {
    .name = "#/",
    .qid = {Qdir, 0, QTDIR},
    .length = 0,
    .perm = DMDIR|0555,
  },
  {    
    .name = "boot", 
    .qid = {Qboot, 0, QTDIR},
    .length = 0,
    .perm = DMDIR|0555,
  }
};
static uchar *rootdata[Nrootfiles];
static Dirlist rootlist = 
{
  .base = 0,
  .dir = rootdir,
  .data = rootdata,
  .ndir = 2,
  .mdir = Nrootfiles
};
/*e: globals rootdir, rootdata, rootlist */

//*****************************************************************************
// Boot
//*****************************************************************************

/*s: globals bootdir, bootdata, bootlist */
static Dirtab bootdir[Nbootfiles] = {
  {
    .name = "boot",
    .qid = {Qboot, 0, QTDIR},
    .length = 0,
    .perm = DMDIR|0555,
  }
};

static uchar *bootdata[Nbootfiles];
static Dirlist bootlist =
{
    .base = Qboot,
    .dir = bootdir,
    .data = bootdata,
    .ndir = 1,
    .mdir = Nbootfiles
};
/*e: globals bootdir, bootdata, bootlist */

//*****************************************************************************
// Functions
//*****************************************************************************

/*s: function addlist */
/*
 *  add a file to the list
 */
static void
addlist(Dirlist *l, char *name, uchar *contents, ulong len, int perm)
{
    Dirtab *d;

    if(l->ndir >= l->mdir)
        panic("too many root files");
    l->data[l->ndir] = contents;
    d = &l->dir[l->ndir];
    strcpy(d->name, name);
    d->length = len;
    d->perm = perm;
    d->qid.type = 0;
    d->qid.vers = 0;
    d->qid.path = ++l->ndir + l->base;
    if(perm & DMDIR)
        d->qid.type |= QTDIR;
}
/*e: function addlist */

/*s: function addbootfile */
/*
 *  add a boot file
 */
void
addbootfile(char *name, uchar *contents, ulong len)
{
    addlist(&bootlist, name, contents, len, 0555);
}
/*e: function addbootfile */

/*s: function addrootdir */
/*
 *  add a root directory
 */
static void
addrootdir(char *name)
{
    addlist(&rootlist, name, nil, 0, DMDIR|0555);
}
/*e: function addrootdir */

/*s: method rootreset */
static void
rootreset(void)
{
    addrootdir("bin");
    addrootdir("dev");
    addrootdir("env");
    addrootdir("fd");
    addrootdir("mnt");
    addrootdir("net");
    addrootdir("net.alt");
    addrootdir("proc");
    addrootdir("root");
    addrootdir("srv");
}
/*e: method rootreset */

static Chan*
rootattach(char *spec)
{
    return devattach('/', spec);
}

/*s: function rootgen */
static int
rootgen(Chan *c, char *name, Dirtab*, int, int s, Dir *dp)
{
    int t;
    Dirtab *d;
    Dirlist *l;

    switch((int)c->qid.path){
    case Qdir:
        if(s == DEVDOTDOT){
            devdir(c, (Qid){Qdir, 0, QTDIR}, "#/", 0, eve, 0555, dp);
            return 1;
        }
        return devgen(c, name, rootlist.dir, rootlist.ndir, s, dp);
    case Qboot:
        if(s == DEVDOTDOT){
            devdir(c, (Qid){Qdir, 0, QTDIR}, "#/", 0, eve, 0555, dp);
            return 1;
        }
        return devgen(c, name, bootlist.dir, bootlist.ndir, s, dp);
    default:
        if(s == DEVDOTDOT){
            if((int)c->qid.path < Qboot)
                devdir(c, (Qid){Qdir, 0, QTDIR}, "#/", 0, eve, 0555, dp);
            else
                devdir(c, (Qid){Qboot, 0, QTDIR}, "#/", 0, eve, 0555, dp);
            return 1;
        }
        if(s != 0)
            return -1;
        if((int)c->qid.path < Qboot){
            t = c->qid.path-1;
            l = &rootlist;
        }else{
            t = c->qid.path - Qboot - 1;
            l = &bootlist;
        }
        if(t >= l->ndir)
            return -1;
        d = &l->dir[t];
        devdir(c, d->qid, d->name, d->length, eve, d->perm, dp);
        return 1;
    }
}
/*e: function rootgen */

static Walkqid*
rootwalk(Chan *c, Chan *nc, char **name, int nname)
{
    return devwalk(c,  nc, name, nname, nil, 0, rootgen);
}

static int
rootstat(Chan *c, uchar *dp, int n)
{
    return devstat(c, dp, n, nil, 0, rootgen);
}

static Chan*
rootopen(Chan *c, int omode)
{
    return devopen(c, omode, nil, 0, devgen);
}

/*s: method rootclose */
/*
 * sysremove() knows this is a nop
 */
static void
rootclose(Chan*)
{
}
/*e: method rootclose */

/*s: method rootread */
static long
rootread(Chan *c, void *buf, long n, vlong off)
{
    ulong t;
    Dirtab *d;
    Dirlist *l;
    uchar *data;
    ulong offset = off;

    t = c->qid.path;
    switch(t){
    case Qdir:
    case Qboot:
        return devdirread(c, buf, n, nil, 0, rootgen);
    }

    if(t<Qboot)
        l = &rootlist;
    else{
        t -= Qboot;
        l = &bootlist;
    }

    t--;
    if(t >= l->ndir)
        error(Egreg);

    d = &l->dir[t];
    data = l->data[t];
    if(offset >= d->length)
        return 0;
    if(offset+n > d->length)
        n = d->length - offset;
#ifdef asdf
print("[%d] kaddr %.8ulx base %.8ulx offset %ld (%.8ulx), n %d %.8ulx %.8ulx %.8ulx\n", 
        t, buf, data, offset, offset, n,
        ((ulong*)(data+offset))[0],
        ((ulong*)(data+offset))[1],
        ((ulong*)(data+offset))[2]);
#endif asdf
    memmove(buf, data+offset, n);
    return n;
}
/*e: method rootread */

/*s: method rootwrite */
static long
rootwrite(Chan*, void*, long, vlong)
{
    error(Egreg);
    return 0;
}
/*e: method rootwrite */

/*s: global rootdevtab */
Dev rootdevtab = {
    .dc       = '/',
    .name     = "root",
  
    .reset    = rootreset,
    .init     = devinit,
    .shutdown = devshutdown,
    .attach   = rootattach,
    .walk     = rootwalk,
    .stat     = rootstat,
    .open     = rootopen,
    .create   = devcreate,
    .close    = rootclose,
    .read     = rootread,
    .bread    = devbread,
    .write    = rootwrite,
    .bwrite   = devbwrite,
    .remove   = devremove,
    .wstat    = devwstat,
};
/*e: global rootdevtab */

/*e: devroot.c */
