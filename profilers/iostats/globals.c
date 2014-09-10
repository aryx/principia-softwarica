/*s: profilers/iostats/globals.c */
#include <u.h>
#include <libc.h>
#include <auth.h>
#include <fcall.h>

#include "statfs.h"

/*s: global Workq */
Fsrpc	*Workq;
/*e: global Workq */
/*s: global dbg */
int  	dbg;
/*e: global dbg */
/*s: global root */
File	*root;
/*e: global root */
/*s: global fhash */
Fid	**fhash;
/*e: global fhash */
/*s: global fidfree */
Fid	*fidfree;
/*e: global fidfree */
/*s: global qid */
int	qid;
/*e: global qid */
/*s: global Proclist */
Proc	*Proclist;
/*e: global Proclist */
/*s: global done */
int	done;
/*e: global done */
/*s: global stats */
Stats	*stats;
/*e: global stats */
/*s: global frhead */
Frec	*frhead;
/*e: global frhead */
/*s: global frtail */
Frec	*frtail;
/*e: global frtail */
/*s: global myiounit */
int	myiounit;
/*e: global myiounit */
/*e: profilers/iostats/globals.c */
