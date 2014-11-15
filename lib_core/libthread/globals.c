#include <u.h>
#include <libc.h>
#include <thread.h>
#include "threadimpl.h"

// used to be in create.c
Pqueue _threadpq;

// used to be in main.c
static Proc **procp;

void
_systhreadinit(void)
{
	procp = privalloc();
}

Proc*
_threadgetproc(void)
{
	return *procp;
}

void
_threadsetproc(Proc *p)
{
	*procp = p;
}
