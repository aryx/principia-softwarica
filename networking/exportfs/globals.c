#include <u.h>
#include <libc.h>
#include <auth.h>
#include <fcall.h>
#include <libsec.h>

#include "exportfs.h"

Fsrpc	*Workq;
int  	dbg;
File	*root;
File	*psmpt;
Fid	**fhash;
Fid	*fidfree;
Proc	*Proclist;
char	psmap[Npsmpt];
Qidtab	*qidtab[Nqidtab];
ulong	messagesize;
char	Enomem[];
int	srvfd;
char*	patternfile;
