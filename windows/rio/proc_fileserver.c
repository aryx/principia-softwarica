/*s: windows/rio/proc_fileserver.c */
#include <u.h>
#include <libc.h>

// for dat.h
#include <draw.h>
#include <mouse.h>
#include <cursor.h>
#include <keyboard.h>
#include <frame.h>
#include <fcall.h>
#include <thread.h>

#include "dat.h"
#include "fns.h"

extern Xfid* 	(*fcall[])(Filsys*, Xfid*, Fid*);
extern char	Ebadfcall[];
extern bool	firstmessage;
extern Fid*		newfid(Filsys*, int);
extern fdt	clockfd;

/*s: global srvpipe (windows/rio/fsys.c) */
char	srvpipe[64];
/*e: global srvpipe (windows/rio/fsys.c) */
/*s: global srvwctl (windows/rio/fsys.c) */
char	srvwctl[64];
/*e: global srvwctl (windows/rio/fsys.c) */

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

        if(convM2S(buf, n, &x->req) != n)
            error("convert error in convM2S");
        /*s: [[filsysproc()]] dump Fcall if debug */
        if(DEBUG)
            fprint(STDERR, "rio:<-%F\n", &x->req);
        /*e: [[filsysproc()]] dump Fcall if debug */

        /*s: [[filsysproc()]] sanity check x type */
        if(fcall[x->req.type] == nil)
            x = filsysrespond(fs, x, &fc, Ebadfcall);
        /*e: [[filsysproc()]] sanity check x type */
        else{
            /*s: [[filsysproc()]] if x type is Tversion or Tauth */
            if(x->req.type==Tversion || x->req.type==Tauth)
                f = nil;
            /*e: [[filsysproc()]] if x type is Tversion or Tauth */
            else
                f = newfid(fs, x->req.fid);
            x->f = f;

            // Dispatch
            x  = (*fcall[x->req.type])(fs, x, f);
        }
        /*s: [[filsysproc()]] end of loop */
        firstmessage = false;
        /*e: [[filsysproc()]] end of loop */
    }
}
/*e: function filsysproc */

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


/*s: function filsysinit */
Filsys*
filsysinit(Channel *cxfidalloc)
{
    int pid;
    Filsys *fs;
    /*s: [[filsysinit()]] other locals */
    fdt p0;
    // chan<??> (listener = ??, sender = ??)
    Channel *c;
    /*x: [[filsysinit()]] other locals */
    fdt fd;
    char buf[128];
    int n;
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
            buf[n] = '\0';
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
/*e: windows/rio/proc_fileserver.c */
