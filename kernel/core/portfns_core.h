/*s: portfns_core.h */

// exported in lib.h, defined in portfns.c
// (*print)

// exported in lib.h, defined in 386/fns.c (but used in port)
// int  cistrcmp(char*, char*);
// int  cistrncmp(char*, char*, int);

// portfns.c (mostly here just to remove some backward dependencies)
// console/devcons.c
int   (*iprint)(char*, ...);
int   (*pprint)(char*, ...);
void    (*panic)(char*, ...);
void    (*_assert)(char*);
// process/386/trap.c
void    (*dumpstack)(void);
// process/proc.c
void    (*dumpaproc)(Proc*);
// process/proc.c
void    (*error)(char*);
void    (*nexterror)(void);
// process/proc.c
void    (*sleep)(Rendez*, int(*)(void*), void*);
void    (*tsleep)(Rendez*, int (*)(void*), void*, ulong);
Proc*   (*wakeup)(Rendez*);
void    (*sched)(void);
void    (*ready)(Proc*);
// process/proc.c
Proc*   (*proctab)(int);
int     (*postnote)(Proc*, int, char*, int);
void    (*pexit)(char*, int);
//process/sysproc.c
int   (*return0)(void*);
// files/chan.c
void    (*cclose)(Chan*);
// init/main.c
void    (*exit)(int);
//TODO?char* (*getconf)(char *name) = 0;
//misc/386/devarch.c
void  (*coherence)(void);
//misc/386/devarch.c
uvlong    (*fastticks)(uvlong*);
// processes/386/i8253.c
void    (*microdelay)(int);
void    (*delay)(int);

// portfns.c
int   readnum(ulong, char*, ulong, ulong, int);
int   readstr(ulong, char*, ulong, char*);

#pragma varargck argpos iprint  1
#pragma varargck argpos pprint  1
#pragma varargck argpos panic 1
/*e: portfns_core.h */
