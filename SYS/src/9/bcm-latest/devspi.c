/*
 * minimal spi interface for testing
 */

#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"
extern int qstate(Queue*);

enum {
	QMAX		= 64*1024,
	Nspislave	= 2,
};

typedef struct Spi Spi;

struct Spi {
	int	csel;
	int	opens;
	QLock;
	Queue	*iq;
	Queue	*oq;
};

Spi spidev[Nspislave];

enum{
	Qdir = 0,
	Qspi,
};

Dirtab spidir[]={
	".",	{Qdir, 0, QTDIR},	0,	0555,
	"spi0",		{Qspi+0, 0},	0,	0664,
	"spi1",		{Qspi+1, 0}, 0, 0664,
};

#define DEVID(path)	((ulong)path - Qspi)

static void
spikick(void *a)
{
	Block *b;
	Spi *spi;

	spi = a;
	b = qget(spi->oq);
	if(b == nil)
		return;
	if(waserror()){
		freeb(b);
		nexterror();
	}
	spirw(spi->csel, b->rp, BLEN(b));
	qpass(spi->iq, b);
	poperror();
}

static void
spiinit(void)
{
}

static long
spiread(Chan *c, void *a, long n, vlong)
{
	Spi *spi;

	if(c->qid.type & QTDIR)
		return devdirread(c, a, n, spidir, nelem(spidir), devgen);

	spi = &spidev[DEVID(c->qid.path)];
	n = qread(spi->iq, a, n);

	return n;
}

static long
spiwrite(Chan*c, void *a, long n, vlong)
{
	Spi *spi;

	if(c->qid.type & QTDIR)
		error(Eperm);

	spi = &spidev[DEVID(c->qid.path)];
	n = qwrite(spi->oq, a, n);

	return n;
}

static Chan*
spiattach(char* spec)
{
	return devattach(L'π', spec);
}

static Walkqid*	 
spiwalk(Chan* c, Chan *nc, char** name, int nname)
{
	return devwalk(c, nc, name, nname, spidir, nelem(spidir), devgen);
}

static int	 
spistat(Chan* c, uchar* dp, int n)
{
	return devstat(c, dp, n, spidir, nelem(spidir), devgen);
}

static Chan*
spiopen(Chan* c, int omode)
{
	Spi *spi;

	c = devopen(c, omode, spidir, nelem(spidir), devgen);
	if(c->qid.type & QTDIR)
		return c;

	spi = &spidev[DEVID(c->qid.path)];
	qlock(spi);
	if(spi->opens++ == 0){
		spi->csel = DEVID(c->qid.path);
		if(spi->iq == nil)
			spi->iq = qopen(QMAX, 0, nil, nil);
		else
			qreopen(spi->iq);
		if(spi->oq == nil)
			spi->oq = qopen(QMAX, Qkick, spikick, spi);
		else
			qreopen(spi->oq);
	}
	qunlock(spi);
	c->iounit = qiomaxatomic;
	return c;
}

static void	 
spiclose(Chan *c)
{
	Spi *spi;

	if(c->qid.type & QTDIR)
		return;
	if((c->flag & COPEN) == 0)
		return;
	spi = &spidev[DEVID(c->qid.path)];
	qlock(spi);
	if(--spi->opens == 0){
		qclose(spi->iq);
		qhangup(spi->oq, nil);
		qclose(spi->oq);
	}
	qunlock(spi);
}

Dev spidevtab = {
	L'π',
	"spi",

	devreset,
	spiinit,
	devshutdown,
	spiattach,
	spiwalk,
	spistat,
	spiopen,
	devcreate,
	spiclose,
	spiread,
	devbread,
	spiwrite,
	devbwrite,
	devremove,
	devwstat,
};

