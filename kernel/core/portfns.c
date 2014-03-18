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




// process/proc.c
void (*error)(char*) = 0;
void (*nexterror)(void) = 0;

Proc* (*wakeup)(Rendez*) = 0;
void (*sched)(void) = 0;
void (*ready)(Proc*) = 0;
void (*sleep)(Rendez*, int(*)(void*), void*) = 0;
void (*tsleep)(Rendez*, int (*)(void*), void*, ulong) = 0;

Proc* (*proctab)(int) = 0;
int (*postnote)(Proc*, int, char*, int) = 0;
void (*pexit)(char*, int) = 0;

//process/sysproc.c
int (*return0)(void*) = 0;

// files/chan.c
void (*cclose)(Chan*);


// init/main.c
char* (*getconf)(char *name) = 0;
void (*exit)(int ispanic) = 0;

//misc/386/devarch.c
void (*coherence)(void) = 0;

//misc/386/devarch.c
uvlong		(*fastticks)(uvlong*) = 0;



// was in devcons.c, could be in lib/misc.c
int
readnum(ulong off, char *buf, ulong n, ulong val, int size)
{
	char tmp[64];

	snprint(tmp, sizeof(tmp), "%*lud", size-1, val);
	tmp[size-1] = ' ';
	if(off >= size)
		return 0;
	if(off+n > size)
		n = size-off;
	memmove(buf, tmp+off, n);
	return n;
}

int
readstr(ulong off, char *buf, ulong n, char *str)
{
	int size;

	size = strlen(str);
	if(off >= size)
		return 0;
	if(off+n > size)
		n = size-off;
	memmove(buf, str+off, n);
	return n;
}
