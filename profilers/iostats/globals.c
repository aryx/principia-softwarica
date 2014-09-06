#include <u.h>
#include <libc.h>
#include <auth.h>
#include <fcall.h>

#include "statfs.h"

Fsrpc	*Workq;
int  	dbg;
File	*root;
Fid	**fhash;
Fid	*fidfree;
int	qid;
Proc	*Proclist;
int	done;
Stats	*stats;
Frec	*frhead;
Frec	*frtail;
int	myiounit;
