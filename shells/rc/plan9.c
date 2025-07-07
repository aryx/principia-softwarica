/*s: rc/plan9.c */
/*
 * Plan 9 versions of system-specific functions
 *	By convention, exported routines herein have names beginning with an
 *	upper case letter.
 */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

// could be in main.c
/*s: global [[Rcmain]] */
char *Rcmain = "/rc/lib/rcmain";
/*e: global [[Rcmain]] */
/*s: function [[Isatty]] */
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
/*e: function [[Isatty]] */

// could be in rc.h
/*s: enum [[MiscPlan9]] */
enum {
    Maxenvname = 256,	/* undocumented limit */
};
/*e: enum [[MiscPlan9]] */

// defined in trap.c
extern int trap[NSIG];
// defined in exec.c
extern fdt envdir;

// could be in env.c
/*s: function [[Vinit]] */
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
/*e: function [[Vinit]] */
/*s: function [[addenv]] */
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
/*e: function [[addenv]] */
/*s: function [[updenvlocal]] */
void
updenvlocal(var *v)
{
    if(v){
        updenvlocal(v->next);
        addenv(v);
    }
}
/*e: function [[updenvlocal]] */
/*s: function [[Updenv]] */
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
/*e: function [[Updenv]] */

// could be in glob.c
/*s: constant [[NDIR]] */
#define	NDIR	256		/* shoud be a better way */
/*e: constant [[NDIR]] */
/*s: function [[Globsize]] */
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
/*e: function [[Globsize]] */

// could be in processes.c
/*s: global [[Fdprefix]] */
char *Fdprefix = "/fd/";
/*e: global [[Fdprefix]] */

// in processes.c
extern int *waitpids;
extern int nwaitpids;

/*s: function [[delwaitpid]] */
void
delwaitpid(int pid)
{
    int r, w;
    
    for(r=w=0; r<nwaitpids; r++)
        if(waitpids[r] != pid)
            waitpids[w++] = waitpids[r];
    nwaitpids = w;
}
/*e: function [[delwaitpid]] */
/*s: function [[havewaitpid]] */
bool
havewaitpid(int pid)
{
    int i;

    for(i=0; i<nwaitpids; i++)
        if(waitpids[i] == pid)
            return true;
    return false;
}
/*e: function [[havewaitpid]] */
/*s: function [[Waitfor]] */
int
Waitfor(int pid, bool _persist)
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
/*e: function [[Waitfor]] */

// could be in trap.c
/*s: global [[signame]] */
char *signame[] = {
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
/*e: global [[signame]] */
/*s: global [[syssigname]] */
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
/*e: global [[syssigname]] */
/*s: global [[interrupted]] */
bool interrupted = false;
/*e: global [[interrupted]] */
/*s: function [[notifyf]] */
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
/*e: function [[notifyf]] */
/*s: function [[Trapinit]] */
void
Trapinit(void)
{
    notify(notifyf);
}
/*e: function [[Trapinit]] */
/*s: function [[Eintr]] */
bool
Eintr(void)
{
    return interrupted;
}
/*e: function [[Eintr]] */
/*s: function [[Noerror]] */
void
Noerror(void)
{
    interrupted = false;
}
/*e: function [[Noerror]] */

// could be in fmt.c
/*s: function [[_efgfmt]] */
/* avoid loading any floating-point library code */
//@Scheck: weird, probably linker trick
int _efgfmt(Fmt *)
{
    return -1;
}
/*e: function [[_efgfmt]] */

// could be in error.c
/*s: function [[Exit]] */
void
Exit(char *stat, char* _LOC)
{
    USED(_LOC);
    Updenv();
    setstatus(stat);
    exits(truestatus() ? "" : getstatus());
}
/*e: function [[Exit]] */

// could be in exec.c
/*s: function [[Xrdfn]] */
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
/*e: function [[Xrdfn]] */

// could be in utils.c
/*s: function [[Malloc]] */
void*
Malloc(ulong n)
{
    return mallocz(n, 1);
}
/*e: function [[Malloc]] */

// could be in utils.c
/*s: constant [[NFD]] */
#define	NFD	50
/*e: constant [[NFD]] */
/*s: struct [[DirEntryWrapper]] */
struct DirEntryWrapper {
    Dir	*dbuf;
    int	i;
    int	n;
};
/*e: struct [[DirEntryWrapper]] */
/*s: global [[dir]] */
struct DirEntryWrapper dir[NFD];
/*e: global [[dir]] */
/*s: function [[trimdirs]] */
static int
trimdirs(Dir *d, int nd)
{
    int r, w;

    for(r=w=0; r<nd; r++)
        if(d[r].mode&DMDIR)
            d[w++] = d[r];
    return w;
}
/*e: function [[trimdirs]] */
/*s: function [[Readdir]] */
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
/*e: function [[Readdir]] */
/*s: function [[Opendir]] */
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
/*e: function [[Opendir]] */
/*s: function [[Closedir]] */
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
/*e: function [[Closedir]] */

/*e: rc/plan9.c */
