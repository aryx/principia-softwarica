#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"
#include    "../port/error.h"

Chan*
fdtochan(int fd, int mode, int chkmnt, int iref)
{
    Chan *c;
    Fgrp *f;

    c = 0;
    f = up->fgrp;

    lock(f);
    if(fd<0 || f->nfd<=fd || (c = f->fd[fd])==0) {
        unlock(f);
        error(Ebadfd);
    }
    if(iref)
        incref(c);
    unlock(f);

    if(chkmnt && (c->flag&CMSG)) {
        if(iref)
            cclose(c);
        error(Ebadusefd);
    }

    if(mode<0 || c->mode==ORDWR)
        return c;

    if((mode&OTRUNC) && c->mode==OREAD) {
        if(iref)
            cclose(c);
        error(Ebadusefd);
    }

    if((mode&~OTRUNC) != c->mode) {
        if(iref)
            cclose(c);
        error(Ebadusefd);
    }

    return c;
}

int
openmode(ulong o)
{
    o &= ~(OTRUNC|OCEXEC|ORCLOSE);
    if(o > OEXEC)
        error(Ebadarg);
    if(o == OEXEC)
        return OREAD;
    return o;
}


void
fdclose(int fd, int flag)
{
    int i;
    Chan *c;
    Fgrp *f = up->fgrp;

    lock(f);
    c = f->fd[fd];
    if(c == 0){
        /* can happen for users with shared fd tables */
        unlock(f);
        return;
    }
    if(flag){
        if(c==0 || !(c->flag&flag)){
            unlock(f);
            return;
        }
    }
    f->fd[fd] = 0;
    if(fd == f->maxfd)
        for(i=fd; --i>=0 && f->fd[i]==0; )
            f->maxfd = i;

    unlock(f);
    cclose(c);
}


void
validstat(uchar *s, int n)
{
    int m;
    char buf[64];

    if(statcheck(s, n) < 0)
        error(Ebadstat);
    /* verify that name entry is acceptable */
    s += STATFIXLEN - 4*BIT16SZ;    /* location of first string */
    /*
     * s now points at count for first string.
     * if it's too long, let the server decide; this is
     * only for his protection anyway. otherwise
     * we'd have to allocate and waserror.
     */
    m = GBIT16(s);
    s += BIT16SZ;
    if(m+1 > sizeof buf)
        return;
    memmove(buf, s, m);
    buf[m] = '\0';
    /* name could be '/' */
    if(strcmp(buf, "/") != 0)
        validname(buf, 0);
}

