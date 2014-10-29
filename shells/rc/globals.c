/*s: globals.c */
#include "rc.h"
#include "exec.h"

// was in rc.h
/*s: global tok */
char tok[NTOK + UTFmax];
/*e: global tok */
/*s: global lastc */
int lastc;
/*e: global lastc */
/*s: global mypid */
int mypid;
/*e: global mypid */
/*s: global gvar */
// hash<string, ref<Var>> (next = Var.next in bucket list)
var *gvar[NVAR];		/* hash for globals */
/*e: global gvar */
/*s: global ndot */
/*
 * How many dot commands have we executed?
 * Used to ensure that -v flag doesn't print rcmain.
 */
int ndot;
/*e: global ndot */
/*s: global promptstr */
char *promptstr;
/*e: global promptstr */
/*s: global nerror */
int nerror;		/* number of errors encountered during compilation */
/*e: global nerror */
/*s: global err */
io *err;
/*e: global err */

// was in exec.h
/*s: global runq */
// list<ref_own<Thread>> (next = Thread.ret)
thread *runq;
/*e: global runq */
/*s: global codebuf */
code *codebuf;				/* compiler output */
/*e: global codebuf */
/*s: global ntrap */
int ntrap;				/* number of outstanding traps */
/*e: global ntrap */
/*s: global trap */
int trap[NSIG];				/* number of outstanding traps per type */
/*e: global trap */
/*s: global eflagok */
bool eflagok;	/* kludge flag so that -e doesn't exit in startup */
/*e: global eflagok */

/*e: globals.c */
