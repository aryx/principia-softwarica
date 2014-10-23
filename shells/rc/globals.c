/*s: globals.c */
#include "rc.h"

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
/*e: globals.c */
