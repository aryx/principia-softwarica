/*s: rc/processes.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */
//#include <string.h>

// Fdprefix, delwaitpid(), havewaitpid() Waitfor() are back in plan9.c
extern char *Fdprefix;

/*s: global [[waitpids]] */
// growing_array<pid> (but really a list)
int *waitpids;
/*e: global [[waitpids]] */
/*s: global [[nwaitpids]] */
int nwaitpids;
/*e: global [[nwaitpids]] */
/*s: function [[addwaitpid]] */
void
addwaitpid(int pid)
{
    /*s: [[addwaitpid()]] grow [[waitpids]] if needed */
    waitpids = realloc(waitpids, (nwaitpids+1)*sizeof waitpids[0]);
    if(waitpids == nil)
        panic("Can't realloc %d waitpids", nwaitpids+1);
    /*e: [[addwaitpid()]] grow [[waitpids]] if needed */
    waitpids[nwaitpids++] = pid;
}
/*e: function [[addwaitpid]] */
/*s: function [[clearwaitpids]] */
void
clearwaitpids(void)
{
    nwaitpids = 0;
}
/*e: function [[clearwaitpids]] */


/*s: function [[mkargv]] */
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
/*e: function [[mkargv]] */

/*s: function [[Execute]] */
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
/*e: function [[Execute]] */

// was in havefork.c

/*s: function [[Xasync]] */
void
Xasync(void)
{
    fdt null = open("/dev/null", 0);
    int pid;
    char npid[10];
    if(null<0){
        Xerror("Can't open /dev/null\n");
        return;
    }
    switch(pid = rfork(RFFDG|RFPROC|RFNOTEG)){
    case -1:
        close(null);
        Xerror("try again");
        break;
    case 0: // child
        clearwaitpids();
        pushredir(ROPEN, null, 0);
        // start a new Thread runq->pc+1 so skip pointer to code after &
        start(runq->code, runq->pc+1, runq->local);
        runq->ret = 0;
        break;
    default: // parent
        addwaitpid(pid);
        close(null);
        // jump to code after &
        runq->pc = runq->code[runq->pc].i;
        inttoascii(npid, pid);
        setvar("apid", newword(npid, (word *)nil));
        break;
    }
}
/*e: function [[Xasync]] */

/*s: function [[Xpipe]] */
void
Xpipe(void)
{
    struct Thread *p = runq;
    int pc = p->pc;
    int forkid;
    fdt lfd = p->code[pc++].i;
    fdt rfd = p->code[pc++].i;
    fdt pfd[2];

    if(pipe(pfd)<0){
        Xerror("can't get pipe");
        return;
    }
    switch(forkid = fork()){
    case -1:
        Xerror("try again");
        break;
    case 0: // child
        clearwaitpids();
        // pc+2 so jump the jump addresses
        start(p->code, pc+2, runq->local);
        runq->ret = nil;
        close(pfd[PRD]);
        pushredir(ROPEN, pfd[PWR], lfd);
        break;
    default: // parent
        addwaitpid(forkid);
        start(p->code, p->code[pc].i, runq->local);
        close(pfd[PWR]);
        pushredir(ROPEN, pfd[PRD], rfd);
        p->pc = p->code[pc+1].i;
        p->pid = forkid;
        break;
    }
}
/*e: function [[Xpipe]] */

/*s: function [[Xbackq]] */
/*
 * Who should wait for the exit from the fork?
 */
void
Xbackq(void)
{
    int n, pid;
    int pfd[2];
    char *stop;
    char utf[UTFmax+1];
    struct Io *f;
    var *ifs = vlook("ifs");
    word *v, *nextv;
    Rune r;
    //String *word;

    stop = ifs->val? ifs->val->word: "";
    if(pipe(pfd)<0){
        Xerror("can't make pipe");
        return;
    }
    switch(pid = fork()){
    case -1:
        Xerror("try again");
        close(pfd[PRD]);
        close(pfd[PWR]);
        return;
    case 0: // child
        clearwaitpids();
        close(pfd[PRD]);
        start(runq->code, runq->pc+1, runq->local);
        pushredir(ROPEN, pfd[PWR], 1);
        return;
    default: // parent
        Exit("TODO", __LOC__);
        //pad: commented because use the string.h s_xxx library
        // that I didn't want to port to goken
        //addwaitpid(pid);
        //close(pfd[PWR]);
        //f = openfd(pfd[PRD]);
        //word = s_new();
        //v = nil;
        ///* rutf requires at least UTFmax+1 bytes in utf */
        //while((n = rutf(f, utf, &r)) != EOF){
        //    utf[n] = '\0';
        //    if(utfutf(stop, utf) == nil)
        //        s_nappend(word, utf, n);
        //    else
        //        /*
        //         * utf/r is an ifs rune (e.g., \t, \n), thus
        //         * ends the current word, if any.
        //         */
        //        if(s_len(word) > 0){
        //            v = newword(s_to_c(word), v);
        //            s_reset(word);
        //        }
        //}
        //if(s_len(word) > 0)
        //    v = newword(s_to_c(word), v);
        //s_free(word);
        //closeio(f);
        //Waitfor(pid, false);
        ///* v points to reversed arglist -- reverse it onto argv */
        //while(v){
        //    nextv = v->next;
        //    v->next = runq->argv->words;
        //    runq->argv->words = v;
        //    v = nextv;
        //}
        //runq->pc = runq->code[runq->pc].i;
        //return;
    }
}
/*e: function [[Xbackq]] */

/*s: function [[Xpipefd]] */
void
Xpipefd(void)
{
    struct Thread *p = runq;
    int pc = p->pc, pid;
    char name[40];
    int pfd[2];
    int sidefd, mainfd;
    if(pipe(pfd)<0){
        Xerror("can't get pipe");
        return;
    }
    if(p->code[pc].i==READ){
        sidefd = pfd[PWR];
        mainfd = pfd[PRD];
    }
    else{
        sidefd = pfd[PRD];
        mainfd = pfd[PWR];
    }
    switch(pid = fork()){
    case -1:
        Xerror("try again");
        break;
    case 0: // child
        clearwaitpids();
        start(p->code, pc+2, runq->local);
        close(mainfd);
        pushredir(ROPEN, sidefd, p->code[pc].i==READ?1:0);
        runq->ret = 0;
        break;
    default: // parent
        addwaitpid(pid);
        close(sidefd);
        pushredir(ROPEN, mainfd, mainfd);	/* isn't this a noop? */
        strcpy(name, Fdprefix);
        inttoascii(name+strlen(name), mainfd);
        pushword(name);
        p->pc = p->code[pc+1].i;
        break;
    }
}
/*e: function [[Xpipefd]] */

/*s: function [[Xsubshell]] */
void
Xsubshell(void)
{
    int pid;
    switch(pid = fork()){
    case -1:
        Xerror("try again");
        break;
    case 0: // child
        clearwaitpids();
        start(runq->code, runq->pc+1, runq->local);
        runq->ret = nil;
        break;
    default: // parent
        addwaitpid(pid);
        Waitfor(pid, true);
        runq->pc = runq->code[runq->pc].i;
        break;
    }
}
/*e: function [[Xsubshell]] */

/*s: function [[execforkexec]] */
int
execforkexec(void)
{
    int pid;
    int n;
    char buf[ERRMAX];

    // fork()!!
    switch(pid = fork()){
    case -1:
        return -1;
    case 0: // child
        clearwaitpids();
        pushword("exec");
        execexec();

        // should not be reached! unless the command did not exist
        strcpy(buf, "can't exec: ");
        n = strlen(buf);
        errstr(buf+n, ERRMAX-n);
        Exit(buf, __LOC__);
    }
    // parent
    addwaitpid(pid);
    return pid;
}
/*e: function [[execforkexec]] */
/*e: rc/processes.c */
