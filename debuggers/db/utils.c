#include "defs.h"
#include "fns.h"

/*
 * An error occurred; save the message for later printing,
 * close open files, and reset to main command loop.
 */
void
error(char *n)
{
	errmsg = n;
	iclose(0, 1);
	oclose();
	flush();
	delbp();
	ending = 0;
	longjmp(env, 1);
}

void
errors(char *m, char *n)
{
	static char buf[128];

	sprint(buf, "%s: %s", m, n);
	error(buf);
}
