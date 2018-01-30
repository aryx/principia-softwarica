/*s: rc/exec.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

/*
 * Opcode routines
 * Arguments on stack (...)
 * Arguments in line [...]
 * Code in line with jump around {...}
 *
 * Xappend(file)[fd]			open file to append
 * Xassign(name, val)			assign val to name
 * Xasync{... Xexit}			make thread for {}, no wait
 * Xbackq{... Xreturn}			make thread for {}, push stdout
 * Xbang				complement condition
 * Xcase(pat, value){...}		exec code on match, leave (value) on
 * 					stack
 * Xclose[i]				close file descriptor
 * Xconc(left, right)			concatenate, push results
 * Xcount(name)				push var count
 * Xdelfn(name)				delete function definition
 * Xdelhere
 * Xdol(name)				get variable value
 * Xdup[i j]				dup file descriptor
 * Xeflag
 * Xerror
 * Xexit				rc exits with status
 * Xfalse{...}				execute {} if false
 * Xfn(name){... Xreturn}			define function
 * Xfor(var, list){... Xreturn}		for loop
 * Xglob
 * Xif
 * Xifnot
 * Xjump[addr]				goto
 * Xlocal(name, val)			create local variable, assign value
 * Xmark				mark stack
 * Xmatch(pat, str)			match pattern, set status
 * Xpipe[i j]{... Xreturn}{... Xreturn}	construct a pipe between 2 new threads,
 * 					wait for both
 * Xpipefd[type]{... Xreturn}		connect {} to pipe (input or output,
 * 					depending on type), push /dev/fd/??
 * Xpipewait
 * Xpopm(value)				pop value from stack
 * Xpopredir
 * Xrdcmds
 * Xrdfn
 * Xrdwr(file)[fd]			open file for reading and writing
 * Xread(file)[fd]			open file to read
 * Xqdol(name)				concatenate variable components
 * Xreturn				kill thread
 * Xsimple(args)			run command and wait
 * Xsub
 * Xsubshell{... Xexit}			execute {} in a subshell and wait
 * Xtrue{...}				execute {} if true
 * Xunlocal				delete local variable
 * Xwastrue
 * Xword[string]			push string
 * Xwrite(file)[fd]			open file to write
 */

/*s: function [[Xappend]] */
void
Xappend(void)
{
    char *file;
    int f;
    switch(count(runq->argv->words)){
    default:
        Xerror1(">> requires singleton");
        return;
    case 0:
        Xerror1(">> requires file");
        return;
    case 1:
        break;
    }
    file = runq->argv->words->word;
    if((f = open(file, 1))<0 && (f = Creat(file))<0){
        pfmt(err, "%s: ", file);
        Xerror("can't open");
        return;
    }
    Seek(f, 0L, 2);
    pushredir(ROPEN, f, runq->code[runq->pc].i);
    runq->pc++;
    poplist();
}
/*e: function [[Xappend]] */

/*s: function [[Xsettrue]] */
void
Xsettrue(void)
{
    setstatus("");
}
/*e: function [[Xsettrue]] */

/*s: function [[Xbang]] */
void
Xbang(void)
{
    setstatus(truestatus()? "false" : "");
}
/*e: function [[Xbang]] */

/*s: function [[Xclose]] */
void
Xclose(void)
{
    pushredir(RCLOSE, runq->code[runq->pc].i, 0);
    runq->pc++;
}
/*e: function [[Xclose]] */

/*s: function [[Xdup]] */
void
Xdup(void)
{
    pushredir(RDUP, runq->code[runq->pc].i, runq->code[runq->pc+1].i);
    runq->pc+=2;
}
/*e: function [[Xdup]] */

/*s: function [[Xeflag]] */
void
Xeflag(void)
{
    if(eflagok && !truestatus()) 
        Xexit();
}
/*e: function [[Xeflag]] */

/*s: function [[Xexit]] */
void
Xexit(void)
{
    struct Var *trapreq;
    struct Word *starval;
    static bool beenhere = false;

    if(getpid()==mypid && !beenhere){
        trapreq = vlook("sigexit");
        if(trapreq->fn){
            beenhere = true;
            --runq->pc;
            starval = vlook("*")->val;
            start(trapreq->fn, trapreq->pc, (struct Var *)0);
            runq->local = newvar(strdup("*"), runq->local);
            runq->local->val = copywords(starval, (struct Word *)0);
            runq->local->changed = true;
            runq->redir = runq->startredir = nil;
            return;
        }
    }
    Exit(getstatus());
}
/*e: function [[Xexit]] */

/*s: function [[Xfalse]] */
void
Xfalse(void)
{
    if(truestatus()) runq->pc = runq->code[runq->pc].i;
    else runq->pc++;
}
/*e: function [[Xfalse]] */
/*s: global [[ifnot]] */
bool ifnot;		/* dynamic if not flag */
/*e: global [[ifnot]] */

/*s: function [[Xifnot]] */
void
Xifnot(void)
{
    if(ifnot)
        runq->pc++;
    else
        runq->pc = runq->code[runq->pc].i;
}
/*e: function [[Xifnot]] */

/*s: function [[Xjump]] */
void
Xjump(void)
{
    runq->pc = runq->code[runq->pc].i;
}
/*e: function [[Xjump]] */

/*s: function [[Xmark]] */
void
Xmark(void)
{
    pushlist();
}
/*e: function [[Xmark]] */

/*s: function [[Xpopm]] */
void
Xpopm(void)
{
    poplist();
}
/*e: function [[Xpopm]] */

/*s: function [[Xread]] */
void
Xread(void)
{
    char *file;
    int f;
    switch(count(runq->argv->words)){
    default:
        Xerror1("< requires singleton\n");
        return;
    case 0:
        Xerror1("< requires file\n");
        return;
    case 1:
        break;
    }
    file = runq->argv->words->word;
    if((f = open(file, 0))<0){
        pfmt(err, "%s: ", file);
        Xerror("can't open");
        return;
    }
    pushredir(ROPEN, f, runq->code[runq->pc].i);
    runq->pc++;
    poplist();
}
/*e: function [[Xread]] */

/*s: function [[Xrdwr]] */
void
Xrdwr(void)
{
    char *file;
    int f;

    switch(count(runq->argv->words)){
    default:
        Xerror1("<> requires singleton\n");
        return;
    case 0:
        Xerror1("<> requires file\n");
        return;
    case 1:
        break;
    }
    file = runq->argv->words->word;
    if((f = open(file, ORDWR))<0){
        pfmt(err, "%s: ", file);
        Xerror("can't open");
        return;
    }
    pushredir(ROPEN, f, runq->code[runq->pc].i);
    runq->pc++;
    poplist();
}
/*e: function [[Xrdwr]] */

/*s: function [[Xtrue]] */
void
Xtrue(void)
{
    if(truestatus()) runq->pc++;
    else runq->pc = runq->code[runq->pc].i;
}
/*e: function [[Xtrue]] */

/*s: function [[Xif]] */
void
Xif(void)
{
    ifnot = true;
    if(truestatus()) 
        runq->pc++;
    else 
        runq->pc = runq->code[runq->pc].i;
}
/*e: function [[Xif]] */

/*s: function [[Xwastrue]] */
void
Xwastrue(void)
{
    ifnot = false;
}
/*e: function [[Xwastrue]] */

/*s: function [[Xword]] */
void
Xword(void)
{
    pushword(runq->code[runq->pc++].s);
}
/*e: function [[Xword]] */

/*s: function [[Xwrite]] */
void
Xwrite(void)
{
    char *file;
    fdt f;
    switch(count(runq->argv->words)){
    default:
        Xerror1("> requires singleton\n");
        return;
    case 0:
        Xerror1("> requires file\n");
        return;
    case 1:
        break;
    }
    file = runq->argv->words->word;
    if((f = Creat(file))<0){
        pfmt(err, "%s: ", file);
        Xerror("can't open");
        return;
    }
    pushredir(ROPEN, f, runq->code[runq->pc].i);
    runq->pc++;
    poplist();
}
/*e: function [[Xwrite]] */

/*s: function [[list2str]] */
char*
list2str(word *words)
{
    char *value, *s, *t;
    int len = 0;
    word *ap;

    for(ap = words;ap;ap = ap->next)
        len += 1+strlen(ap->word);
    value = emalloc(len+1);

    s = value;
    for(ap = words;ap;ap = ap->next){
        for(t = ap->word;*t;) 
            *s++=*t++;
        *s++=' ';
    }
    if(s==value)
        *s='\0';
    else s[-1]='\0';
    return value;
}
/*e: function [[list2str]] */

/*s: function [[Xmatch]] */
void
Xmatch(void)
{
    word *p;
    char *subject;

    subject = list2str(runq->argv->words);
    setstatus("no match");
    for(p = runq->argv->next->words;p;p = p->next)
        if(match(subject, p->word, '\0')){
            setstatus("");
            break;
        }
    efree(subject);
    poplist();
    poplist();
}
/*e: function [[Xmatch]] */

/*s: function [[Xcase]] */
void
Xcase(void)
{
    word *p;
    char *s;
    bool ok = false;

    s = list2str(runq->argv->next->words);
    for(p = runq->argv->words;p;p = p->next){
        if(match(s, p->word, '\0')){
            ok = true;
            break;
        }
    }
    efree(s);
    if(ok)
        runq->pc++;
    else
        runq->pc = runq->code[runq->pc].i;
    poplist();
}
/*e: function [[Xcase]] */

/*s: function [[conclist]] */
word*
conclist(word *lp, word *rp, word *tail)
{
    char *buf;
    word *v;
    if(lp->next || rp->next)
        tail = conclist(lp->next==0? lp: lp->next,
            rp->next==0? rp: rp->next, tail);
    buf = emalloc(strlen(lp->word)+strlen((char *)rp->word)+1);
    strcpy(buf, lp->word);
    strcat(buf, rp->word);
    v = newword(buf, tail);
    efree(buf);
    return v;
}
/*e: function [[conclist]] */

/*s: function [[Xconc]] */
void
Xconc(void)
{
    word *lp = runq->argv->words;
    word *rp = runq->argv->next->words;
    word *vp = runq->argv->next->next->words;
    int lc = count(lp), rc = count(rp);
    if(lc!=0 || rc!=0){
        if(lc==0 || rc==0){
            Xerror1("null list in concatenation");
            return;
        }
        if(lc!=1 && rc!=1 && lc!=rc){
            Xerror1("mismatched list lengths in concatenation");
            return;
        }
        vp = conclist(lp, rp, vp);
    }
    poplist();
    poplist();
    runq->argv->words = vp;
}
/*e: function [[Xconc]] */

/*s: function [[Xassign]] */
void
Xassign(void)
{
    var *v;
    if(count(runq->argv->words)!=1){
        Xerror1("variable name not singleton!");
        return;
    }
    deglob(runq->argv->words->word); // remove the special \001 mark
    v = vlook(runq->argv->words->word);
    poplist();

    globlist();
    freewords(v->val);
    v->val = runq->argv->words;
    v->changed = true;
    runq->argv->words = nil;
    poplist();
}
/*e: function [[Xassign]] */

/*s: function [[Xdol]] */
void
Xdol(void)
{
    word *a, *star;
    char *s, *t;
    int n;

    if(count(runq->argv->words)!=1){
        Xerror1("variable name not singleton!");
        return;
    }
    s = runq->argv->words->word;
    deglob(s);

    n = 0;
    for(t = s;'0'<=*t && *t<='9';t++) 
        n = n*10+*t-'0';

    a = runq->argv->next->words;
    if(n==0 || *t)
        a = copywords(vlook(s)->val, a);
    else{
        star = vlook("*")->val;
        if(star && 1 <= n && n <= count(star)){
            while(--n) 
                star = star->next;
            a = newword(star->word, a);
        }
    }
    poplist();
    runq->argv->words = a;
}
/*e: function [[Xdol]] */

/*s: function [[Xqdol]] */
void
Xqdol(void)
{
    word *a, *p;
    char *s;
    int n;
    if(count(runq->argv->words)!=1){
        Xerror1("variable name not singleton!");
        return;
    }
    s = runq->argv->words->word;
    deglob(s);
    a = vlook(s)->val;
    poplist();
    n = count(a);
    if(n==0){
        pushword("");
        return;
    }
    for(p = a;p;p = p->next) n+=strlen(p->word);
    s = emalloc(n);
    if(a){
        strcpy(s, a->word);
        for(p = a->next;p;p = p->next){
            strcat(s, " ");
            strcat(s, p->word);
        }
    }
    else
        s[0]='\0';
    pushword(s);
    efree(s);
}
/*e: function [[Xqdol]] */

/*s: function [[subwords]] */
word*
subwords(word *val, int len, word *sub, word *a)
{
    int n, m;
    char *s;
    if(!sub)
        return a;
    a = subwords(val, len, sub->next, a);
    s = sub->word;
    deglob(s);
    m = 0;
    n = 0;
    while('0'<=*s && *s<='9')
        n = n*10+ *s++ -'0';
    if(*s == '-'){
        if(*++s == 0)
            m = len - n;
        else{
            while('0'<=*s && *s<='9')
                m = m*10+ *s++ -'0';
            m -= n;
        }
    }
    if(n<1 || n>len || m<0)
        return a;
    if(n+m>len)
        m = len-n;
    while(--n > 0)
        val = val->next;
    return copynwords(val, a, m+1);
}
/*e: function [[subwords]] */

/*s: function [[Xsub]] */
void
Xsub(void)
{
    word *a, *v;
    char *s;
    if(count(runq->argv->next->words)!=1){
        Xerror1("variable name not singleton!");
        return;
    }
    s = runq->argv->next->words->word;
    deglob(s);
    a = runq->argv->next->next->words;
    v = vlook(s)->val;
    a = subwords(v, count(v), runq->argv->words, a);
    poplist();
    poplist();
    runq->argv->words = a;
}
/*e: function [[Xsub]] */

/*s: function [[Xcount]] */
void
Xcount(void)
{
    word *a;
    char *s, *t;
    int n;
    char num[12];

    if(count(runq->argv->words)!=1){
        Xerror1("variable name not singleton!");
        return;
    }
    s = runq->argv->words->word;
    deglob(s);

    // n = int_of_string(s)
    n = 0;
    for(t = s;'0'<=*t && *t<='9';t++) 
        n = n*10 + *t - '0';
    if(n==0 || *t){
        a = vlook(s)->val;
        inttoascii(num, count(a));
    }
    else{
        a = vlook("*")->val;
        inttoascii(num, (a && 1<=n && n<=count(a)) ? 1 : 0);
    }
    poplist();
    pushword(num);
}
/*e: function [[Xcount]] */

/*s: function [[Xlocal]] */
void
Xlocal(void)
{
    if(count(runq->argv->words)!=1){
        Xerror1("variable name must be singleton\n");
        return;
    }
    deglob(runq->argv->words->word);
    runq->local = newvar(strdup(runq->argv->words->word), runq->local);
    poplist();
    globlist();
    runq->local->val = runq->argv->words;
    runq->local->changed = true;
    // change ownership
    runq->argv->words = nil;
    poplist();
}
/*e: function [[Xlocal]] */

/*s: function [[Xunlocal]] */
void
Xunlocal(void)
{
    var *v = runq->local, *hid;
    if(v==0)
        panic("Xunlocal: no locals!", 0);
    runq->local = v->next;
    hid = vlook(v->name);
    hid->changed = true;
    efree(v->name);
    freewords(v->val);
    efree((char *)v);
}
/*e: function [[Xunlocal]] */

/*s: function [[Xfn]] */
void
Xfn(void)
{
    var *v;
    word *a;
    int end;

    end = runq->code[runq->pc].i;
    globlist();
    for(a = runq->argv->words;a;a = a->next){
        v = gvlook(a->word);
        if(v->fn)
            codefree(v->fn);
        v->fn = codecopy(runq->code);
        v->pc = runq->pc+2;
        v->fnchanged = true;
    }
    runq->pc = end;
    poplist();
}
/*e: function [[Xfn]] */

/*s: function [[Xdelfn]] */
void
Xdelfn(void)
{
    var *v;
    word *a;

    for(a = runq->argv->words;a;a = a->next){
        v = gvlook(a->word);
        if(v->fn)
            codefree(v->fn);
        v->fn = nil;
        v->fnchanged = true;
    }
    poplist();
}
/*e: function [[Xdelfn]] */

/*s: function [[Xpipewait]] */
void
Xpipewait(void)
{
    char status[NSTATUS+1];
    if(runq->pid==-1)
        setstatus(concstatus(runq->status, getstatus()));
    else{
        strncpy(status, getstatus(), NSTATUS);
        status[NSTATUS]='\0';
        Waitfor(runq->pid, 1);
        runq->pid=-1;
        setstatus(concstatus(getstatus(), status));
    }
}
/*e: function [[Xpipewait]] */

/*s: function [[Xrdcmds]] */
void
Xrdcmds(void)
{
    struct Thread *p = runq;
    bool error;
    /*s: [[Xrdcmds()]] other locals */
    word *prompt;
    /*e: [[Xrdcmds()]] other locals */

    /*s: [[Xrdcmds()]] flush errors and reset error count */
    flush(err);
    nerror = 0;
    /*e: [[Xrdcmds()]] flush errors and reset error count */
    /*s: [[Xrdcmds()]] print status if -s */
    if(flag['s'] && !truestatus())
        pfmt(err, "status=%v\n", vlook("status")->val);
    /*e: [[Xrdcmds()]] print status if -s */
    /*s: [[Xrdcmds()]] set promptstr if interactive mode */
    if(runq->iflag){
        prompt = vlook("prompt")->val;
        if(prompt)
            promptstr = prompt->word;
        else
            promptstr="% ";
    }
    /*e: [[Xrdcmds()]] set promptstr if interactive mode */

    /*s: [[Xrdcmds()]] calls Noerror() before yyparse() */
    Noerror();
    /*e: [[Xrdcmds()]] calls Noerror() before yyparse() */
    // read one cmd line, compiles it, and modifies codebuf global
    error = yyparse();

    /*s: [[Xrdcmds()]] if yyparse() returned an error */
    if(error){
        if(!p->iflag  ||  p->eof && !Eintr()){
            if(p->cmdfile)
                efree(p->cmdfile);
            closeio(p->cmdfd);

            Xreturn();	/* should this be omitted? */
        }else{
            if(Eintr()){
                pchr(err, '\n');
                p->eof = false;
            }
            --p->pc;	/* go back for next command */
        }
    }
    /*e: [[Xrdcmds()]] if yyparse() returned an error */
    else{
        /*s: [[Xrdcmds()]] reset ntrap */
        ntrap = 0;	/* avoid double-interrupts during blocked writes */
        /*e: [[Xrdcmds()]] reset ntrap */
        --p->pc;	/* re-execute Xrdcmds after codebuf runs */
        // modifies runq, new thread (linked to bootstrap one)
        start(codebuf, 1, runq->local);
    }
    freenodes(); // allocated in yyparse()
}
/*e: function [[Xrdcmds]] */

/*s: function [[Xdelhere]] */
void
Xdelhere(void)
{
    Unlink(runq->code[runq->pc++].s);
}
/*e: function [[Xdelhere]] */

/*s: function [[Xfor]] */
void
Xfor(void)
{
    if(runq->argv->words==0){
        poplist();
        runq->pc = runq->code[runq->pc].i;
    }
    else{
        freelist(runq->local->val);
        runq->local->val = runq->argv->words;
        runq->local->changed = true;
        runq->argv->words = runq->argv->words->next;
        runq->local->val->next = 0;
        runq->pc++;
    }
}
/*e: function [[Xfor]] */

/*s: function [[Xglob]] */
void
Xglob(void)
{
    globlist();
}
/*e: function [[Xglob]] */

/*s: global [[envdir]] */
fdt envdir;
/*e: global [[envdir]] */

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

/*e: rc/exec.c */
