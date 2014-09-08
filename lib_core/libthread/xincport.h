#include <u.h>
#include <libc.h>
#include <thread.h>

static Lock xincport_lock;

void
_xinc(long *p)
{

	lock(&xincport_lock);
	(*p)++;
	unlock(&xincport_lock);
}

long
_xdec(long *p)
{
	long r;

	lock(&xincport_lock);
	r = --(*p);
	unlock(&xincport_lock);
	return r;
}
