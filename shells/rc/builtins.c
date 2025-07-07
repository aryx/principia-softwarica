/*s: rc/builtins.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

/*s: function [[dochdir]] */
errorneg1
dochdir(char *word)
{
    /*s: [[dochdir()]] locals */
    /* report to /dev/wdir if it exists and we're interactive */
    static fdt wdirfd = -2;
    /*e: [[dochdir()]] locals */

    // the actual syscall
    if(chdir(word)<0) 
        return ERROR_NEG1;

    /*s: [[dochdir()]] adjust [[/dev/wdir]] if run under [[rio]] */
    if(flag['i']!=nil){
        if(wdirfd==-2)  /* try only once */
            wdirfd = open("/dev/wdir", OWRITE); // TODO: |OCEXEC but plan9 specific?
        if(wdirfd>=0) {
            //fcntl(wdirfd, F_SETFD, FD_CLOEXEC);
            write(wdirfd, word, strlen(word));
        }
    }
    /*e: [[dochdir()]] adjust [[/dev/wdir]] if run under [[rio]] */
    return OK_1;
}
/*e: function [[dochdir]] */

/*s: function [[appfile]] */
static char *
appfile(char *dir, char *comp)
{
    int dirlen, complen;
    char *s, *p;

    dirlen = strlen(dir);
    complen = strlen(comp);
    s = emalloc(dirlen + 1 + complen + 1);
    memmove(s, dir, dirlen);
    p = s + dirlen;
    *p++ = '/';
    memmove(p, comp, complen);
    p[complen] = '\0';
    return s;
}
/*e: function [[appfile]] */

/*s: function [[mapfd]] */
int
mapfd(int fd)
{
    redir *rp;
    for(rp = runq->redir;rp;rp = rp->next){
        switch(rp->type){
        case RCLOSE:
            if(rp->from==fd)
                fd=-1;
            break;
        case RDUP:
        case ROPEN:
            if(rp->to==fd)
                fd = rp->from;
            break;
        }
    }
    return fd;
}
/*e: function [[mapfd]] */


/*s: function [[execcd]] */
void
execcd(void)
{
    word *a = runq->argv->words;
    word *cdpath;
    char *dir;

    setstatus("can't cd");
    cdpath = vlook("cdpath")->val;

    switch(count(a)){
    case 1:
        a = vlook("home")->val;
        if(count(a)>=1){
            if(dochdir(a->word)>=0)
                setstatus("");
            else
                pfmt(err, "Can't cd %s: %r\n", a->word);
        }
        else
            pfmt(err, "Can't cd -- $home empty\n"); //$
        break;
    case 2:
        if(a->next->word[0]=='/' || cdpath==nil)
            cdpath = &nullpath;
        for(; cdpath; cdpath = cdpath->next){
            if(cdpath->word[0] != '\0')
                dir = appfile(cdpath->word, a->next->word);
            else
                dir = strdup(a->next->word);

            if(dochdir(dir) >= 0){
                if(cdpath->word[0] != '\0' &&
                    strcmp(cdpath->word, ".") != 0)
                    pfmt(err, "%s\n", dir);
                free(dir);
                setstatus("");
                break;
            }
            free(dir);
        }
        if(cdpath==nil)
            pfmt(err, "Can't cd %s: %r\n", a->next->word);
        break;
    default:
        pfmt(err, "Usage: cd [directory]\n");
        break;
    }
    poplist();
}
/*e: function [[execcd]] */

/*s: function [[execexit]] */
void
execexit(void)
{
    switch(count(runq->argv->words)){
    default:
        pfmt(err, "Usage: exit [status]\nExiting anyway\n");
        // FALLTHROUGH
    case 2:
        setstatus(runq->argv->words->next->word);
        // FALLTHROUGH
    case 1: 
        Xexit();
    }
}
/*e: function [[execexit]] */

/*s: function [[execeval]] */
void
execeval(void)
{
    char *cmdline, *s, *t;
    int len = 0;
    word *ap;

    if(count(runq->argv->words)<=1){
        Xerror1("Usage: eval cmd ...");
        return;
    }
    eflagok = true;
    for(ap = runq->argv->words->next;ap;ap = ap->next)
        len+=1+strlen(ap->word);

    cmdline = emalloc(len);
    s = cmdline;
    for(ap = runq->argv->words->next;ap;ap = ap->next){
        for(t = ap->word;*t;) *s++=*t++;
        *s++=' ';
    }
    s[-1]='\n';
    poplist();

    execcmds(opencore(cmdline, len));
    efree(cmdline);
}
/*e: function [[execeval]] */

/*s: function [[execflag]] */
void
execflag(void)
{
    char *letter, *val;

    switch(count(runq->argv->words)){
    case 2:
        setstatus(flag[(uchar)runq->argv->words->next->word[0]]?"":"flag not set");
        break;
    case 3:
        letter = runq->argv->words->next->word;
        val = runq->argv->words->next->next->word;
        if(strlen(letter)==1){
            if(strcmp(val, "+")==0){
                flag[(uchar)letter[0]] = flagset;
                break;
            }
            if(strcmp(val, "-")==0){
                flag[(uchar)letter[0]] = 0;
                break;
            }
        }
    default:
        Xerror1("Usage: flag [letter] [+-]");
        return;
    }
    poplist();
}
/*e: function [[execflag]] */

// was in plan9.c

/*s: function [[Executable]] */
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
/*e: function [[Executable]] */

/*s: function [[execwhatis]] */
void
execwhatis(void){   /* mildly wrong -- should fork before writing */
    word *a, *b, *path;
    var *v;
    struct Builtin *bp;
    char *file;
    struct Io out[1];
    int found, sep;
    a = runq->argv->words->next;
    if(a==0){
        Xerror1("Usage: whatis name ...");
        return;
    }
    setstatus("");
    memset(out, 0, sizeof out);
    out->fd = mapfd(1);
    out->bufp = out->buf;
    out->ebuf = &out->buf[NBUF];
    out->strp = nil;
    for(;a;a = a->next){
        v = vlook(a->word);
        if(v->val){
            pfmt(out, "%s=", a->word);
            if(v->val->next==0)
                pfmt(out, "%q\n", v->val->word);
            else{
                sep='(';
                for(b = v->val;b && b->word;b = b->next){
                    pfmt(out, "%c%q", sep, b->word);
                    sep=' ';
                }
                pfmt(out, ")\n");
            }
            found = 1;
        }
        else
            found = 0;
        v = gvlook(a->word);
        if(v->fn)
            pfmt(out, "fn %q %s\n", v->name, v->fn[v->pc-1].s);
        else{
            for(bp = builtins;bp->name;bp++)
                if(strcmp(a->word, bp->name)==0){
                    pfmt(out, "builtin %s\n", a->word);
                    break;
                }
            if(!bp->name){
                for(path = searchpath(a->word); path;
                    path = path->next){
                    if(path->word[0] != '\0')
                        file = appfile(path->word, a->word);
                    else
                        file = strdup(a->word);
                    if(Executable(file)){
                        pfmt(out, "%s\n", file);
                        free(file);
                        break;
                    }
                    free(file);
                }
                if(!path && !found){
                    pfmt(err, "%s: not found\n", a->word);
                    setstatus("not found");
                }
            }
        }
    }
    poplist();
    flush(err);
}
/*e: function [[execwhatis]] */


/*s: function [[execshift]] */
void
execshift(void)
{
    int n;
    word *a;
    var *star;
    switch(count(runq->argv->words)){
    default:
        pfmt(err, "Usage: shift [n]\n");
        setstatus("shift usage");
        poplist();
        return;
    case 2:
        n = atoi(runq->argv->words->next->word);
        break;
    case 1:
        n = 1;
        break;
    }
    star = vlook("*");
    for(;n && star->val;--n){
        a = star->val->next;
        efree(star->val->word);
        efree((char *)star->val);
        star->val = a;
        star->changed = true;
    }
    setstatus("");
    poplist();
}
/*e: function [[execshift]] */


/*s: global [[dotcmds]] */
union Code dotcmds[14];
/*e: global [[dotcmds]] */

/*s: function [[execdot]] */
void
execdot(void)
{
    thread *p = runq;
    bool iflag = false;
    fdt fd;
    list *av;
    char *zero; // new stdin
    char *file;
    word *path;
    /*s: [[execdot()]] other locals */
    static bool first = true;
    /*e: [[execdot()]] other locals */

    /*s: [[execdot()]] if first */
    if(first){
        dotcmds[0].i = 1;
        dotcmds[1].f = Xmark;
          dotcmds[2].f = Xword;
          dotcmds[3].s = "0";
        dotcmds[4].f = Xlocal; // will pop_list twice

        dotcmds[5].f = Xmark;
          dotcmds[6].f = Xword;
          dotcmds[7].s="*";
        dotcmds[8].f = Xlocal; // will pop_list twice

        dotcmds[9].f = Xrdcmds; // =~ a REPL

        dotcmds[10].f = Xunlocal;
        dotcmds[11].f = Xunlocal;
        dotcmds[12].f = Xreturn;

        first = false;
    }
    /*e: [[execdot()]] if first */
    /*s: [[execdot()]] if not first execution */
    else
        eflagok = true;
    /*e: [[execdot()]] if not first execution */

    popword(); // "."
    if(p->argv->words && strcmp(p->argv->words->word, "-i")==0){
        iflag = true;
        popword();
    }

    /* get input file */
    if(p->argv->words==nil){
        Xerror1("Usage: . [-i] file [arg ...]");
        return;
    }
    zero = strdup(p->argv->words->word);
    popword();

    fd = -1;
    for(path = searchpath(zero); path; path = path->next){
        if(path->word[0] != '\0')
            file = appfile(path->word, zero);
        else
            file = strdup(zero);

        fd = open(file, 0);
        free(file);
        if(fd >= 0)
            break;
    }
    if(fd<0){
        pfmt(err, "%s: ", zero);
        setstatus("can't open"); // what for? it is reseted by Xerror anyway
        Xerror(".: can't open");
        return;
    }

    /* set up for a new command loop */
    start(dotcmds, 1, (struct Var *)nil);

    pushredir(RCLOSE, fd, 0);
    runq->cmdfile = zero;
    runq->cmdfd = openfd(fd);
    runq->iflag = iflag;

    runq->iflast = false;

    /* push $* value */
    pushlist();
    runq->argv->words = p->argv->words;

    /* free caller's copy of $* */
    av = p->argv;
    p->argv = av->next;
    efree((char *)av);

    /* push $0 value */
    pushlist();
    pushword(zero);

    ndot++;
}
/*e: function [[execdot]] */


/*s: function [[execwait]] */
void
execwait(void)
{
    switch(count(runq->argv->words)){
    default:
        Xerror1("Usage: wait [pid]");
        return;
    case 2:
        Waitfor(atoi(runq->argv->words->next->word), false);
        break;
    case 1:
        Waitfor(-1, false);
        break;
    }
    poplist();
}
/*e: function [[execwait]] */


/*s: function [[execnewpgrp]] */
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
        //pad: commented for goken
        //case 'm':
        //    arg|=RFNOMNT;  break;
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
/*e: function [[execnewpgrp]] */


/*s: global [[rdfns]] */
union Code rdfns[4];
/*e: global [[rdfns]] */

extern fdt envdir;

/*s: function [[execfinit]] */
void
execfinit(void)
{
    //TODO: commented for goken for now because Linux does not have a /env
    // probably need to move to Plan9.c
    //static bool first = true;
    //if(first){
    //    rdfns[0].i = 1;
    //    rdfns[1].f = Xrdfn;
    //    rdfns[2].f = Xjump;
    //    rdfns[3].i = 1;
    //    first = false;
    //}
    //Xpopm(); // pop_list()
    //envdir = open("/env", 0);
    //if(envdir<0){
    //    pfmt(err, "rc: can't open /env: %r\n");
    //    return;
    //}
    //start(rdfns, 1, runq->local);
}
/*e: function [[execfinit]] */


/*s: global [[builtins]] */
builtin builtins[] = {
    "cd"     ,      execcd,
    "exit"   ,      execexit,
    "."      ,      execdot,
    "eval"   ,      execeval,

    "whatis" ,      execwhatis,

    "exec"   ,      execexec,   /* but with popword first */
    "rfork"  ,      execnewpgrp,
    "wait"   ,      execwait,

    "shift"  ,      execshift,
    "finit"  ,      execfinit,
    "flag"   ,      execflag,
    //TODO: unix-specific	"umask",	execumask,
    0
};
/*e: global [[builtins]] */

/*e: rc/builtins.c */
