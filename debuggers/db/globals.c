#include "defs.h"

// was in main.c
int wtflag = OREAD;
BOOL kflag;

BOOL mkfault;
ADDR maxoff;

int xargc;		/* bullshit */

// was static in main.c
char *errmsg;
jmp_buf env;

// was in runpcs.c
int ending;
int pid;
int nnote;
char note[NNOTE][ERRMAX];
BKPT *bkpthead;

// was in command.c
WORD	adrval;
int	adrflg;

// was in expr.c
uvlong	expv;

// was in trcrun.c
int pcsactive = 0;

// was in setup.c
Map	*symmap;
Map	*cormap;

// was in command.c
ADDR	dot;
int	dotinc;
ADDR	ditto;
WORD cntval;
int	cntflg;
