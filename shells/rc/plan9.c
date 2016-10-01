/*s: rc/plan9.c */
/*
 * Plan 9 versions of system-specific functions
 *	By convention, exported routines herein have names beginning with an
 *	upper case letter.
 */
#include "rc.h"
#include "exec.h"
#include "io.h"
#include "fns.h"
#include "getflags.h"

char**	mkargv(word*);
void	delwaitpid(int);
int	havewaitpid(int);

/*s: enum _anon_ (rc/plan9.c) */
enum {
    Maxenvname = 256,	/* undocumented limit */
};
/*e: enum _anon_ (rc/plan9.c) */

/*s: global Signame */
char *Signame[] = {
    "sigexit",	
    "sighup",	
    "sigint",	
    "sigquit",
    "sigalrm",	
    "sigkill",	
    "sigfpe",	
    "sigterm",
    0
};
/*e: global Signame */
/*s: global syssigname */
char *syssigname[] = {
    "exit",		/* can't happen */
    "hangup",
    "interrupt",
    "quit",		/* can't happen */
    "alarm",
    "kill",
    "sys: fp: ",
    "term",
    0
};
/*e: global syssigname */
/*s: global Rcmain */
char *Rcmain = "/rc/lib/rcmain";
/*e: global Rcmain */
/*s: global Fdprefix */
char *Fdprefix = "/fd/";
/*e: global Fdprefix */

void execfinit(void);
void execnewpgrp(void);

/*s: global Builtin */
builtin Builtin[] = {
    "cd",		execcd,
    "exit",		execexit,
    ".",		execdot,
    "eval",		execeval,

    "whatis",		execwhatis,

    "exec",		execexec,	/* but with popword first */
    "rfork",		execnewpgrp,
    "wait",		execwait,

    "shift",		execshift,
    "finit",		execfinit,
    "flag",		execflag,
    0
};
/*e: global Builtin */

/*s: function execnewpgrp */
void
execnewpgrp(void)
{
    int arg;
    char *s;
    switch(count(runq->argv->words)){
    case 1:
        arg = RFENVG|RFNAMEG|RFNOTEG;
        break;
    case 2:
        arg = 0;
        for(s = runq->argv->words->next->word;*s;s++) switch(*s){
        default:
            goto Usage;
        case 'n':
            arg|=RFNAMEG;  break;
        case 'N':
            arg|=RFCNAMEG;
            break;
        case 'm':
            arg|=RFNOMNT;  break;
        case 'e':
            arg|=RFENVG;   break;
        case 'E':
            arg|=RFCENVG;  break;
        case 's':
            arg|=RFNOTEG;  break;
        case 'f':
            arg|=RFFDG;    break;
        case 'F':
            arg|=RFCFDG;   break;
        }
        break;
    default:
    Usage:
        pfmt(err, "Usage: %s [fnesFNEm]\n", runq->argv->words->word);
        setstatus("rfork usage");
        poplist();
        return;
    }
    if(rfork(arg)==-1){
        pfmt(err, "rc: %s failed\n", runq->argv->words->word);
        setstatus("rfork failed");
    }
    else
        setstatus("");
    poplist();
}
/*e: function execnewpgrp */

/*s: function Vinit */
void
Vinit(void)
{
    int dir, f, len, i, n, nent;
    char *buf, *s;
    char envname[Maxenvname];
    word *val;
    Dir *ent;

    dir = open("/env", OREAD);
    if(dir<0){
        pfmt(err, "rc: can't open /env: %r\n");
        return;
    }
    ent = nil;
    for(;;){
        nent = dirread(dir, &ent);
        if(nent <= 0)
            break;
        for(i = 0; i<nent; i++){
            len = ent[i].length;
            if(len && strncmp(ent[i].name, "fn#", 3)!=0){
                snprint(envname, sizeof envname, "/env/%s", ent[i].name);
                if((f = open(envname, 0))>=0){
                    buf = emalloc(len+1);
                    n = readn(f, buf, len);
                    if (n <= 0)
                        buf[0] = '\0';
                    else
                        buf[n] = '\0';
                    val = 0;
                    /* Charitably add a 0 at the end if need be */
                    if(buf[len-1])
                        buf[len++]='\0';
                    s = buf+len-1;
                    for(;;){
                        while(s!=buf && s[-1]!='\0') --s;
                        val = newword(s, val);
                        if(s==buf)
                            break;
                        --s;
                    }
                    setvar(ent[i].name, val);
                    vlook(ent[i].name)->changed = false;
                    close(f);
                    efree(buf);
                }
            }
        }
        free(ent);
    }
    close(dir);
}
/*e: function Vinit */
/*s: global envdir */
int envdir;
/*e: global envdir */

/*s: function Xrdfn */
void
Xrdfn(void)
{
    int f, len;
    Dir *e;
    char envname[Maxenvname];
    static Dir *ent, *allocent;
    static int nent;

    for(;;){
        if(nent == 0){
            free(allocent);
            nent = dirread(envdir, &allocent);
            ent = allocent;
        }
        if(nent <= 0)
            break;
        while(nent){
            e = ent++;
            nent--;
            len = e->length;
            if(len && strncmp(e->name, "fn#", 3)==0){
                snprint(envname, sizeof envname, "/env/%s", e->name);
                if((f = open(envname, 0))>=0){
                    execcmds(openfd(f));
                    return;
                }
            }
        }
    }
    close(envdir);
    Xreturn();
}
/*e: function Xrdfn */
/*s: global rdfns */
union Code rdfns[4];
/*e: global rdfns */

/*s: function execfinit */
void
execfinit(void)
{
    static int first = 1;
    if(first){
        rdfns[0].i = 1;
        rdfns[1].f = Xrdfn;
        rdfns[2].f = Xjump;
        rdfns[3].i = 1;
        first = 0;
    }
    Xpopm();
    envdir = open("/env", 0);
    if(envdir<0){
        pfmt(err, "rc: can't open /env: %r\n");
        return;
    }
    start(rdfns, 1, runq->local);
}
/*e: function execfinit */

/*s: function Waitfor */
int
Waitfor(int pid, int)
{
    thread *p;
    Waitmsg *w;
    char errbuf[ERRMAX];

    if(pid >= 0 && !havewaitpid(pid))
        return 0;

    // wait()!! until we found it
    while((w = wait()) != nil){
        delwaitpid(w->pid);

        if(w->pid==pid){
            setstatus(w->msg);
            free(w);
            return 0;
        }
        /*s: [[Waitfor()]] in while loop, if wait returns another pid */
        // else
        for(p = runq->ret;p;p = p->ret)
            if(p->pid==w->pid){
                p->pid=-1;
                strcpy(p->status, w->msg);
            }
        free(w);
        /*e: [[Waitfor()]] in while loop, if wait returns another pid */
    }

    errstr(errbuf, sizeof errbuf);
    if(strcmp(errbuf, "interrupted")==0) 
        return -1;
    return 0;
}
/*e: function Waitfor */

/*s: function mkargv */
char **
mkargv(word *a)
{
    char **argv = (char **)emalloc((count(a)+2) * sizeof(char *));
    char **argp = argv+1;	/* leave one at front for runcoms */

    for(;a;a = a->next) 
        *argp++=a->word;
    *argp = nil;
    return argv;
}
/*e: function mkargv */

/*s: function addenv */
void
addenv(var *v)
{
    char envname[Maxenvname];
    word *w;
    int f;
    io *fd;

    if(v->changed){
        v->changed = false;
        snprint(envname, sizeof envname, "/env/%s", v->name);
        if((f = Creat(envname))<0)
            pfmt(err, "rc: can't open %s: %r\n", envname);
        else{
            for(w = v->val;w;w = w->next)
                write(f, w->word, strlen(w->word)+1L);
            close(f);
        }
    }
    if(v->fnchanged){
        v->fnchanged = false;
        snprint(envname, sizeof envname, "/env/fn#%s", v->name);
        if((f = Creat(envname))<0)
            pfmt(err, "rc: can't open %s: %r\n", envname);
        else{
            if(v->fn){
                fd = openfd(f);
                pfmt(fd, "fn %q %s\n", v->name, v->fn[v->pc-1].s);
                closeio(fd);
            }
            close(f);
        }
    }
}
/*e: function addenv */

/*s: function updenvlocal */
void
updenvlocal(var *v)
{
    if(v){
        updenvlocal(v->next);
        addenv(v);
    }
}
/*e: function updenvlocal */

/*s: function Updenv */
void
Updenv(void)
{
    var *v, **h;

    for(h = gvar;h!=&gvar[NVAR];h++)
        for(v=*h;v;v = v->next)
            addenv(v);
    if(runq)
        updenvlocal(runq->local);
}
/*e: function Updenv */


/*s: function Execute */
void
Execute(word *args, word *path)
{
    char **argv = mkargv(args);
    char file[1024];
    char errstr[1024];
    int nc;

    Updenv();
    errstr[0] = '\0';

    for(;path;path = path->next){
        nc = strlen(path->word);
        if(nc < sizeof file - 1){	/* 1 for / */
            strcpy(file, path->word);
            if(file[0]){
                strcat(file, "/");
                nc++;
            }
            if(nc + strlen(argv[1]) < sizeof file){
                strcat(file, argv[1]);

                // The actual exec() system call!
                exec(file, argv+1);

                // reached if the file does not exist

                rerrstr(errstr, sizeof errstr);
                /*
                 * if file exists and is executable, exec should
                 * have worked, unless it's a directory or an
                 * executable for another architecture.  in
                 * particular, if it failed due to lack of
                 * swap/vm (e.g., arg. list too long) or other
                 * allocation failure, stop searching and print
                 * the reason for failure.
                 */
                if (strstr(errstr, " allocat") != nil ||
                    strstr(errstr, " full") != nil)
                    break;
            }
            else werrstr("command name too long");
        }
    }
    // should not be reached if found an actual binary to exec
    pfmt(err, "%s: %s\n", argv[1], errstr);
    efree((char *)argv);
}
/*e: function Execute */
/*s: constant NDIR */
#define	NDIR	256		/* shoud be a better way */
/*e: constant NDIR */

/*s: function Globsize */
int
Globsize(char *p)
{
    int isglob = 0, globlen = NDIR+1;
    for(;*p;p++){
        if(*p==GLOB){
            p++;
            if(*p!=GLOB)
                isglob++;
            globlen+=*p=='*'?NDIR:1;
        }
        else
            globlen++;
    }
    return isglob?globlen:0;
}
/*e: function Globsize */
/*s: constant NFD */
#define	NFD	50
/*e: constant NFD */

/*s: global dir */
struct{
    Dir	*dbuf;
    int	i;
    int	n;
}dir[NFD];
/*e: global dir */

/*s: function Opendir */
int
Opendir(char *name)
{
    Dir *db;
    int f;
    f = open(name, 0);
    if(f==-1)
        return f;
    db = dirfstat(f);
    if(db!=nil && (db->mode&DMDIR)){
        if(f<NFD){
            dir[f].i = 0;
            dir[f].n = 0;
        }
        free(db);
        return f;
    }
    free(db);
    close(f);
    return -1;
}
/*e: function Opendir */

/*s: function trimdirs */
static int
trimdirs(Dir *d, int nd)
{
    int r, w;

    for(r=w=0; r<nd; r++)
        if(d[r].mode&DMDIR)
            d[w++] = d[r];
    return w;
}
/*e: function trimdirs */

/*s: function Readdir */
/*
 * onlydirs is advisory -- it means you only
 * need to return the directories.  it's okay to
 * return files too (e.g., on unix where you can't
 * tell during the readdir), but that just makes 
 * the globber work harder.
 */
int
Readdir(int f, void *p, int onlydirs)
{
    int n;

    if(f<0 || f>=NFD)
        return 0;
Again:
    if(dir[f].i==dir[f].n){	/* read */
        free(dir[f].dbuf);
        dir[f].dbuf = 0;
        n = dirread(f, &dir[f].dbuf);
        if(n>0){
            if(onlydirs){
                n = trimdirs(dir[f].dbuf, n);
                if(n == 0)
                    goto Again;
            }	
            dir[f].n = n;
        }else
            dir[f].n = 0;
        dir[f].i = 0;
    }
    if(dir[f].i == dir[f].n)
        return 0;
    strcpy(p, dir[f].dbuf[dir[f].i].name);
    dir[f].i++;
    return 1;
}
/*e: function Readdir */

/*s: function Closedir */
void
Closedir(int f)
{
    if(f>=0 && f<NFD){
        free(dir[f].dbuf);
        dir[f].i = 0;
        dir[f].n = 0;
        dir[f].dbuf = 0;
    }
    close(f);
}
/*e: function Closedir */
/*s: global interrupted */
bool interrupted = false;
/*e: global interrupted */
/*s: function notifyf */
void
notifyf(void*, char *s)
{
    int i;
    for(i = 0;syssigname[i];i++) 
     if(strncmp(s, syssigname[i], strlen(syssigname[i]))==0){
        if(strncmp(s, "sys: ", 5)!=0) 
            interrupted = true;
        goto Out;
    }
    pfmt(err, "rc: note: %s\n", s);
    noted(NDFLT);
    return;
Out:
    if(strcmp(s, "interrupt")!=0 || trap[i]==0){
        trap[i]++;
        ntrap++;
    }
    if(ntrap>=32){	/* rc is probably in a trap loop */
        pfmt(err, "rc: Too many traps (trap %s), aborting\n", s);
        abort();
    }
    noted(NCONT);
}
/*e: function notifyf */

/*s: function Trapinit */
void
Trapinit(void)
{
    notify(notifyf);
}
/*e: function Trapinit */

/*s: function Unlink */
void
Unlink(char *name)
{
    remove(name);
}
/*e: function Unlink */

/*s: function Write */
long
Write(int fd, void *buf, long cnt)
{
    return write(fd, buf, cnt);
}
/*e: function Write */

/*s: function Read */
long
Read(int fd, void *buf, long cnt)
{
    return read(fd, buf, cnt);
}
/*e: function Read */

/*s: function Seek */
long
Seek(int fd, long cnt, long whence)
{
    return seek(fd, cnt, whence);
}
/*e: function Seek */

/*s: function Executable */
bool
Executable(char *file)
{
    Dir *statbuf;
    bool ret;

    statbuf = dirstat(file);
    if(statbuf == nil)
        return false;
    ret = ((statbuf->mode&0111)!=0 && (statbuf->mode&DMDIR)==0);
    free(statbuf);
    return ret;
}
/*e: function Executable */

/*s: function Creat */
int
Creat(char *file)
{
    return create(file, 1, 0666L);
}
/*e: function Creat */

/*s: function Dup */
int
Dup(int a, int b)
{
    return dup(a, b);
}
/*e: function Dup */

/*s: function Dup1 */
int
Dup1(int)
{
    return -1;
}
/*e: function Dup1 */

/*s: function Exit */
void
Exit(char *stat)
{
    Updenv();
    setstatus(stat);
    exits(truestatus() ? "" : getstatus());
}
/*e: function Exit */

/*s: function Eintr */
bool
Eintr(void)
{
    return interrupted;
}
/*e: function Eintr */

/*s: function Noerror */
void
Noerror(void)
{
    interrupted = false;
}
/*e: function Noerror */

/*s: function Isatty */
bool
Isatty(fdt fd)
{
    char buf[64];

    if(fd2path(fd, buf, sizeof buf) != 0)
        return false;

    /* might be #c/cons during boot - fixed 22 april 2005, remove this later */
    if(strcmp(buf, "#c/cons") == 0)
        return true;

    /* might be /mnt/term/dev/cons */
    return strlen(buf) >= 9 && strcmp(buf+strlen(buf)-9, "/dev/cons") == 0;
}
/*e: function Isatty */

/*s: function Abort */
void
Abort(void)
{
    pfmt(err, "aborting\n");
    flush(err);
    Exit("aborting");
}
/*e: function Abort */

/*s: function Memcpy */
void
Memcpy(void *a, void *b, long n)
{
    memmove(a, b, n);
}
/*e: function Memcpy */

/*s: function Malloc */
void*
Malloc(ulong n)
{
    return mallocz(n, 1);
}
/*e: function Malloc */

/*s: global waitpids */
// growing_array<pid> (but really a list)
int *waitpids;
/*e: global waitpids */
/*s: global nwaitpids */
int nwaitpids;
/*e: global nwaitpids */

/*s: function addwaitpid */
void
addwaitpid(int pid)
{
    waitpids = realloc(waitpids, (nwaitpids+1)*sizeof waitpids[0]);
    if(waitpids == nil)
        panic("Can't realloc %d waitpids", nwaitpids+1);
    waitpids[nwaitpids++] = pid;
}
/*e: function addwaitpid */

/*s: function delwaitpid */
void
delwaitpid(int pid)
{
    int r, w;
    
    for(r=w=0; r<nwaitpids; r++)
        if(waitpids[r] != pid)
            waitpids[w++] = waitpids[r];
    nwaitpids = w;
}
/*e: function delwaitpid */

/*s: function clearwaitpids */
void
clearwaitpids(void)
{
    nwaitpids = 0;
}
/*e: function clearwaitpids */

/*s: function havewaitpid */
bool
havewaitpid(int pid)
{
    int i;

    for(i=0; i<nwaitpids; i++)
        if(waitpids[i] == pid)
            return true;
    return false;
}
/*e: function havewaitpid */

/*s: function _efgfmt */
/* avoid loading any floating-point library code */
//@Scheck: weird, probably linker trick
int _efgfmt(Fmt *)
{
    return -1;
}
/*e: function _efgfmt */
/*e: rc/plan9.c */
