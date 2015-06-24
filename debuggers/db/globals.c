/*s: db/globals.c */
#include "defs.h"

/*s: global wtflag */
// was in main.c
int wtflag = OREAD;
/*e: global wtflag */
/*s: global kflag */
bool kflag;
/*e: global kflag */

/*s: global mkfault */
bool mkfault;
/*e: global mkfault */
/*s: global maxoff */
ADDR maxoff = MAXOFF;
/*e: global maxoff */

/*s: global errmsg */
// was static in main.c
char *errmsg;
/*e: global errmsg */
/*s: global env */
jmp_buf env;
/*e: global env */

/*s: global symmap */
// was in setup.c
Map	*symmap;
/*e: global symmap */
/*s: global cormap */
Map	*cormap;
/*e: global cormap */

/*s: global ending */
// was in runpcs.c
bool ending;
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
// list<BKPT>, next = BKPT.next
BKPT *bkpthead;
/*e: global bkpthead */

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

/*s: global adrval */
// was in command.c
WORD	adrval;
/*e: global adrval */
/*s: global adrflg */
bool	adrflg;
/*e: global adrflg */

/*s: global cntval */
WORD cntval;
/*e: global cntval */
/*s: global cntflg */
bool	cntflg;
/*e: global cntflg */

/*s: global expv */
// was in expr.c
uvlong	expv;
/*e: global expv */

/*s: global pcsactive */
// was in trcrun.c
bool pcsactive = false;
/*e: global pcsactive */


/*s: global xargc */
int xargc;		/* bullshit */
/*e: global xargc */

/*e: db/globals.c */
