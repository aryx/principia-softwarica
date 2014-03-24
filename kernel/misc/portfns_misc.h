
// cis.c
// pcmcisread in fns.h

// parse.c
void		cmderror(Cmdbuf*, char*);
Cmdbuf*		parsecmd(char *a, int n);
Cmdtab*		lookupcmd(Cmdbuf*, Cmdtab*, int);

// print.c
//dead?

// random.c
ulong		randomread(void*, ulong);
void		randominit(void);

// rebootcmd.c
//TODO does not show in cg, but not empty file so weird
//void		readn(Chan *, void *, long);
void		rebootcmd(int, char**);
