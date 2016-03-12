/*s: windows/rio/fsys.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>
#include <cursor.h>
#include <mouse.h>
#include <keyboard.h>
#include <frame.h>
#include <fcall.h>

#include "dat.h"
#include "fns.h"

/*s: global Eperm */
char Eperm[] = "permission denied";
/*e: global Eperm */
/*s: global Eexist */
char Eexist[] = "file does not exist";
/*e: global Eexist */
/*s: global Enotdir */
char Enotdir[] = "not a directory";
/*e: global Enotdir */
/*s: global Ebadfcall */
char	Ebadfcall[] = "bad fcall type";
/*e: global Ebadfcall */
/*s: global Eoffset */
char	Eoffset[] = "illegal offset";
/*e: global Eoffset */


/*s: constant DEBUG */
#define DEBUG false
/*e: constant DEBUG */

/*s: global dirtab */
Dirtab dirtab[]=
{
    { ".",		QTDIR,	Qdir,		0500|DMDIR },
    /*s: dirtab array elements */
    { "mouse",		QTFILE,	Qmouse,		0600 },
    /*x: dirtab array elements */
    { "cons",		QTFILE,	Qcons,		0600 },
    /*x: dirtab array elements */
    { "consctl",	QTFILE,	Qconsctl,	0200 },
    /*x: dirtab array elements */
    { "cursor",		QTFILE,	Qcursor,	0600 },
    /*x: dirtab array elements */
    { "winname",	QTFILE,	Qwinname,	0400 },
    /*x: dirtab array elements */
    { "window",		QTFILE,	Qwindow,	0400 },
    /*x: dirtab array elements */
    { "text",		QTFILE,	Qtext,		0400 },
    /*x: dirtab array elements */
    { "winid",		QTFILE,	Qwinid,		0400 },
    /*x: dirtab array elements */
    { "label",		QTFILE,	Qlabel,		0600 },
    /*x: dirtab array elements */
    { "wdir",		QTFILE,	Qwdir,		0600 },
    /*x: dirtab array elements */
    { "screen",		QTFILE,	Qscreen,	0400 },
    /*x: dirtab array elements */
    { "wsys",		QTDIR,	Qwsys,		0500|DMDIR },
    /*x: dirtab array elements */
    { "wctl",		QTFILE,	Qwctl,		0600 },
    /*x: dirtab array elements */
    { "snarf",		QTFILE,	Qsnarf,		0600 },
    /*x: dirtab array elements */
    { "kbdin",		QTFILE,	Qkbdin,		0200 },
    /*e: dirtab array elements */
    { nil, }
};
/*e: global dirtab */

static uint		getclock(void);
static void		filsysproc(void*);
static Fid*		newfid(Filsys*, int);
static int		dostat(Filsys*, int, Dirtab*, uchar*, int, uint);

/*s: global clockfd */
fdt	clockfd;
/*e: global clockfd */
/*s: global firstmessage */
bool	firstmessage = true;
/*e: global firstmessage */

/*s: global srvpipe (windows/rio/fsys.c) */
char	srvpipe[64];
/*e: global srvpipe (windows/rio/fsys.c) */
/*s: global srvwctl (windows/rio/fsys.c) */
char	srvwctl[64];
/*e: global srvwctl (windows/rio/fsys.c) */

static	Xfid*	filsysflush(Filsys*, Xfid*, Fid*);
static	Xfid*	filsysversion(Filsys*, Xfid*, Fid*);
static	Xfid*	filsysauth(Filsys*, Xfid*, Fid*);
static	Xfid*	filsysnop(Filsys*, Xfid*, Fid*);
static	Xfid*	filsysattach(Filsys*, Xfid*, Fid*);
static	Xfid*	filsyswalk(Filsys*, Xfid*, Fid*);
static	Xfid*	filsysopen(Filsys*, Xfid*, Fid*);
static	Xfid*	filsyscreate(Filsys*, Xfid*, Fid*);
static	Xfid*	filsysread(Filsys*, Xfid*, Fid*);
static	Xfid*	filsyswrite(Filsys*, Xfid*, Fid*);
static	Xfid*	filsysclunk(Filsys*, Xfid*, Fid*);
static	Xfid*	filsysremove(Filsys*, Xfid*, Fid*);
static	Xfid*	filsysstat(Filsys*, Xfid*, Fid*);
static	Xfid*	filsyswstat(Filsys*, Xfid*, Fid*);

/*s: global fcall */
Xfid* 	(*fcall[Tmax])(Filsys*, Xfid*, Fid*) =
{
    [Tattach]  = filsysattach,

    [Twalk]    = filsyswalk,

    [Topen]    = filsysopen,
    [Tclunk]   = filsysclunk,
    [Tread]    = filsysread,
    [Twrite]   = filsyswrite,
    [Tstat]    = filsysstat,

    /*s: [[fcall]] other methods */
    [Tcreate]  = filsyscreate,
    [Tremove]  = filsysremove,
    [Twstat]   = filsyswstat,
    /*x: [[fcall]] other methods */
    [Tversion] = filsysversion,
    [Tauth]    = filsysauth,
    /*x: [[fcall]] other methods */
    [Tflush]   = filsysflush,
    /*e: [[fcall]] other methods */
};
/*e: global fcall */

/*s: function post */
void
post(char *name, char *envname, fdt srvfd)
{
    fdt fd;
    char buf[32];

    fd = create(name, OWRITE|ORCLOSE|OCEXEC, 0600);
    if(fd < 0)
        error(name);
    sprint(buf, "%d", srvfd);
    if(write(fd, buf, strlen(buf)) != strlen(buf))
        error("srv write");

    putenv(envname, name);
}
/*e: function post */

/*s: function cexecpipe */
/*
 * Build pipe with OCEXEC set on second fd.
 * Can't put it on both because we want to post one in /srv.
 */
errorneg1
cexecpipe(fdt *p0, fdt *p1)
{
    /* pipe the hard way to get close on exec */
    if(bind("#|", "/mnt/temp", MREPL) < 0)
        return ERROR_NEG1;
    *p0 = open("/mnt/temp/data", ORDWR);
    *p1 = open("/mnt/temp/data1", ORDWR|OCEXEC);
    unmount(nil, "/mnt/temp");
    if(*p0<0 || *p1<0)
        return ERROR_NEG1;
    return OK_0;
}
/*e: function cexecpipe */

/*s: function filsysinit */
Filsys*
filsysinit(Channel *cxfidalloc)
{
    int pid;
    Filsys *fs;
    /*s: [[filsysinit()]] other locals */
    fdt fd;
    char buf[128];
    int n;
    /*x: [[filsysinit()]] other locals */
    fdt p0;
    // chan<??> (listener = ??, sender = ??)
    Channel *c;
    /*e: [[filsysinit()]] other locals */

    /*s: [[filsysinit()]] install dumper */
    fmtinstall('F', fcallfmt);
    /*e: [[filsysinit()]] install dumper */

    fs = emalloc(sizeof(Filsys));

    if(cexecpipe(&fs->cfd, &fs->sfd) < 0)
        goto Rescue;

    /*s: [[filsysinit()]] set clockfd */
    clockfd = open("/dev/time", OREAD|OCEXEC);
    /*e: [[filsysinit()]] set clockfd */
    /*s: [[filsysinit()]] set fs user */
    fd = open("/dev/user", OREAD);
    strcpy(buf, "Jean-Paul_Belmondo"); // lol
    if(fd >= 0){
        n = read(fd, buf, sizeof buf-1);
        if(n > 0)
            buf[n] = 0;
        close(fd);
    }
    fs->user = estrdup(buf);
    /*e: [[filsysinit()]] set fs user */
    pid = getpid();

    fs->cxfidalloc = cxfidalloc;

    /*s: [[filsysinit()]] wctl pipe, process, and thread creation */
    /*
     * Create and post wctl pipe
     */
    /*s: [[filsysinit()]] create wctl pipe */
    if(cexecpipe(&p0, &wctlfd) < 0)
        goto Rescue;
    sprint(srvwctl, "/srv/riowctl.%s.%d", fs->user, pid);
    post(srvwctl, "wctl", p0);
    close(p0);
    /*e: [[filsysinit()]] create wctl pipe */

    /*
     * Start server processes
     */
    /*s: [[filsysinit()]] create wctl process and thread */
    c = chancreate(sizeof(char*), 0);
    if(c == nil)
        error("wctl channel");

    proccreate(wctlproc, c, 4096);
    threadcreate(wctlthread, c, 4096);
    /*e: [[filsysinit()]] create wctl process and thread */
    /*e: [[filsysinit()]] wctl pipe, process, and thread creation */

    proccreate(filsysproc, fs, 10000);

    /*s: [[filsysinit()]] srv pipe */
    /*
     * Post srv pipe
     */
    sprint(srvpipe, "/srv/rio.%s.%d", fs->user, pid);
    post(srvpipe, "wsys", fs->cfd);
    /*e: [[filsysinit()]] srv pipe */

    return fs;

Rescue:
    free(fs);
    return nil;
}
/*e: function filsysinit */

/*s: function filsysproc */
static
void
filsysproc(void *arg)
{
    Filsys *fs = arg;
    int n;
    byte *buf;
    Xfid *x = nil;
    Fid *f;
    /*s: [[filsysproc()]] other locals */
    Fcall fc;
    /*e: [[filsysproc()]] other locals */

    threadsetname("FILSYSPROC");

    for(;;){
        buf = emalloc(messagesize+UTFmax);	/* UTFmax for appending partial rune in xfidwrite */

        n = read9pmsg(fs->sfd, buf, messagesize);
        /*s: [[filsysproc()]] sanity check n */
        if(n <= 0){
            yield();	/* if threadexitsall'ing, will not return */
            fprint(STDERR, "rio: %d: read9pmsg: %d %r\n", getpid(), n);
            errorshouldabort = false;
            error("eof or i/o error on server channel");
        }
        /*e: [[filsysproc()]] sanity check n */
        if(x == nil){
            send(fs->cxfidalloc, nil);
            recv(fs->cxfidalloc, &x);
            x->fs = fs;
        }
        x->buf = buf;

        if(convM2S(buf, n, x) != n)
            error("convert error in convM2S");
        /*s: [[filsysproc()]] dump Fcall if debug */
        if(DEBUG)
            fprint(STDERR, "rio:<-%F\n", &x->Fcall);
        /*e: [[filsysproc()]] dump Fcall if debug */

        /*s: [[filsysproc()]] sanity check x type */
        if(fcall[x->type] == nil)
            x = filsysrespond(fs, x, &fc, Ebadfcall);
        /*e: [[filsysproc()]] sanity check x type */
        else{
            /*s: [[filsysproc()]] if x type is Tversion or Tauth */
            if(x->type==Tversion || x->type==Tauth)
                f = nil;
            /*e: [[filsysproc()]] if x type is Tversion or Tauth */
            else
                f = newfid(fs, x->fid);

            x->f = f;

            // Dispatch
            x  = (*fcall[x->type])(fs, x, f);
        }
        firstmessage = false;
    }
}
/*e: function filsysproc */

/*s: function filsysmount */
/*
 * Called only from a different FD group
 */
errorneg1
filsysmount(Filsys *fs, int id)
{
    char buf[32];
    errorneg1 err;

    close(fs->sfd);	/* close server end so mount won't hang if exiting */
    sprint(buf, "%d", id);
    err = mount(fs->cfd, -1, "/mnt/wsys", MREPL, buf);
    /*s: [[filsysmount()]] sanity check err mount */
    if(err < 0){
        fprint(STDERR, "mount failed: %r\n");
        return ERROR_NEG1;
    }
    /*e: [[filsysmount()]] sanity check err mount */
    err = bind("/mnt/wsys", "/dev", MBEFORE);
    /*s: [[filsysmount()]] sanity check err bind */
    if(err < 0){
        fprint(STDERR, "bind failed: %r\n");
        return ERROR_NEG1;
    }
    /*e: [[filsysmount()]] sanity check err bind */
    return OK_0;
}
/*e: function filsysmount */

/*s: function filsysrespond */
Xfid*
filsysrespond(Filsys *fs, Xfid *x, Fcall *fc, char *err)
{
    int n;

    if(err){
        fc->type = Rerror;
        fc->ename = err;
    }else
        fc->type = x->type+1; // Reply type just after Transmit type

    fc->fid = x->fid;
    fc->tag = x->tag;

    if(x->buf == nil)
        x->buf = malloc(messagesize);
    n = convS2M(fc, x->buf, messagesize);
    /*s: [[filsysrespond()]] sanity check n */
    if(n <= 0)
        error("convert error in convS2M");
    /*e: [[filsysrespond()]] sanity check n */

    if(write(fs->sfd, x->buf, n) != n)
        error("write error in respond");
    /*s: [[filsysrespond()]] dump Fcall t if debug */
    if(DEBUG)
        fprint(STDERR, "rio:->%F\n", fc);
    /*e: [[filsysrespond()]] dump Fcall t if debug */
    free(x->buf);
    x->buf = nil;
    return x;
}
/*e: function filsysrespond */

/*s: function filsyscancel */
void
filsyscancel(Xfid *x)
{
    if(x->buf){
        free(x->buf);
        x->buf = nil;
    }
}
/*e: function filsyscancel */

/*s: function filsysversion */
static
Xfid*
filsysversion(Filsys *fs, Xfid *x, Fid*)
{
    Fcall fc;

    if(!firstmessage)
        return filsysrespond(x->fs, x, &fc, "version request not first message");
    if(x->msize < 256)
        return filsysrespond(x->fs, x, &fc, "version: message size too small");

    messagesize = x->msize;

    fc.msize = messagesize;
    if(strncmp(x->version, "9P2000", 6) != 0)
        return filsysrespond(x->fs, x, &fc, "unrecognized 9P version");
    // else
    fc.version = "9P2000";
    return filsysrespond(fs, x, &fc, nil);
}
/*e: function filsysversion */

/*s: function filsysauth */
static
Xfid*
filsysauth(Filsys *fs, Xfid *x, Fid*)
{
    Fcall fc;

    return filsysrespond(fs, x, &fc, "rio: authentication not required");
}
/*e: function filsysauth */

/*s: function filsysflush */
static
Xfid*
filsysflush(Filsys*, Xfid *x, Fid*)
{
    sendp(x->c, xfidflush);
    return nil;
}
/*e: function filsysflush */

/*s: function filsysattach */
static
Xfid*
filsysattach(Filsys *, Xfid *x, Fid *f)
{
    /*s: [[filsysattach()]] locals */
    Fcall fc;
    /*e: [[filsysattach()]] locals */

    /*s: [[filsysattach()]] sanity check same user */
    if(strcmp(x->uname, x->fs->user) != 0)
        return filsysrespond(x->fs, x, &fc, Eperm);
    /*e: [[filsysattach()]] sanity check same user */

    f->busy = true;
    f->open = false;

    f->qid.path = Qdir;
    f->qid.type = QTDIR;
    f->qid.vers = 0;
    f->dir = dirtab; // entry for "."

    f->nrpart = 0;

    sendp(x->c, xfidattach);
    return nil;
}
/*e: function filsysattach */

/*s: function numeric */
static
int
numeric(char *s)
{
    for(; *s!='\0'; s++)
        if(*s<'0' || '9'<*s)
            return 0;
    return 1;
}
/*e: function numeric */

/*s: function filsyswalk */
static
Xfid*
filsyswalk(Filsys *fs, Xfid *x, Fid *f)
{
    Fcall fc;
    Fid *nf;
    int i, id;
    uchar type;
    ulong path;
    Dirtab *d, *dir;
    Window *w;
    char *err;
    Qid qid;

    if(f->open)
        return filsysrespond(fs, x, &fc, "walk of open file");
    nf = nil;
    if(x->fid  != x->newfid){
        /* BUG: check exists */
        nf = newfid(fs, x->newfid);
        if(nf->busy)
            return filsysrespond(fs, x, &fc, "clone to busy fid");
        nf->busy = true;
        nf->open = false;
        nf->dir = f->dir;
        nf->qid = f->qid;
        nf->w = f->w;
        incref(f->w);
        nf->nrpart = 0;	/* not open, so must be zero */
        f = nf;	/* walk f */
    }

    fc.nwqid = 0;
    err = nil;

    /* update f->qid, f->dir only if walk completes */
    qid = f->qid;
    dir = f->dir;

    if(x->nwname > 0){
        for(i=0; i<x->nwname; i++){
            if((qid.type & QTDIR) == 0){
                err = Enotdir;
                break;
            }
            if(strcmp(x->wname[i], "..") == 0){
                type = QTDIR;
                path = Qdir;
                dir = dirtab;
                if(FILE(qid) == Qwsysdir)
                    path = Qwsys;
                id = 0;
    Accept:
                if(i == MAXWELEM){
                    err = "name too long";
                    break;
                }
                qid.type = type;
                qid.vers = 0;
                qid.path = QID(id, path);
                fc.wqid[fc.nwqid++] = qid;
                continue;
            }

            if(qid.path == Qwsys){
                /* is it a numeric name? */
                if(!numeric(x->wname[i]))
                    break;
                /* yes: it's a directory */
                id = atoi(x->wname[i]);
                qlock(&all);
                w = wlookid(id);
                if(w == nil){
                    qunlock(&all);
                    break;
                }
                path = Qwsysdir;
                type = QTDIR;
                qunlock(&all);
                incref(w);
                sendp(winclosechan, f->w);
                f->w = w;
                dir = dirtab;
                goto Accept;
            }
        
            if(snarffd>=0 && strcmp(x->wname[i], "snarf")==0)
                break;	/* don't serve /dev/snarf if it's provided in the environment */
            id = WIN(f->qid);
            d = dirtab;
            d++;	/* skip '.' */
            for(; d->name; d++)
                if(strcmp(x->wname[i], d->name) == 0){
                    path = d->qid;
                    type = d->type;
                    dir = d;
                    goto Accept;
                }

            break;	/* file not found */
        }

        if(i==0 && err==nil)
            err = Eexist;
    }

    if(err!=nil || fc.nwqid<x->nwname){
        if(nf){
            if(nf->w)
                sendp(winclosechan, nf->w);
            nf->open = false;
            nf->busy = false;
        }
    }else if(fc.nwqid == x->nwname){
        f->dir = dir;
        f->qid = qid;
    }

    return filsysrespond(fs, x, &fc, err);
}
/*e: function filsyswalk */

/*s: function filsysopen */
static
Xfid*
filsysopen(Filsys *fs, Xfid *x, Fid *f)
{
    Fcall fc;
    int m;

    /*s: [[filsysopen()]] sanity check mode */
    /* can't truncate anything, so just disregard */
    x->mode &= ~(OTRUNC|OCEXEC);
    /* can't execute or remove anything */
    if(x->mode==OEXEC || (x->mode&ORCLOSE))
        goto Deny;

    switch(x->mode){
    case OREAD:
        m = 0400;
        break;
    case OWRITE:
        m = 0200;
        break;
    case ORDWR:
        m = 0600;
        break;
    default:
        goto Deny;
    }
    if(((f->dir->perm & ~(DMDIR|DMAPPEND)) & m) != m)
        goto Deny;
    /*e: [[filsysopen()]] sanity check mode */
        
    sendp(x->c, xfidopen);
    return nil;

    Deny:
    return filsysrespond(fs, x, &fc, Eperm);
}
/*e: function filsysopen */

/*s: function filsyscreate */
static
Xfid*
filsyscreate(Filsys *fs, Xfid *x, Fid*)
{
    Fcall fc;

    return filsysrespond(fs, x, &fc, Eperm);
}
/*e: function filsyscreate */

/*s: function idcmp */
static
int
idcmp(void *a, void *b)
{
    return *(int*)a - *(int*)b;
}
/*e: function idcmp */

/*s: function filsysread */
static
Xfid*
filsysread(Filsys *fs, Xfid *x, Fid *f)
{
    Fcall fc;
    int o, e;
    uint clock;
    byte *b;
    /*s: [[filsysread()]] other locals */
    int i, n, len, j, k, *ids;
    Dirtab *d, dt;
    char buf[16];
    /*e: [[filsysread()]] other locals */

    if(!(f->qid.type & QTDIR)){
        sendp(x->c, xfidread);
        return nil;
    }
    // else, a directory

    o = x->offset;
    e = x->offset + x->count;
    clock = getclock();
    b = malloc(messagesize-IOHDRSZ);	/* avoid memset of emalloc */
    /*s: [[filsysread()]] sanity check b */
    if(b == nil)
        return filsysrespond(fs, x, &fc, "out of memory");
    /*e: [[filsysread()]] sanity check b */

    n = 0;
    switch(FILE(f->qid)){
    /*s: [[filsysread()]] cases */
    case Qwsys:

        qlock(&all);
        ids = emalloc(nwindow * sizeof(int));
        for(j=0; j<nwindow; j++)
            ids[j] = windows[j]->id;
        qunlock(&all);

        qsort(ids, nwindow, sizeof ids[0], idcmp);
        dt.name = buf;
        for(i=0, j=0; j<nwindow && i<e; i+=len){
            k = ids[j];
            sprint(dt.name, "%d", k);
            dt.qid = QID(k, Qdir);
            dt.type = QTDIR;
            dt.perm = DMDIR|0700;
            len = dostat(fs, k, &dt, b+n, x->count-n, clock);
            if(len == 0)
                break;
            if(i >= o)
                n += len;
            j++;
        }
        free(ids);
        break;
    /*x: [[filsysread()]] cases */
    case Qdir:
    case Qwsysdir:
        d = dirtab;
        d++;	/* first entry is '.' */
        for(i=0; d->name != nil && i<e; i+=len){
            len = dostat(fs, WIN(x->f->qid), d, b+n, x->count-n, clock);
            if(len <= BIT16SZ)
                break;
            if(i >= o)
                n += len;
            d++;
        }
        break;
    /*e: [[filsysread()]] cases */
    }

    fc.data = (char*)b;
    fc.count = n;
    filsysrespond(fs, x, &fc, nil);
    free(b);
    return x;
}
/*e: function filsysread */

/*s: function filsyswrite */
static
Xfid*
filsyswrite(Filsys*, Xfid *x, Fid*)
{
    sendp(x->c, xfidwrite);
    return nil;
}
/*e: function filsyswrite */

/*s: function filsysclunk */
static
Xfid*
filsysclunk(Filsys *fs, Xfid *x, Fid *f)
{
    Fcall fc;

    if(f->open){
        f->busy = false;
        f->open = false;
        sendp(x->c, xfidclose);
        return nil;
    }
    // else
    if(f->w)
        sendp(winclosechan, f->w);
    f->busy = false;
    f->open = false;
    return filsysrespond(fs, x, &fc, nil);
}
/*e: function filsysclunk */

/*s: function filsysremove */
static
Xfid*
filsysremove(Filsys *fs, Xfid *x, Fid*)
{
    Fcall fc;

    return filsysrespond(fs, x, &fc, Eperm);
}
/*e: function filsysremove */

/*s: function filsysstat */
static
Xfid*
filsysstat(Filsys *fs, Xfid *x, Fid *f)
{
    Fcall fc;

    fc.stat = emalloc(messagesize-IOHDRSZ);
    fc.nstat = dostat(fs, WIN(x->f->qid), f->dir, fc.stat, messagesize-IOHDRSZ, getclock());
    x = filsysrespond(fs, x, &fc, nil);
    free(fc.stat);
    return x;
}
/*e: function filsysstat */

/*s: function filsyswstat */
static
Xfid*
filsyswstat(Filsys *fs, Xfid *x, Fid*)
{
    Fcall fc;

    return filsysrespond(fs, x, &fc, Eperm);
}
/*e: function filsyswstat */

/*s: function newfid */
static
Fid*
newfid(Filsys *fs, int fid)
{
    Fid *f, *ff, **fh;

    ff = nil;
    fh = &fs->fids[fid&(Nhash-1)];

    // lookup_hash(fid, fs->fids)
    for(f=*fh; f; f=f->next) {
        if(f->fid == fid)
            return f;
        else if(ff==nil && f->busy==false)
            ff = f;
    }
    if(ff){
        ff->fid = fid;
        return ff;
    }
    // else

    f = emalloc(sizeof(Fid));
    f->fid = fid;

    // insert_hash(f, fs->fids)
    f->next = *fh;
    *fh = f;

    return f;
}
/*e: function newfid */

/*s: function getclock */
static
uint
getclock(void)
{
    char buf[32];

    seek(clockfd, 0, 0);
    read(clockfd, buf, sizeof buf);
    return atoi(buf);
}
/*e: function getclock */

/*s: function dostat */
static
int
dostat(Filsys *fs, int id, Dirtab *dir, uchar *buf, int nbuf, uint clock)
{
    Dir d;

    d.qid.path = QID(id, dir->qid);
    if(dir->qid == Qsnarf)
        d.qid.vers = snarfversion;
    else
        d.qid.vers = 0;
    d.qid.type = dir->type;
    d.mode = dir->perm;
    d.length = 0;	/* would be nice to do better */
    d.name = dir->name;
    d.uid = fs->user;
    d.gid = fs->user;
    d.muid = fs->user;
    d.atime = clock;
    d.mtime = clock;

    return convD2M(&d, buf, nbuf);
}
/*e: function dostat */
/*e: windows/rio/fsys.c */
