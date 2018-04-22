/*s: networking/exportfs/globals.c */
#include <u.h>
#include <libc.h>
#include <auth.h>
#include <fcall.h>
#include <libsec.h>

#include "exportfs.h"

/*s: global [[Workq]] */
Fsrpc	*Workq;
/*e: global [[Workq]] */
/*s: global [[dbg]]([[(networking/exportfs/globals.c)]]) */
int  	dbg;
/*e: global [[dbg]]([[(networking/exportfs/globals.c)]]) */
/*s: global [[root]] */
File	*root;
/*e: global [[root]] */
/*s: global [[psmpt]] */
File	*psmpt;
/*e: global [[psmpt]] */
/*s: global [[fhash]] */
Fid	**fhash;
/*e: global [[fhash]] */
/*s: global [[fidfree]] */
Fid	*fidfree;
/*e: global [[fidfree]] */
/*s: global [[Proclist]] */
Proc	*Proclist;
/*e: global [[Proclist]] */
/*s: global [[psmap]] */
char	psmap[Npsmpt];
/*e: global [[psmap]] */
/*s: global [[qidtab]] */
Qidtab	*qidtab[Nqidtab];
/*e: global [[qidtab]] */
/*s: global [[messagesize]]([[(networking/exportfs/globals.c)]]) */
ulong	messagesize;
/*e: global [[messagesize]]([[(networking/exportfs/globals.c)]]) */
/*s: global [[Enomem]]([[(networking/exportfs/globals.c)]]) */
char	Enomem[];
/*e: global [[Enomem]]([[(networking/exportfs/globals.c)]]) */
/*s: global [[srvfd]]([[(networking/exportfs/globals.c)]]) */
int	srvfd;
/*e: global [[srvfd]]([[(networking/exportfs/globals.c)]]) */
/*s: global [[patternfile]]([[(networking/exportfs/globals.c)]]) */
char*	patternfile;
/*e: global [[patternfile]]([[(networking/exportfs/globals.c)]]) */
/*e: networking/exportfs/globals.c */
