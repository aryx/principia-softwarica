#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"

enum{
	Qdir,
	Qmisc,
};

static Dirtab sysdir[]={
	".",	{Qdir, 0, QTDIR},	0,		DMDIR|0555,
	"misc",	{Qmisc},	0,		0444,
};

static void
sysinit(void)
{
}

static Chan*
sysattach(char *spec)
{
	return devattach('k', spec);
}

static Walkqid*
syswalk(Chan *c, Chan *nc, char **name, int nname)
{
	return devwalk(c, nc, name, nname, sysdir, nelem(sysdir), devgen);
}

static int
sysstat(Chan *c, uchar *dp, int n)
{
	return devstat(c, dp, n, sysdir, nelem(sysdir), devgen);
}

static Chan*
sysopen(Chan *c, int omode)
{
	c->aux = nil;
	c = devopen(c, omode, sysdir, nelem(sysdir), devgen);
	switch((ulong)c->qid.path){
	}
	return c;
}

static void
sysclose(Chan *c)
{
	switch((ulong)c->qid.path){
	}
}

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
	return -1;		/* never reached */
}

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
