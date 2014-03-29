#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"

//pad: was in chan.c
// see also _incnt and _deccnt in tasklock.c
long
incref(Ref *r)
{
	long x;

	lock(r);
	x = ++r->ref;
	unlock(r);
	return x;
}

long
decref(Ref *r)
{
	long x;

	lock(r);
	x = --r->ref;
	unlock(r);
	if(x < 0)
		panic("decref pc=%#p", getcallerpc(&r));

	return x;
}
