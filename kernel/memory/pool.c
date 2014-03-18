#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"error.h"

#include	<pool.h>

static void poolprint(Pool*, char*, ...);
static void ppanic(Pool*, char*, ...);
static void plock(Pool*);
static void punlock(Pool*);

typedef struct Private	Private;
struct Private {
	Lock		lk;
	char		msg[256];	/* a rock for messages to be printed at unlock */
};

static Private pmainpriv;
static Pool pmainmem = {
	.name=	"Main",
	.maxsize=	4*1024*1024,
	.minarena=	128*1024,
	.quantum=	32,
	.alloc=	xalloc,
	.merge=	xmerge,
	.flags=	POOL_TOLERANCE,

	.lock=	plock,
	.unlock=	punlock,
	.print=	poolprint,
	.panic=	ppanic,

	.private=	&pmainpriv,
};

static Private pimagpriv;
static Pool pimagmem = {
	.name=	"Image",
	.maxsize=	16*1024*1024,
	.minarena=	2*1024*1024,
	.quantum=	32,
	.alloc=	xalloc,
	.merge=	xmerge,
	.flags=	0,

	.lock=	plock,
	.unlock=	punlock,
	.print=	poolprint,
	.panic=	ppanic,

	.private=	&pimagpriv,
};

// include/pool.h
Pool*	mainmem = &pmainmem;
Pool*	imagmem = &pimagmem;


/*
 * because we can't print while we're holding the locks, 
 * we have the save the message and print it once we let go.
 */
static void
poolprint(Pool *p, char *fmt, ...)
{
	va_list v;
	Private *pv;

	pv = p->private;
	va_start(v, fmt);
	vseprint(pv->msg+strlen(pv->msg), pv->msg+sizeof pv->msg, fmt, v);
	va_end(v);
}

static void
ppanic(Pool *p, char *fmt, ...)
{
	va_list v;
	Private *pv;
	char msg[sizeof pv->msg];

	pv = p->private;
	va_start(v, fmt);
	vseprint(pv->msg+strlen(pv->msg), pv->msg+sizeof pv->msg, fmt, v);
	va_end(v);
	memmove(msg, pv->msg, sizeof msg);
	iunlock(&pv->lk);
	panic("%s", msg);
}

static void
plock(Pool *p)
{
	Private *pv;

	pv = p->private;
	ilock(&pv->lk);
	pv->lk.pc = getcallerpc(&p);
	pv->msg[0] = 0;
}

static void
punlock(Pool *p)
{
	Private *pv;
	char msg[sizeof pv->msg];

	pv = p->private;
	if(pv->msg[0] == 0){
		iunlock(&pv->lk);
		return;
	}

	memmove(msg, pv->msg, sizeof msg);
	iunlock(&pv->lk);
	iprint("%.*s", sizeof pv->msg, msg);
}

void
poolsummary(Pool *p)
{
	print("%s max %lud cur %lud free %lud alloc %lud\n", p->name,
		p->maxsize, p->cursize, p->curfree, p->curalloc);
}
