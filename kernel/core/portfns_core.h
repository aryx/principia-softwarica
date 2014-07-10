/*s: portfns_core.h */

// exported in lib.h, defined in portfns.c
// (*print)

// exported in lib.h, defined in 386/fns.c (but used in port)
// int  cistrcmp(char*, char*);
// int  cistrncmp(char*, char*, int);

// portfns.c (mostly here just to remove some backward dependencies)
/*s: portfns_core.h backward deps breaker */
// console/devcons.c
int   (*iprint)(char*, ...);
int   (*pprint)(char*, ...);
void  (*panic)(char*, ...);
void  (*_assert)(char*);
/*x: portfns_core.h backward deps breaker */
// process/386/trap.c
void    (*dumpstack)(void);
/*x: portfns_core.h backward deps breaker */
// process/proc.c
void    (*dumpaproc)(Proc*);
/*x: portfns_core.h backward deps breaker */
// process/proc.c
void    (*error)(char*);
void    (*nexterror)(void);
/*x: portfns_core.h backward deps breaker */
// process/proc.c
void    (*sleep)(Rendez*, int(*)(void*), void*);
void    (*tsleep)(Rendez*, int (*)(void*), void*, ulong);
Proc*   (*wakeup)(Rendez*);
void    (*sched)(void);
void    (*ready)(Proc*);
/*x: portfns_core.h backward deps breaker */
// process/proc.c
Proc*   (*proctab)(int);
int     (*postnote)(Proc*, int, char*, int);
void    (*pexit)(char*, bool);
/*x: portfns_core.h backward deps breaker */
// files/chan.c
void    (*cclose)(Chan*);
/*x: portfns_core.h backward deps breaker */
// init/main.c
void    (*exit)(int);
/*x: portfns_core.h backward deps breaker */
//misc/386/devarch.c
void    (*coherence)(void);
uvlong  (*fastticks)(uvlong*);
/*x: portfns_core.h backward deps breaker */
// processes/386/i8253.c
void    (*microdelay)(int);
void    (*delay)(int);
/*e: portfns_core.h backward deps breaker */

// portfns.c
bool returnfalse(void*);
int   readnum(ulong, char*, ulong, ulong, int);
int   readstr(ulong, char*, ulong, char*);

/*s: portfns_core.h pragmas */
#pragma varargck argpos iprint  1
#pragma varargck argpos pprint  1
#pragma varargck argpos panic 1
/*e: portfns_core.h pragmas */
/*e: portfns_core.h */
