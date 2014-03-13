#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"

// non functional properties backward dependencies breaker
// (logging, security, error, profiling/timing)

// console/devcons.c
int (*print)(char*, ...) = 0;
int (*iprint)(char*, ...) = 0;
int (*pprint)(char *fmt, ...) = 0;

void (*panic)(char*, ...) = 0;
void (*_assert)(char *fmt) = 0;

// process/386/trap.c
void (*dumpstack)(void) = 0;
// process/proc.c
void (*dumpaproc)(Proc*) = 0;

// security/auth.c
char	*eve;
iseve(void) { return strcmp(eve, up->user) == 0; }

// process/proc.c
void (*error)(char*) = 0;
void (*nexterror)(void) = 0;

Proc* (*wakeup)(Rendez*) = 0;
void (*sched)(void) = 0;
void (*ready)(Proc*) = 0;
void (*sleep)(Rendez*, int(*)(void*), void*) = 0;
void (*tsleep)(Rendez*, int (*)(void*), void*, ulong) = 0;

void (*proctrace)(Proc*, int, vlong) = 0;
Proc* (*proctab)(int) = 0;
int (*postnote)(Proc*, int, char*, int) = 0;
void (*pexit)(char*, int) = 0;

//process/sysproc.c
int (*return0)(void*) = 0;

// files/chan.c
void (*cclose)(Chan*);


// conf/pcf.c
Dev** devtab = 0;

// init/main.c
Mach *m;
Conf conf;
char* (*getconf)(char *name) = 0;
void (*exit)(int ispanic) = 0;

//misc/386/devarch.c
void (*coherence)(void) = 0;

//misc/386/devarch.c
uvlong		(*fastticks)(uvlong*) = 0;
