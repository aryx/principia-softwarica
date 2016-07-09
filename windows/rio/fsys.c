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
    { "screen",		QTFILE,	Qscreen,	0400 },
    /*x: dirtab array elements */
    { "wsys",		QTDIR,	Qwsys,		0500|DMDIR },
    /*x: dirtab array elements */
    { "wctl",		QTFILE,	Qwctl,		0600 },
    /*x: dirtab array elements */
    { "snarf",		QTFILE,	Qsnarf,		0600 },
    /*x: dirtab array elements */
    { "wdir",		QTFILE,	Qwdir,		0600 },
    /*x: dirtab array elements */
    { "kbdin",		QTFILE,	Qkbdin,		0200 },
    /*e: dirtab array elements */
    { nil, }
};
/*e: global dirtab */

static uint		getclock(void);
Fid*		newfid(Filsys*, int);
static int		dostat(Filsys*, int, Dirtab*, uchar*, int, uint);

/*s: global clockfd */
fdt	clockfd;
/*e: global clockfd */
/*s: global firstmessage */
bool	firstmessage = true;
/*e: global firstmessage */


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
    [Tflush]   = filsysflush,
    /*x: [[fcall]] other methods */
    [Tversion] = filsysversion,
    [Tauth]    = filsysauth,
    /*e: [[fcall]] other methods */
};
/*e: global fcall */




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
    int o, e;
    uint clock;
    byte *b;
    int n;
    Fcall fc;
    /*s: [[filsysread()]] other locals */
    int i, j, k;
    int len;
    int *ids;
    Dirtab dt;
    char buf[16];
    /*x: [[filsysread()]] other locals */
    Dirtab *d;
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
