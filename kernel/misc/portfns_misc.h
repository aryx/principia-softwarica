
// cis.c
// pcmcisread in fns.h

// parse.c
void		cmderror(Cmdbuf*, char*);
Cmdbuf*		parsecmd(char *a, int n);
Cmdtab*		lookupcmd(Cmdbuf*, Cmdtab*, int);

// random.c
ulong		randomread(void*, ulong);
void		randominit(void);

// rebootcmd.c
//void		readn(Chan *, void *, long);
void		rebootcmd(int, char**);
