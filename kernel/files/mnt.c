#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"

// this used to be in devmnt.c, but to avoid backward deps I've splitted
// this file in 2 (which forced to put more stuff in portdat_files.h though).

struct Mntalloc mntalloc;

void
freetag(int t)
{
	mntalloc.tagmask[t>>TAGSHIFT] &= ~(1<<(t&TAGMASK));
}

void
mntfree(Mntrpc *r)
{
	if(r->b != nil)
		freeblist(r->b);
	lock(&mntalloc);
	if(mntalloc.nrpcfree >= 10){
		free(r->rpc);
		freetag(r->request.tag);
		free(r);
	}
	else{
		r->list = mntalloc.rpcfree;
		mntalloc.rpcfree = r;
		mntalloc.nrpcfree++;
	}
	mntalloc.nrpcused--;
	unlock(&mntalloc);
}


void
mntpntfree(Mnt *m)
{
	Mnt *f, **l;
	Queue *q;

	lock(&mntalloc);
	l = &mntalloc.list;
	for(f = *l; f; f = f->list) {
		if(f == m) {
			*l = m->list;
			break;
		}
		l = &f->list;
	}
	m->list = mntalloc.mntfree;
	mntalloc.mntfree = m;
	q = m->q;
	unlock(&mntalloc);

	qfree(q);
}



void
muxclose(Mnt *m)
{
	Mntrpc *q, *r;

	for(q = m->queue; q; q = r) {
		r = q->list;
		mntfree(q);
	}
	m->id = 0;
	free(m->version);
	m->version = nil;
	mntpntfree(m);
}
