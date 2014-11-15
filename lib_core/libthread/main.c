/*s: lib_core/libthread/main.c */
#include <u.h>
#include <libc.h>
#include <thread.h>
#include "threadimpl.h"

typedef struct Mainarg Mainarg;

/*s: struct Mainarg */
struct Mainarg
{
    int		argc;
    char	**argv;
};
/*e: struct Mainarg */

/*s: global mainstacksize */
int	mainstacksize;
/*e: global mainstacksize */
/*s: global _threadnotefd */
int	_threadnotefd;
/*e: global _threadnotefd */
/*s: global _threadpasserpid */
int	_threadpasserpid;
/*e: global _threadpasserpid */

/*s: global _mainjmp */
static jmp_buf _mainjmp;
/*e: global _mainjmp */

static void mainlauncher(void*);
extern void (*_sysfatal)(char*, va_list);
extern void (*__assert)(char*);
extern int (*_dial)(char*, char*, char*, int*);

extern int _threaddial(char*, char*, char*, int*);

/*s: global mainp */
static Proc **mainp;
/*e: global mainp */

/*s: function main */
void
main(int argc, char **argv)
{
    Mainarg *a;
    Proc *p;

    rfork(RFREND);
    mainp = &p;
    if(setjmp(_mainjmp))
        _schedinit(p);

//_threaddebuglevel = (DBGSCHED|DBGCHAN|DBGREND)^~0;
    _systhreadinit();
    _qlockinit(_threadrendezvous);
    _sysfatal = _threadsysfatal;
    _dial = _threaddial;
    __assert = _threadassert;
    notify(_threadnote);
    if(mainstacksize == 0)
        mainstacksize = 8*1024;

    a = _threadmalloc(sizeof *a, 1);
    a->argc = argc;
    a->argv = argv;

    p = _newproc(mainlauncher, a, mainstacksize, "threadmain", 0, 0);
    _schedinit(p);
    abort();	/* not reached */
}
/*e: function main */

/*s: function mainlauncher */
static void
mainlauncher(void *arg)
{
    Mainarg *a;

    a = arg;
    threadmain(a->argc, a->argv);
    threadexits("threadmain");
}
/*e: function mainlauncher */

/*s: function skip */
static char*
skip(char *p)
{
    while(*p == ' ')
        p++;
    while(*p != ' ' && *p != 0)
        p++;
    return p;
}
/*e: function skip */

/*s: function _times */
static long
_times(long *t)
{
    char b[200], *p;
    int f;
    ulong r;

    memset(b, 0, sizeof(b));
    f = open("/dev/cputime", OREAD|OCEXEC);
    if(f < 0)
        return 0;
    if(read(f, b, sizeof(b)) <= 0){
        close(f);
        return 0;
    }
    p = b;
    if(t)
        t[0] = atol(p);
    p = skip(p);
    if(t)
        t[1] = atol(p);
    p = skip(p);
    r = atol(p);
    if(t){
        p = skip(p);
        t[2] = atol(p);
        p = skip(p);
        t[3] = atol(p);
    }
    return r;
}
/*e: function _times */

/*s: function efork */
static void
efork(Execargs *e)
{
    char buf[ERRMAX];

    _threaddebug(DBGEXEC, "_schedexec %s", e->prog);
    close(e->fd[0]);
    exec(e->prog, e->args);
    _threaddebug(DBGEXEC, "_schedexec failed: %r");
    rerrstr(buf, sizeof buf);
    if(buf[0]=='\0')
        strcpy(buf, "exec failed");
    write(e->fd[1], buf, strlen(buf));
    close(e->fd[1]);
    _exits(buf);
}
/*e: function efork */

/*s: function _schedexec */
int
_schedexec(Execargs *e)
{
    int pid;

    switch(pid = rfork(RFREND|RFNOTEG|RFFDG|RFMEM|RFPROC)){
    case 0:
        efork(e);
    default:
        return pid;
    }
}
/*e: function _schedexec */

/*s: function _schedfork */
int
_schedfork(Proc *p)
{
    int pid;

    switch(pid = rfork(RFPROC|RFMEM|RFNOWAIT|p->rforkflag)){
    case 0:
        *mainp = p;	/* write to stack, so local to proc */
        longjmp(_mainjmp, 1);
    default:
        return pid;
    }
}
/*e: function _schedfork */

/*s: function _schedexit */
void
_schedexit(Proc *p)
{
    char ex[ERRMAX];
    Proc **l;

    lock(&_threadpq.lock);
    for(l=&_threadpq.head; *l; l=&(*l)->next){
        if(*l == p){
            *l = p->next;
            if(*l == nil)
                _threadpq.tail = l;
            break;
        }
    }
    unlock(&_threadpq.lock);

    utfecpy(ex, ex+sizeof ex, p->exitstr);
    free(p);
    _exits(ex);
}
/*e: function _schedexit */

/*s: function _schedexecwait */
void
_schedexecwait(void)
{
    int pid;
    Channel *c;
    Proc *p;
    Thread *t;
    Waitmsg *w;

    p = _threadgetproc();
    t = p->thread;
    pid = t->ret;
    _threaddebug(DBGEXEC, "_schedexecwait %d", t->ret);

    rfork(RFCFDG);
    for(;;){
        w = wait();
        if(w == nil)
            break;
        if(w->pid == pid)
            break;
        free(w);
    }
    if(w != nil){
        if((c = _threadwaitchan) != nil)
            sendp(c, w);
        else
            free(w);
    }
    threadexits("procexec");
}
/*e: function _schedexecwait */

/*e: lib_core/libthread/main.c */
