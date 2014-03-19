
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

// rdb.c
void		rdb(void);

