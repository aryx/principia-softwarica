/*s: sysfile.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

/*
 * The sys*() routines needn't poperror() as they return directly to syscall().
 */

/*s: function [[unlockfgrp]] */
static void
unlockfgrp(Fgrp *f)
{
    int ex;

    ex = f->exceed;
    f->exceed = 0;
    unlock(f);
    if(ex)
        pprint("warning: process exceeds %d file descriptors\n", ex);
}
/*e: function [[unlockfgrp]] */

/*s: function [[growfd]] */
int
growfd(Fgrp *f, int fd) /* fd is always >= 0 */
{
    Chan **newfd, **oldfd;

    if(fd < f->nfd)
        return 0; // should never happen no?
    if(fd >= f->nfd+DELTAFD)
        return -1;  /* out of range */ // when can this happen? from sysdup?
    /*
     * Unbounded allocation is unwise
     */
    if(f->nfd >= 5000){
    Exhausted:
        print("no free file descriptors\n");
        return -1;
    }
    // realloc
    newfd = malloc((f->nfd+DELTAFD)*sizeof(Chan*));
    if(newfd == nil)
        goto Exhausted;
    oldfd = f->fd;
    memmove(newfd, oldfd, f->nfd*sizeof(Chan*));
    f->fd = newfd;
    free(oldfd);
    f->nfd += DELTAFD;
    if(fd > f->maxfd){
        if(fd/100 > f->maxfd/100)
            f->exceed = (fd/100)*100;
        f->maxfd = fd;
    }
    return 1;
}
/*e: function [[growfd]] */

/*s: function [[findfreefd]] */
/*
 *  this assumes that the fgrp is locked
 */
int
findfreefd(Fgrp *f, int start)
{
    int fd;

    for(fd=start; fd<f->nfd; fd++)
        if(f->fd[fd] == nil)
            break;
    if(fd >= f->nfd && growfd(f, fd) < 0)
        return -1;
    return fd;
}
/*e: function [[findfreefd]] */

/*s: function [[newfd]] */
int
newfd(Chan *c)
{
    int fd;
    Fgrp *f;

    f = up->fgrp;
    lock(f);
    fd = findfreefd(f, 0);
    if(fd < 0){
        unlockfgrp(f);
        return -1;
    }
    if(fd > f->maxfd)
        f->maxfd = fd;
    f->fd[fd] = c;
    unlockfgrp(f);
    return fd;
}
/*e: function [[newfd]] */

/*s: function [[newfd2]] */
int
newfd2(int fd[2], Chan *c[2])
{
    Fgrp *f;

    f = up->fgrp;
    lock(f);
    fd[0] = findfreefd(f, 0);
    if(fd[0] < 0){
        unlockfgrp(f);
        return -1;
    }
    fd[1] = findfreefd(f, fd[0]+1);
    if(fd[1] < 0){
        unlockfgrp(f);
        return -1;
    }
    if(fd[1] > f->maxfd)
        f->maxfd = fd[1];
    f->fd[fd[0]] = c[0];
    f->fd[fd[1]] = c[1];
    unlockfgrp(f);

    return 0;
}
/*e: function [[newfd2]] */


/*s: syscall fd2path */
// int fd2path(int fd, char *buf, int nbuf);
long
sysfd2path(ulong* arg)
{
    Chan *c;

    validaddr(arg[1], arg[2], true);
    c = fdtochan(arg[0], -1, false, true);
    snprint((char*)arg[1], arg[2], "%s", 
              chanpath(c));
    cclose(c);
    return 0;
}
/*e: syscall fd2path */

/*s: syscall pipe */
// int pipe(int fd[2]);
long
syspipe(ulong* arg)
{
    int fd[2];
    Chan *c[2];
    Dev *d;
    static char *datastr[] = {"data", "data1"};

    validaddr(arg[0], 2*BY2WD, true);
    arch_validalign(arg[0], sizeof(int));
    d = devtab[devno('|', false)];
    c[0] = namec("#|", Atodir, 0, 0);
    c[1] = nil;
    fd[0] = -1;
    fd[1] = -1;

    if(waserror()){
        cclose(c[0]);
        if(c[1])
            cclose(c[1]);
        nexterror();
    }
    c[1] = cclone(c[0]);
    if(walk(&c[0], datastr+0, 1, 1, nil) < 0)
        error(Egreg);
    if(walk(&c[1], datastr+1, 1, 1, nil) < 0)
        error(Egreg);
    c[0] = d->open(c[0], ORDWR);
    c[1] = d->open(c[1], ORDWR);
    if(newfd2(fd, c) < 0)
        error(Enofd);
    poperror();

    ((long*)arg[0])[0] = fd[0];
    ((long*)arg[0])[1] = fd[1];
    return 0;
}
/*e: syscall pipe */

/*s: syscall dup */
// int dup(int oldfd, int newfd);
long
sysdup(ulong* arg)
{
    int fd;
    Chan *c, *oc;
    Fgrp *f = up->fgrp;

    /*
     * Close after dup'ing, so date > #d/1 works
     */
    c = fdtochan(arg[0], -1, false, true);
    fd = arg[1];
    if(fd != -1){
        lock(f);
        if(fd<0 || growfd(f, fd)<0) {
            unlockfgrp(f);
            cclose(c);
            error(Ebadfd);
        }
        if(fd > f->maxfd)
            f->maxfd = fd;

        oc = f->fd[fd];
        f->fd[fd] = c;
        unlockfgrp(f);
        if(oc)
            cclose(oc);
    }else{
        if(waserror()) {
            cclose(c);
            nexterror();
        }
        fd = newfd(c);
        if(fd < 0)
            error(Enofd);
        poperror();
    }

    return fd;
}
/*e: syscall dup */

/*s: syscall open */
// int open(char *file, int omode);
long
sysopen(ulong* arg)
{
    int fd;
    Chan *c;

    openmode(arg[1]);   /* error check only */
    validaddr(arg[0], 1, false);
    c = namec((char*)arg[0], Aopen, arg[1], 0);
    if(waserror()){
        cclose(c);
        nexterror();
    }
    fd = newfd(c);
    if(fd < 0)
        error(Enofd);
    poperror();
    return fd;
}
/*e: syscall open */

/*s: syscall close */
// int close(int fd);
long
sysclose(ulong* arg)
{
    fdtochan(arg[0], -1, false, false); // for its error checking side effect
    fdclose(arg[0], 0);
    return 0;
}
/*e: syscall close */

/*s: function [[unionread]] */
long
unionread(Chan *c, virt_vp va, long n)
{
    int i;
    long nr;
    Mhead *m;
    Mount *mount;

    qlock(&c->umqlock);
    m = c->umh;
    rlock(&m->lock);
    mount = m->mount;

    /* bring mount in sync with c->uri and c->umc */
    for(i = 0; mount != nil && i < c->uri; i++)
        mount = mount->next;

    nr = 0;
    while(mount != nil){
        /* Error causes component of union to be skipped */
        if(mount->to && !waserror()){
            if(c->umc == nil){
                c->umc = cclone(mount->to);
                c->umc = devtab[c->umc->type]->open(c->umc, OREAD);
            }
    
            nr = devtab[c->umc->type]->read(c->umc, va, n, c->umc->offset);
            c->umc->offset += nr;
            poperror();
        }
        if(nr > 0)
            break;

        /* Advance to next element */
        c->uri++;
        if(c->umc){
            cclose(c->umc);
            c->umc = nil;
        }
        mount = mount->next;
    }
    runlock(&m->lock);
    qunlock(&c->umqlock);
    return nr;
}
/*e: function [[unionread]] */

/*s: function [[unionrewind]] */
static void
unionrewind(Chan *c)
{
    qlock(&c->umqlock);
    c->uri = 0;
    if(c->umc){
        cclose(c->umc);
        c->umc = nil;
    }
    qunlock(&c->umqlock);
}
/*e: function [[unionrewind]] */

/*s: function [[dirfixed]] */
static int
dirfixed(byte *p, byte *e, DirEntry *d)
{
    int len;

    len = GBIT16(p)+BIT16SZ;
    if(p + len > e)
        return -1;

    p += BIT16SZ;   /* ignore size */
    d->type = devno(GBIT16(p), true);
    p += BIT16SZ;
    d->dev = GBIT32(p);
    p += BIT32SZ;
    d->qid.type = GBIT8(p);
    p += BIT8SZ;
    d->qid.vers = GBIT32(p);
    p += BIT32SZ;
    d->qid.path = GBIT64(p);
    p += BIT64SZ;
    d->mode = GBIT32(p);
    p += BIT32SZ;
    d->atime = GBIT32(p);
    p += BIT32SZ;
    d->mtime = GBIT32(p);
    p += BIT32SZ;
    d->length = GBIT64(p);

    return len;
}
/*e: function [[dirfixed]] */

/*s: function [[offsetof_name_direntry]] */
static char*
offsetof_name_direntry(byte *p, int *n)
{
    p += BIT16SZ+BIT16SZ+BIT32SZ +
          BIT8SZ+BIT32SZ+BIT64SZ +
          BIT32SZ+BIT32SZ+BIT32SZ + 
          BIT64SZ;
    *n = GBIT16(p);
    return (char*)p+BIT16SZ;
}
/*e: function [[offsetof_name_direntry]] */

/*s: function [[direntry_setname]] */
static long
direntry_setname(char *name, int len, byte *p, long n, long maxn)
{
    char *oname;
    int olen;
    long nn;

    if(n == BIT16SZ)
        return BIT16SZ;

    oname = offsetof_name_direntry(p, &olen);

    nn = n+len-olen;
    PBIT16(p, nn-BIT16SZ);
    if(nn > maxn)
        return BIT16SZ;

    if(len != olen)
        memmove(oname+len, oname+olen, p+n-(byte*)(oname+olen));
    PBIT16((byte*)(oname-2), len);
    memmove(oname, name, len);
    return nn;
}
/*e: function [[direntry_setname]] */

/*s: function [[mountrock]] */
/*
 * Mountfix might have caused the fixed results of the directory read
 * to overflow the buffer.  Catch the overflow in c->dirrock.
 */
static void
mountrock(Chan *c, byte *p, byte **pe)
{
    byte *e, *r;
    int len, n;

    e = *pe;

    /* find last directory entry */
    for(;;){
        len = BIT16SZ+GBIT16(p);
        if(p+len >= e)
            break;
        p += len;
    }

    /* save it away */
    qlock(&c->rockqlock);
    if(c->nrock+len > c->mrock){
        n = ROUND(c->nrock+len, 1024);
        r = smalloc(n);
        memmove(r, c->dirrock, c->nrock);
        free(c->dirrock);
        c->dirrock = r;
        c->mrock = n;
    }
    memmove(c->dirrock+c->nrock, p, len);
    c->nrock += len;
    qunlock(&c->rockqlock);

    /* drop it */
    *pe = p;
}
/*e: function [[mountrock]] */

/*s: function [[mountrockread]] */
/*
 * Satisfy a directory read with the results saved in c->dirrock.
 */
static bool
mountrockread(Chan *c, byte *op, long n, long *nn)
{
    long dirlen;
    byte *rp, *erp, *ep, *p;

    /* common case */
    if(c->nrock == 0)
        return false;

    /* copy out what we can */
    qlock(&c->rockqlock);
    rp = c->dirrock;
    erp = rp+c->nrock;
    p = op;
    ep = p+n;
    while(rp+BIT16SZ <= erp){
        dirlen = BIT16SZ+GBIT16(rp);
        if(p+dirlen > ep)
            break;
        memmove(p, rp, dirlen);
        p += dirlen;
        rp += dirlen;
    }

    if(p == op){
        qunlock(&c->rockqlock);
        return false;
    }

    /* shift the rest */
    if(rp != erp)
        memmove(c->dirrock, rp, erp-rp);
    c->nrock = erp - rp;

    *nn = p - op;
    qunlock(&c->rockqlock);
    return true;
}
/*e: function [[mountrockread]] */

/*s: function [[mountrewind]] */
static void
mountrewind(Chan *c)
{
    c->nrock = 0;
}
/*e: function [[mountrewind]] */

/*s: function [[mountfix]] */
/*
 * Rewrite the results of a directory read to reflect current 
 * name space bindings and mounts.  Specifically, replace
 * directory entries for bind and mount points with the results
 * of statting what is mounted there.  Except leave the old names.
 */
static long
mountfix(Chan *c, byte *op, long n, long maxn)
{
    char *name;
    int nbuf, nname;
    Chan *nc;
    Mhead *mh;
    Mount *m;
    byte *p;
    int dirlen, rest;
    long l;
    byte *buf, *e;
    DirEntry d;

    p = op;
    buf = nil;
    nbuf = 0;
    for(e=&p[n]; p+BIT16SZ<e; p+=dirlen){
        dirlen = dirfixed(p, e, &d);
        if(dirlen < 0)
            break;
        nc = nil;
        mh = nil;
        if(findmount(&nc, &mh, d.type, d.dev, d.qid)){
            /*
             * If it's a union directory and the original is
             * in the union, don't rewrite anything.
             */
            for(m=mh->mount; m; m=m->next)
                if(eqchantdqid(m->to, d.type, d.dev, d.qid, true))
                    goto Norewrite;

            name = offsetof_name_direntry(p, &nname);
            /*
             * Do the stat but fix the name.  If it fails, leave old entry.
             * BUG: If it fails because there isn't room for the entry,
             * what can we do?  Nothing, really.  Might as well skip it.
             */
            if(buf == nil){
                buf = smalloc(4096);
                nbuf = 4096;
            }
            if(waserror())
                goto Norewrite;
            l = devtab[nc->type]->stat(nc, buf, nbuf);
            l = direntry_setname(name, nname, buf, l, nbuf);
            if(l == BIT16SZ)
                error("direntry_setname");
            poperror();

            /*
             * Shift data in buffer to accomodate new entry,
             * possibly overflowing into rock.
             */
            rest = e - (p+dirlen);
            if(l > dirlen){
                while(p+l+rest > op+maxn){
                    mountrock(c, p, &e);
                    if(e == p){
                        dirlen = 0;
                        goto Norewrite;
                    }
                    rest = e - (p+dirlen);
                }
            }
            if(l != dirlen){
                memmove(p+l, p+dirlen, rest);
                dirlen = l;
                e = p+dirlen+rest;
            }

            /*
             * Rewrite directory entry.
             */
            memmove(p, buf, l);

            Norewrite:
            cclose(nc);
            putmhead(mh);
        }
    }
    if(buf)
        free(buf);

    if(p != e)
        error("oops in rockfix");

    return e-op;
}
/*e: function [[mountfix]] */

/*s: function [[read]] */
// long pread(int fd, void *buf, long nbytes, vlong offset);
static long
read(ulong *arg, vlong *offp)
{
    long n, nn, nnn;
    byte *p;
    Chan *c;
    vlong off;

    n = arg[2];
    validaddr(arg[1], n, true);
    p = (void*)arg[1];
    c = fdtochan(arg[0], OREAD, true, true);

    if(waserror()){
        cclose(c);
        nexterror();
    }

    if(offp == nil) /* use and maintain channel's offset */
        off = c->offset;
    else
        off = *offp;

    if(off < 0)
        error(Enegoff);

    if(off == 0){   /* rewind to the beginning of the directory */
        if(offp == nil){
            c->offset = 0;
            c->devoffset = 0;
        }
        /*s: [[read()]] rewind when off == 0 */
        /*
         * The offset is passed through on directories, normally.
         * Sysseek complains, but pread is used by servers like exportfs,
         * that shouldn't need to worry about this issue.
         *
         * Notice that c->devoffset is the offset that c's dev is seeing.
         * The number of bytes read on this fd (c->offset) may be different
         * due to rewritings in rockfix.
         */
            mountrewind(c);
            unionrewind(c);
        /*e: [[read()]] rewind when off == 0 */
    }

    /*s: [[read()]] if c is a QTDIR */
        if(c->qid.type & QTDIR){
            if(mountrockread(c, p, n, &nn)){
                /* do nothing: mountrockread filled buffer */
            }else if(c->umh){
                nn = unionread(c, p, n);
            }else{
                if(off != c->offset)
                    error(Edirseek);
                nn = devtab[c->type]->read(c, p, n, c->devoffset);
            }
            nnn = mountfix(c, p, nn, n);
        }
    /*e: [[read()]] if c is a QTDIR */
    else
        nnn = nn = devtab[c->type]->read(c, p, n, off);

    lock(c);
    c->offset += nnn;
    c->devoffset += nn;
    unlock(c);

    poperror();
    cclose(c);

    return nnn;
}
/*e: function [[read]] */

/*s: syscall pread */
// long pread(int fd, void *buf, long nbytes, vlong offset);
long
syspread(ulong* arg)
{
    vlong v;
    va_list list;

    /* use varargs to guarantee alignment of vlong */
    va_start(list, arg[2]);
    v = va_arg(list, vlong);
    va_end(list);

    if(v == ~0ULL)
        return read(arg, nil);

    return read(arg, &v);
}
/*e: syscall pread */

/*s: function [[write]] */
// long pwrite(int fd, void *buf, long nbytes, vlong offset);
static long
write(ulong *arg, vlong *offp)
{
    Chan *c;
    long m, n;
    vlong off;

    validaddr(arg[1], arg[2], false);
    n = 0;
    c = fdtochan(arg[0], OWRITE, true, true);
    if(waserror()) {
        if(offp == nil){
            lock(c);
            c->offset -= n;
            unlock(c);
        }
        cclose(c);
        nexterror();
    }

    if(c->qid.type & QTDIR)
        error(Eisdir);

    n = arg[2];

    if(offp == nil){    /* use and maintain channel's offset */
        lock(c);
        off = c->offset;
        c->offset += n;
        unlock(c);
    }else
        off = *offp;

    if(off < 0)
        error(Enegoff);

    m = devtab[c->type]->write(c, (void*)arg[1], n, off);

    if(offp == nil && m < n){
        lock(c);
        c->offset -= n - m;
        unlock(c);
    }

    poperror();
    cclose(c);

    return m;
}
/*e: function [[write]] */

/*s: syscall pwrite */
// long pwrite(int fd, void *buf, long nbytes, vlong offset);
long
syspwrite(ulong* arg)
{
    vlong v;
    va_list list;

    /* use varargs to guarantee alignment of vlong */
    va_start(list, arg[2]);
    v = va_arg(list, vlong);
    va_end(list);

    if(v == ~0ULL)
        return write(arg, nil);

    return write(arg, &v);
}
/*e: syscall pwrite */

/*s: function [[sseek]] */
union v_or_u2 {
  vlong v;
  ulong u[2];
};

// vlong seek(int fd, vlong n, int type);
static void
sseek(ulong *arg)
{
    Chan *c;
    byte buf[sizeof(DirEntry)+100];
    DirEntry dir;
    int n;
    vlong off;
    union v_or_u2 o;

    c = fdtochan(arg[1], -1, true, true);
    if(waserror()){
        cclose(c);
        nexterror();
    }
    /*s: [[sseek()]] and pipes */
    if(devtab[c->type]->dc == '|')
        error(Eisstream);
    /*e: [[sseek()]] and pipes */

    off = 0;
    o.u[0] = arg[2];
    o.u[1] = arg[3];
    switch(arg[4]){
    case 0: // from the start
        off = o.v;
        /*s: [[sseek()]] ensures off is 0 for directories */
        if((c->qid.type & QTDIR) && off != 0)
            error(Eisdir);
        /*e: [[sseek()]] ensures off is 0 for directories */
        if(off < 0)
            error(Enegoff);
        c->offset = off; // just write, no need for lock
        break;

    case 1: // from the current location
        /*s: [[sseek()]] disallows seek type 1 or 2 for directories */
        if(c->qid.type & QTDIR)
            error(Eisdir);
        /*e: [[sseek()]] disallows seek type 1 or 2 for directories */
        lock(c);    /* lock for read/write update */
        off = o.v + c->offset;
        if(off < 0){
            unlock(c);
            error(Enegoff);
        }
        c->offset = off;
        unlock(c);
        break;

    case 2: // from the end of the file
        /*s: [[sseek()]] disallows seek type 1 or 2 for directories */
        if(c->qid.type & QTDIR)
            error(Eisdir);
        /*e: [[sseek()]] disallows seek type 1 or 2 for directories */
        n = devtab[c->type]->stat(c, buf, sizeof buf);
        if(convM2D(buf, n, &dir, nil) == 0)
            error("internal error: stat error in seek");
        off = dir.length + o.v;
        if(off < 0)
            error(Enegoff);
        c->offset = off;
        break;

    default:
        error(Ebadarg);
    }
    *(vlong*)arg[0] = off;
    c->uri = 0;
    c->dri = 0;
    cclose(c);
    poperror();
}
/*e: function [[sseek]] */

/*s: syscall seek */
// vlong seek(int fd, vlong n, int type);
long
sysseek(ulong* arg)
{
    validaddr(arg[0], BY2V, true);
    arch_validalign(arg[0], sizeof(vlong));
    sseek(arg);
    return 0;
}
/*e: syscall seek */

/*s: function [[pathlast]] */
static char*
pathlast(Path *p)
{
    char *s;

    if(p == nil)
        return nil;
    if(p->len == 0)
        return nil;
    s = strrchr(p->s, '/');
    if(s)
        return s+1;
    return p->s;
}
/*e: function [[pathlast]] */

/*s: syscall fstat */
// int fstat(int fd, byte *edir, int nedir);
long
sysfstat(ulong* arg)
{
    Chan *c;
    uint l;

    l = arg[2];
    validaddr(arg[1], l, true);
    c = fdtochan(arg[0], -1, false, true);
    if(waserror()) {
        cclose(c);
        nexterror();
    }
    l = devtab[c->type]->stat(c, (byte*)arg[1], l);
    poperror();
    cclose(c);
    return l;
}
/*e: syscall fstat */

/*s: syscall stat */
// int stat(char *name, byte *edir, int nedir);
long
sysstat(ulong* arg)
{
    char *name;
    Chan *c;
    uint l;

    l = arg[2];
    validaddr(arg[1], l, true);
    validaddr(arg[0], 1, false);
    c = namec((char*)arg[0], Aaccess, 0, 0);
    if(waserror()){
        cclose(c);
        nexterror();
    }
    l = devtab[c->type]->stat(c, (byte*)arg[1], l);

    name = pathlast(c->path);
    if(name)
        l = direntry_setname(name, strlen(name), (byte*)arg[1], l, arg[2]);

    poperror();
    cclose(c);
    return l;
}
/*e: syscall stat */

/*s: syscall chdir */
// int chdir(char *dirname);
long
syschdir(ulong* arg)
{
    Chan *c;

    validaddr(arg[0], 1, false);
    c = namec((char*)arg[0], Atodir, 0, 0);
    cclose(up->dot);
    up->dot = c;
    return 0;
}
/*e: syscall chdir */

/*s: function [[bindmount]] */

long
bindmount(bool ismount, int fd, int afd, char* arg0, char* arg1, ulong flag, char* spec)
{
    /*s: [[bindmount()]] locals */
    Chan *c0;
    Chan *c1;
    /*x: [[bindmount()]] locals */
    int ret;
    /*x: [[bindmount()]] locals */
    Chan *ac, *bc;

    struct Bogus bogus;
    /*e: [[bindmount()]] locals */

    if((flag&~MMASK) || (flag&MORDERMASK)==(MBEFORE|MAFTER))
        error(Ebadarg);

    /*s: [[bindmount()]] if ismount */
    if(ismount){
        validaddr((ulong)spec, 1, false);
        spec = validnamedup(spec, true);
        if(waserror()){
            free(spec);
            nexterror();
        }
        /*s: [[bindmount()]] error if noattach */
        if(up->pgrp->noattach)
            error(Enoattach);
        /*e: [[bindmount()]] error if noattach */

        ac = nil;
        bc = fdtochan(fd, ORDWR, false, true);
        if(waserror()) {
            if(ac)
                cclose(ac);
            cclose(bc);
            nexterror();
        }

        if(afd >= 0)
            ac = fdtochan(afd, ORDWR, false, true);

        bogus.flags = flag & MCACHE;
        bogus.chan = bc;
        bogus.authchan = ac;
        bogus.spec = spec;
        ret = devno('M', false);
        c0 = devtab[ret]->attach((char*)&bogus);
        poperror(); /* ac bc */
        if(ac)
            cclose(ac);
        cclose(bc);
    }
    /*e: [[bindmount()]] if ismount */
    else{
        spec = nil;
        validaddr((ulong)arg0, 1, false);
        c0 = namec(arg0, Abind, 0, 0);
    }

    if(waserror()){
        cclose(c0);
        nexterror();
    }

    validaddr((ulong)arg1, 1, false);
    c1 = namec(arg1, Amount, 0, 0);

    if(waserror()){
        cclose(c1);
        nexterror();
    }

    ret = cmount(c0, c1, flag, spec);

    poperror();
    cclose(c1);
    poperror();
    cclose(c0);
    /*s: [[bindmount()]] if ismount free */
    if(ismount){
        fdclose(fd, 0);
        poperror();
        free(spec);
    }
    /*e: [[bindmount()]] if ismount free */
    return ret;
}
/*e: function [[bindmount]] */

/*s: syscall bind */
// int bind(char *to, char *from, int flag);
long
sysbind(ulong* arg)
{
    return bindmount(false, -1, -1, (char*)arg[0], (char*)arg[1], arg[2], nil);
}
/*e: syscall bind */

/*s: syscall mount */
// int mount(int fd, int afd, char *old, int flag, char *aname);
long
sysmount(ulong* arg)
{
    return bindmount(true, arg[0], arg[1], nil, (char*)arg[2], arg[3], (char*)arg[4]);
}
/*e: syscall mount */

/*s: syscall unmount */
// int unmount(char *name, char *old);
long
sysunmount(ulong* arg)
{
    Chan *cmount, *cmounted;

    cmounted = nil;

    validaddr(arg[1], 1, false);
    cmount = namec((char *)arg[1], Amount, 0, 0);
    if(waserror()) {
        cclose(cmount);
        if(cmounted)
            cclose(cmounted);
        nexterror();
    }

    if(arg[0]) {
        /*
         * This has to be namec(..., Aopen, ...) because
         * if arg[0] is something like /srv/cs or /fd/0,
         * opening it is the only way to get at the real
         * Chan underneath.
         */
        validaddr(arg[0], 1, false);
        cmounted = namec((char*)arg[0], Aopen, OREAD, 0);
    }
    cunmount(cmount, cmounted);
    poperror();
    cclose(cmount);
    if(cmounted)
        cclose(cmounted);
    return 0;
}
/*e: syscall unmount */

/*s: syscall create */
// int create(char *file, int omode, ulong perm);
long
syscreate(ulong* arg)
{
    int fd;
    Chan *c;

    openmode(arg[1]&~OEXCL);    /* error check only; OEXCL okay here */
    validaddr(arg[0], 1, false);
    c = namec((char*)arg[0], Acreate, arg[1], arg[2]);
    if(waserror()){
        cclose(c);
        nexterror();
    }
    fd = newfd(c);
    if(fd < 0)
        error(Enofd);
    poperror();
    return fd;
}
/*e: syscall create */

/*s: syscall remove */
// int remove(char *file);
long
sysremove(ulong* arg)
{
    Chan *c;

    validaddr(arg[0], 1, false);
    c = namec((char*)arg[0], Aremove, 0, 0);
    /*
     * Removing mount points is disallowed to avoid surprises
     * (which should be removed: the mount point or the mounted Chan?).
     */
    if(c->ismtpt){
        cclose(c);
        error(Eismtpt);
    }
    if(waserror()){
        c->type = 0;    /* see below */
        cclose(c);
        nexterror();
    }
    devtab[c->type]->remove(c);
    /*
     * Remove clunks the fid, but we need to recover the Chan
     * so fake it up.  rootclose() is known to be a nop.
     */
    c->type = 0;
    poperror();
    cclose(c);
    return 0;
}
/*e: syscall remove */

/*s: function [[wstat]] */
static long
wstat(Chan *c, byte *d, int nd)
{
    long l;
    int namelen;

    if(waserror()){
        cclose(c);
        nexterror();
    }
    if(c->ismtpt){
        /*
         * Renaming mount points is disallowed to avoid surprises
         * (which should be renamed? the mount point or the mounted Chan?).
         */
        offsetof_name_direntry(d, &namelen);
        if(namelen)
            nameerror(chanpath(c), Eismtpt);
    }
    l = devtab[c->type]->wstat(c, d, nd);
    poperror();
    cclose(c);
    return l;
}
/*e: function [[wstat]] */

/*s: syscall wstat */
// int wstat(char *name, byte *edir, int nedir);
long
syswstat(ulong* arg)
{
    Chan *c;
    uint l;

    l = arg[2];
    validaddr(arg[1], l, false);
    validstat((byte*)arg[1], l);
    validaddr(arg[0], 1, false);
    c = namec((char*)arg[0], Aaccess, 0, 0);
    return wstat(c, (byte*)arg[1], l);
}
/*e: syscall wstat */

/*s: syscall fwstat */
// int fwstat(int fd, byte *edir, int nedir);
long
sysfwstat(ulong* arg)
{
    Chan *c;
    uint l;

    l = arg[2];
    validaddr(arg[1], l, false);
    validstat((byte*)arg[1], l);
    c = fdtochan(arg[0], -1, true, true);
    return wstat(c, (byte*)arg[1], l);
}
/*e: syscall fwstat */

/*e: sysfile.c */
