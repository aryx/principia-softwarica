#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"

// non functional properties backward dependencies breaker
// (logging, security, error)
// todo? profiling?

// devcons.c
int		(*print)(char*, ...) = 0;
int		(*iprint)(char*, ...) = 0;
int             (*pprint)(char *fmt, ...) = 0;
void		(*panic)(char*, ...) = 0;
void (*_assert)(char *fmt) = 0;

//trap.c
void (*dumpstack)(void) = 0;

//auth.c
char	*eve;
iseve(void) { return strcmp(eve, up->user) == 0; }

// proc.c
void		(*error)(char*) = 0;
void		(*nexterror)(void) = 0;

Proc*		(*wakeup)(Rendez*) = 0;
void		(*sched)(void) = 0;
void		(*ready)(Proc*) = 0;
void		(*sleep)(Rendez*, int(*)(void*), void*) = 0;


// pcf.c
Dev** devtab = 0;

//main.c
Mach *m;
Conf conf;
char* (*getconf)(char *name) = 0;
void (*exit)(int ispanic) = 0;

