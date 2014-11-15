/*s: lib_core/libthread/id.c */
#include <u.h>
#include <libc.h>
#include <thread.h>
#include "threadimpl.h"
#include <tos.h>

/*s: function threadid */
int
threadid(void)
{
    return _threadgetproc()->thread->id;
}
/*e: function threadid */

/*s: function threadpid */
int
threadpid(int id)
{
    int pid;
    Proc *p;
    Thread *t;

    if (id < 0)
        return -1;
    if (id == 0)
        return _threadgetproc()->pid;
    lock(&_threadpq.lock);
    for (p = _threadpq.head; p; p = p->next){
        lock(&p->lock);
        for (t = p->threads.head; t; t = t->nextt)
            if (t->id == id){
                pid = p->pid;
                unlock(&p->lock);
                unlock(&_threadpq.lock);
                return pid;
            }
        unlock(&p->lock);
    }
    unlock(&_threadpq.lock);
    return -1;
}
/*e: function threadpid */

/*s: function threadsetgrp */
int
threadsetgrp(int ng)
{
    int og;
    Thread *t;

    t = _threadgetproc()->thread;
    og = t->grp;
    t->grp = ng;
    return og;
}
/*e: function threadsetgrp */

/*s: function threadgetgrp */
int
threadgetgrp(void)
{
    return _threadgetproc()->thread->grp;
}
/*e: function threadgetgrp */

/*s: function threadsetname */
void
threadsetname(char *fmt, ...)
{
    int fd;
    char buf[128];
    va_list arg;
    Proc *p;
    Thread *t;

    p = _threadgetproc();
    t = p->thread;
    if (t->cmdname)
        free(t->cmdname);
    va_start(arg, fmt);
    t->cmdname = vsmprint(fmt, arg);
    va_end(arg);
    if(t->cmdname && p->nthreads == 1){
        snprint(buf, sizeof buf, "#p/%lud/args", _tos->pid); //getpid());
        if((fd = open(buf, OWRITE)) >= 0){
            write(fd, t->cmdname, strlen(t->cmdname)+1);
            close(fd);
        }
    }
}
/*e: function threadsetname */

/*s: function threadgetname */
char*
threadgetname(void)
{
    Proc *p;

    if((p = _threadgetproc()) && p->thread)
        return p->thread->cmdname;
    return nil;
}
/*e: function threadgetname */

/*s: function threaddata */
void**
threaddata(void)
{
    return &_threadgetproc()->thread->udata[0];
}
/*e: function threaddata */

/*s: function _workerdata */
void**
_workerdata(void)
{
    return &_threadgetproc()->wdata;
}
/*e: function _workerdata */

/*s: function procdata */
void**
procdata(void)
{
    return &_threadgetproc()->udata;
}
/*e: function procdata */

/*s: global privlock */
static Lock privlock;
/*e: global privlock */
/*s: global privmask */
static int privmask = 1;
/*e: global privmask */

/*s: function tprivalloc */
int
tprivalloc(void)
{
    int i;

    lock(&privlock);
    for(i=0; i<NPRIV; i++)
        if(!(privmask&(1<<i))){
            privmask |= 1<<i;
            unlock(&privlock);
            return i;
        }
    unlock(&privlock);
    return -1;
}
/*e: function tprivalloc */

/*s: function tprivfree */
void
tprivfree(int i)
{
    if(i < 0 || i >= NPRIV)
        abort();
    lock(&privlock);
    privmask &= ~(1<<i);
}
/*e: function tprivfree */

/*s: function tprivaddr */
void**
tprivaddr(int i)
{
    return &_threadgetproc()->thread->udata[i];
}
/*e: function tprivaddr */
/*e: lib_core/libthread/id.c */
