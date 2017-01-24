/*s: portfns_misc.h */

// parse.c
Cmdbuf*  parsecmd(char *a, int n);
Cmdtab*  lookupcmd(Cmdbuf*, Cmdtab*, int);
void     cmderror(Cmdbuf*, char*);

// random.c
void     randominit(void);
ulong    randomread(void*, ulong);

/*e: portfns_misc.h */
