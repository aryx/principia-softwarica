/*s: rc/globals.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

// was in rc.h
/*s: global [[mypid]] */
int mypid;
/*e: global [[mypid]] */
/*s: global [[promptstr]] */
char *promptstr;
/*e: global [[promptstr]] */
/*s: global [[ndot]] */
/*
 * How many dot commands have we executed?
 * Used to ensure that -v flag doesn't print rcmain.
 */
int ndot;
/*e: global [[ndot]] */

// was in exec.h
/*s: global [[runq]] */
// stack<ref_own<Thread>> (next = Thread.ret)
thread *runq;
/*e: global [[runq]] */
/*s: global [[codebuf]] */
// growing_array<ref_own<Code>>
code *codebuf;				/* compiler output */
/*e: global [[codebuf]] */
/*s: global [[eflagok]] */
bool eflagok;	/* kludge flag so that -e doesn't exit in startup */
/*e: global [[eflagok]] */

/*e: rc/globals.c */
