/*s: error.h */
extern char Emount[];   /* inconsistent mount */
extern char Eunmount[];   /* not mounted */
extern char Eismtpt[];    /* is a mount point */
extern char Eunion[];   /* not in union */
extern char Emountrpc[];  /* mount rpc error */
extern char Eshutdown[];  /* device shut down */
extern char Enocreate[];  /* mounted directory forbids creation */
extern char Enonexist[];  /* file does not exist */
extern char Eexist[];   /* file already exists */
extern char Ebadsharp[];  /* unknown device in # filename */
extern char Enotdir[];    /* not a directory */
extern char Eisdir[];   /* file is a directory */
extern char Ebadchar[];   /* bad character in file name */
extern char Efilename[];  /* file name syntax */
extern char Eperm[];    /* permission denied */
extern char Ebadusefd[];  /* inappropriate use of fd */
extern char Ebadarg[];    /* bad arg in system call */
extern char Einuse[];   /* device or object already in use */
extern char Eio[];    /* i/o error */
extern char Etoobig[];    /* read or write too large */
extern char Etoosmall[];  /* read or write too small */
extern char Ehungup[];    /* i/o on hungup channel */
extern char Ebadctl[];    /* bad process or channel control request */
extern char Enodev[];   /* no free devices */
extern char Eprocdied[];  /* process exited */
extern char Enochild[];   /* no living children */
extern char Eioload[];    /* i/o error in demand load */
extern char Enovmem[];    /* virtual memory allocation failed */
extern char Ebadfd[];   /* fd out of range or not open */
extern char Enofd[];    /* no free file descriptors */
extern char Eisstream[];  /* seek on a stream */
extern char Ebadexec[];   /* exec header invalid */
extern char Etimedout[];  /* connection timed out */
extern char Econrefused[];  /* connection refused */
extern char Econinuse[];  /* connection in use */
extern char Eintr[];    /* interrupted */
extern char Enomem[];   /* kernel allocate failed */
extern char Esoverlap[];  /* segments overlap */
extern char Eshort[];   /* i/o count too small */
extern char Egreg[];    /* jmk added reentrancy for threads */
extern char Ebadspec[];   /* bad attach specifier */
extern char Enoreg[];   /* process has no saved registers */
extern char Enoattach[];  /* mount/attach disallowed */
extern char Eshortstat[]; /* stat buffer too small */
extern char Ebadstat[];   /* malformed stat buffer */
extern char Enegoff[];    /* negative i/o offset */
extern char Ecmdargs[];   /* wrong #args in control message */
extern char Ebadip[];   /* bad ip address syntax */
extern char Edirseek[];   /* seek in directory */
extern char Echange[];    /* media or partition has changed */
extern char Edetach[];    /* device is detached */
extern char Enotconf[];   /* endpoint not configured */
extern char Estalled[];   /* endpoint stalled */
extern char Esbadstat[];  /* invalid directory entry received from server */
extern char Enoversion[]; /* version not established for mount channel */
/*e: error.h */
