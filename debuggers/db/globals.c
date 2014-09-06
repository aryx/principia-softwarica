/*s: db/globals.c */
#include "defs.h"

/*s: global wtflag */
// was in main.c
int wtflag = OREAD;
/*e: global wtflag */
/*s: global kflag */
BOOL kflag;
/*e: global kflag */

/*s: global mkfault */
BOOL mkfault;
/*e: global mkfault */
/*s: global maxoff */
ADDR maxoff;
/*e: global maxoff */

/*s: global xargc */
int xargc;		/* bullshit */
/*e: global xargc */

/*s: global errmsg */
// was static in main.c
char *errmsg;
/*e: global errmsg */
/*s: global env */
jmp_buf env;
/*e: global env */

/*s: global ending */
// was in runpcs.c
int ending;
/*e: global ending */
/*s: global pid */
int pid;
/*e: global pid */
/*s: global nnote */
int nnote;
/*e: global nnote */
/*s: global note */
char note[NNOTE][ERRMAX];
/*e: global note */
/*s: global bkpthead */
BKPT *bkpthead;
/*e: global bkpthead */

/*s: global adrval */
// was in command.c
WORD	adrval;
/*e: global adrval */
/*s: global adrflg */
int	adrflg;
/*e: global adrflg */

/*s: global expv */
// was in expr.c
uvlong	expv;
/*e: global expv */

/*s: global pcsactive */
// was in trcrun.c
int pcsactive = 0;
/*e: global pcsactive */

/*s: global symmap */
// was in setup.c
Map	*symmap;
/*e: global symmap */
/*s: global cormap */
Map	*cormap;
/*e: global cormap */

/*s: global dot */
// was in command.c
ADDR	dot;
/*e: global dot */
/*s: global dotinc */
int	dotinc;
/*e: global dotinc */
/*s: global ditto */
ADDR	ditto;
/*e: global ditto */
/*s: global cntval */
WORD cntval;
/*e: global cntval */
/*s: global cntflg */
int	cntflg;
/*e: global cntflg */
/*e: db/globals.c */
