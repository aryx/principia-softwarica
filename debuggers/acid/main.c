/*s: acid/main.c */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>
#include "acid.h"
#include "y.tab.h"

extern int _ifmt(Fmt*);

/*s: global [[bioout]] */
static Biobuf	bioout;
/*e: global [[bioout]] */
/*s: global [[prog]] */
static char	prog[128];
/*e: global [[prog]] */
/*s: global [[lm]] */
// array<ref<string>> length = nlm
static char*	lm[16];
/*e: global [[lm]] */
/*s: global [[nlm]] */
static int	nlm;
/*e: global [[nlm]] */
/*s: global [[mtype]] */
static char*	mtype;
/*e: global [[mtype]] */

static  int attachfiles(char*, int);
int     xfmt(Fmt*);
int     isnumeric(char*);
void    die(void);
void    loadmoduleobjtype(void);

/*s: function usage (acid/main.c) */
void
usage(void)
{
    fprint(STDERR, "usage: acid [-kqw] [-l library] [-m machine] [pid] [file]\n");
    exits("usage");
}
/*e: function usage (acid/main.c) */

/*s: function [[main]] */
void
main(int argc, char *argv[])
{
    int pid = 0;
    /*s: [[main()]](acid) other locals */
    char *s;
    /*x: [[main()]](acid) other locals */
    int i;
    /*x: [[main()]](acid) other locals */
    Lsym *l;
    Node *n;
    /*e: [[main()]](acid) other locals */

    argv0 = argv[0];
    aout = "8.out";
    quiet = true;
    mtype = nil;

    ARGBEGIN{
    /*s: [[main()]](acid) command line processing */
    case 'q':
        quiet = false;
        break;
    /*x: [[main()]](acid) command line processing */
    case 'l':
        s = ARGF();
        if(s == nil)
            usage();
        lm[nlm++] = s;
        break;
    /*x: [[main()]](acid) command line processing */
    case 'w':
        wtflag = true;
        break;
    /*x: [[main()]](acid) command line processing */
    case 'k':
        kernel++;
        break;
    /*x: [[main()]](acid) command line processing */
    case 'r':
        pid = 1;
        remote++;
        kernel++;
        break;
    /*x: [[main()]](acid) command line processing */
    case 'm':
        mtype = ARGF();
        break;
    /*e: [[main()]](acid) command line processing */
    default:
        usage();
    }ARGEND

    if(argc > 0) {
        /*s: [[main()]](acid) if argc and remote adjust [[aout]] */
        if(remote)
            aout = argv[0];
        /*e: [[main()]](acid) if argc and remote adjust [[aout]] */
        else
        /*s: [[main()]](acid) if [[acid pid]] */
        if(isnumeric(argv[0])) {
            pid = strtol(argv[0], 0, 0);
            snprint(prog, sizeof(prog), "/proc/%d/text", pid);
            aout = prog;

            if(argc > 1)
                aout = argv[1];
            else
              /*s: [[main()]](acid) when [[acid pid]] if [[kernel]] */
              if(kernel)
                 aout = system();
              /*e: [[main()]](acid) when [[acid pid]] if [[kernel]] */
        }
        /*e: [[main()]](acid) if [[acid pid]] */
        else {
            /*s: [[main()]](acid) if kernel and no pid */
            if(kernel) {
                fprint(STDERR, "acid: -k requires a pid\n");
                usage();
            }
            /*e: [[main()]](acid) if kernel and no pid */
            aout = argv[0];
        }
    } else
       /*s: [[main()]](acid) if not argc and remote adjust [[aout]] */
       if(remote)
           aout = "/mips/9ch";
       /*e: [[main()]](acid) if not argc and remote adjust [[aout]] */

    /*s: [[main()]](acid) format initializations */
    fmtinstall('L', Lfmt);
    /*x: [[main()]](acid) format initializations */
    fmtinstall('x', xfmt);
    /*e: [[main()]](acid) format initializations */
    /*s: [[main()]](acid) initializations */
    initialising = 1;

    Binit(&bioout, STDOUT, OWRITE);
    bout = &bioout;

    // setup lexer
    kinit();
    // to read commands on stdin once all modules have been loaded
    pushfile(nil);

    loadvars();
    installbuiltin();

    /*s: [[main()]](acid) sanity check [[mtype]] */
    if(mtype && machbyname(mtype) == 0)
        print("unknown machine %s", mtype);
    /*e: [[main()]](acid) sanity check [[mtype]] */
    /*s: [[main()]](acid) [[attachfiles(aout, pid)]] */
    if (attachfiles(aout, pid) < 0)
        varreg();		/* use default register set on error */
    /*e: [[main()]](acid) [[attachfiles(aout, pid)]] */

    /*s: [[main()]](acid) load modules */
    loadmodule("/lib/acid/port.acid");
    loadmoduleobjtype();
    /*s: [[main()]](acid) load [[-l]] modules */
    for(i = 0; i < nlm; i++) {
        if(access(lm[i], AREAD) >= 0)
            loadmodule(lm[i]);
        else {
            s = smprint("/lib/acid/%s.acid", lm[i]);
            loadmodule(s);
            free(s);
        }
    }
    /*e: [[main()]](acid) load [[-l]] modules */
    /*e: [[main()]](acid) load modules */
    userinit();
    varsym();
    /*s: [[main()]](acid) check acidmap */
    l = look("acidmap");
    if(l && l->proc) {
        n = an(ONAME, ZN, ZN);
        n->sym = l;
        n = an(OCALL, n, ZN);
        execute(n);
    }
    /*e: [[main()]](acid) check acidmap */

    /*s: [[main()]](acid) notify setup */
    notify(catcher);
    /*e: [[main()]](acid) notify setup */

    initialising = 0;
    /*e: [[main()]](acid) initializations */

    /*s: [[main()]](acid) infinite loop */
    interactive = true;
    line = 1;

    for(;;) {
    
        /*s: [[main()]](acid) in infinite loop, reinit and unwind if error */
        if(setjmp(err)) {
            Binit(&bioout, STDOUT, OWRITE);
            unwind();
        }
        /*e: [[main()]](acid) in infinite loop, reinit and unwind if error */
        stacked = 0;

        Bprint(bout, "acid: ");

        // yyparse() will internally call execute() !
        if(yyparse() != OK_1)
            die();
        restartio();

        unwind();
    }
    /*e: [[main()]](acid) infinite loop */
    /* not reached */
}
/*e: function [[main]] */

/*s: function [[attachfiles]] */
/// main -> <>
static errorneg1
attachfiles(char *aout, int pid)
{
    interactive = false;
    /*s: [[attachfiles()]] if error */
    if(setjmp(err))
        return ERROR_NEG1;
    /*e: [[attachfiles()]] if error */

    if(aout) {				/* executable given */
        /*s: [[attachfiles()]] if [[wtflag]] */
        if(wtflag)
            text = open(aout, ORDWR);
        /*e: [[attachfiles()]] if [[wtflag]] */
        else
            text = open(aout, OREAD);

        /*s: [[attachfiles()]] sanity check [[text]] */
        if(text < 0)
            error("%s: can't open %s: %r\n", argv0, aout);
        /*e: [[attachfiles()]] sanity check [[text]] */
        readtext(aout);
    }
    /*s: [[attachfiles()]] if [[pid]] given */
    if(pid)					/* pid given */
        sproc(pid);
    /*e: [[attachfiles()]] if [[pid]] given */
    return OK_0;
}
/*e: function [[attachfiles]] */

/*s: function die (acid/main.c) */
/// main -> <>
void
die(void)
{
    Lsym *s;
    List *f;

    Bprint(bout, "\n");

    s = look("proclist");
    if(s && s->v->type == TLIST) {
        for(f = s->v->l; f; f = f->next)
            Bprint(bout, "echo kill > /proc/%d/ctl\n", (int)f->ival);
    }
    exits(nil);
}
/*e: function die (acid/main.c) */

/*s: function [[loadmoduleobjtype]] */
void
loadmoduleobjtype(void)
{
    char *buf;

    buf = smprint("/lib/acid/%s.acid", mach->name);
    loadmodule(buf);
    free(buf);
}
/*e: function [[loadmoduleobjtype]] */

/*s: function [[userinit]] */
/// main -> <>
void
userinit(void)
{
    Lsym *l;
    Node *n;
    char *buf, *p;

    /*s: [[userinit()]] if user acid file */
    p = getenv("home");
    if(p != nil) {
        buf = smprint("%s/lib/acid", p);
        silent = 1;
        loadmodule(buf);
        free(buf);
    }
    /*e: [[userinit()]] if user acid file */

    interactive = false;
    /*s: [[userinit()]] unwind if error */
    if(setjmp(err)) {
        unwind();
        return;
    }
    /*e: [[userinit()]] unwind if error */

    l = look("acidinit");
    if(l && l->proc) {
        n = an(ONAME, ZN, ZN);
        n->sym = l;
        n = an(OCALL, n, ZN);
        execute(n);
    }
}
/*e: function [[userinit]] */

/*s: function [[loadmodule]] */
void
loadmodule(char *s)
{
    interactive = false;
    /*s: [[loadmodule()]] unwind if error */
    if(setjmp(err)) {
        unwind();
        return;
    }
    /*e: [[loadmodule()]] unwind if error */
    pushfile(s);
    silent = 0;
    yyparse();
    popio();
    return;
}
/*e: function [[loadmodule]] */

/*s: function [[readtext]] */
/// main -> attachfiles -> <>
void
readtext(char *s)
{
    /*s: [[readtext()]] locals */
    Dir *d;
    uvlong length;
    /*x: [[readtext()]] locals */
    Lsym *l;
    Value *v;
    /*x: [[readtext()]] locals */
    Symbol sym;
    /*e: [[readtext()]] locals */

    /*s: [[readtext()]] if [[mtype != nil]] */
    if(mtype != nil){
        symmap = newmap(0, 1);
        if(symmap == 0)
            print("%s: (error) loadmap: cannot make symbol map\n", argv0);
        length = 1<<24;
        d = dirfstat(text);
        if(d != nil){
            length = d->length;
            free(d);
        }
        setmap(symmap, text, 0, length, 0, "binary");
        return;
    }
    /*e: [[readtext()]] if [[mtype != nil]] */
    // else

    if(!crackhdr(text, &fhdr)) {
        print("can't decode file header\n");
        return;
    }

    symmap = loadmap(nil, text, &fhdr);
    /*s: [[readtext()]] sanity check [[symmap]] */
    if(symmap == nil)
        print("%s: (error) loadmap: cannot make symbol map\n", argv0);
    /*e: [[readtext()]] sanity check [[symmap]] */

    if(syminit(text, &fhdr) < 0) {
        print("%s: (error) syminit: %r\n", argv0);
        return;
    }
    // first output! the executable and archi
    print("%s:%s\n", s, fhdr.name);

    /*s: [[readtext()]] if [[mach->sbreg]] */
    if(mach->sbreg && lookup(0, mach->sbreg, &sym)) {
        mach->sb = sym.value;
        l = enter("SB", Tid);
        l->v->fmt = 'X';
        l->v->ival = mach->sb;
        l->v->type = TINT;
        l->v->set = 1;
    }
    /*e: [[readtext()]] if [[mach->sbreg]] */
    /*s: [[readtext()]] make objtype variable */
    l = mkvar("objtype");
    v = l->v;
    v->fmt = 's';
    v->set = 1;
    v->string = strnode(mach->name);
    v->type = TSTRING;
    /*e: [[readtext()]] make objtype variable */
    /*s: [[readtext()]] make textfile variable */
    l = mkvar("textfile");
    v = l->v;
    v->fmt = 's';
    v->set = 1;
    v->string = strnode(s);
    v->type = TSTRING;
    /*e: [[readtext()]] make textfile variable */

    machbytype(fhdr.type);
    varreg();
}
/*e: function [[readtext]] */

/*s: function [[an]] */
Node*
an(int op, Node *l, Node *r)
{
    Node *n;

    n = gmalloc(sizeof(Node));
    memset(n, 0, sizeof(Node));

    /*s: [[an()]] add node [[n]] to [[gcl]] */
    n->gclink = gcl;
    gcl = n;
    /*e: [[an()]] add node [[n]] to [[gcl]] */

    n->op = op;
    n->left = l;
    n->right = r;

    return n;
}
/*e: function [[an]] */
/*s: function [[al]] */
List*
al(int t)
{
    List *l;

    l = gmalloc(sizeof(List));
    memset(l, 0, sizeof(List));
    l->type = t;

    /*s: [[al()]] add List [[l]] to [[gcl]] */
    l->gclink = gcl;
    gcl = l;
    /*e: [[al()]] add List [[l]] to [[gcl]] */

    return l;
}
/*e: function [[al]] */

/*s: function [[con]] */
Node*
con(vlong v)
{
    Node *n;

    n = an(OCONST, ZN, ZN);
    n->ival = v;
    n->fmt = 'W';
    n->type = TINT;
    return n;
}
/*e: function [[con]] */

/*s: function fatal (acid/main.c) */
/// -> <>
void
fatal(char *fmt, ...)
{
    char buf[128];
    va_list arg;

    va_start(arg, fmt);
    vseprint(buf, buf+sizeof(buf), fmt, arg);
    va_end(arg);
    fprint(STDERR, "%s: %L (fatal problem) %s\n", argv0, buf);
    exits(buf);
}
/*e: function fatal (acid/main.c) */

/*s: function [[yyerror]] */
void
yyerror(char *fmt, ...)
{
    char buf[128];
    va_list arg;

    if(strcmp(fmt, "syntax error") == 0) {
        yyerror("syntax error, near symbol '%s'", symbol);
        return;
    }
    va_start(arg, fmt);
    vseprint(buf, buf+sizeof(buf), fmt, arg);
    va_end(arg);
    print("%L: %s\n", buf);
}
/*e: function [[yyerror]] */

/*s: function [[marktree]] */
void
marktree(Node *n)
{

    if(n == nil)
        return;

    marktree(n->left);
    marktree(n->right);

    n->gcmark = 1;
    if(n->op != OCONST)
        return;

    switch(n->type) {
    case TSTRING:
        n->string->gcmark = 1;
        break;
    case TLIST:
        marklist(n->l);
        break;

    case TCODE:
        marktree(n->cc);
        break;
    }
}
/*e: function [[marktree]] */
/*s: function [[marklist]] */
void
marklist(List *l)
{
    while(l) {
        l->gcmark = 1;
        switch(l->type) {
        case TSTRING:
            l->string->gcmark = 1;
            break;
        case TLIST:
            marklist(l->l);
            break;
        case TCODE:
            marktree(l->cc);
            break;
        }
        l = l->next;
    }
}
/*e: function [[marklist]] */

/*s: function [[gc]] */
void
gc(void)
{
    int i;
    Lsym *f;
    Value *v;
    Gc *m, **p, *next;

    /*s: [[gc()]] return if still enough memory */
    if(dogc < Mempergc)
        return;
    dogc = 0;
    /*e: [[gc()]] return if still enough memory */

    /* Mark */
    for(m = gcl; m; m = m->gclink)
        m->gcmark = 0;

    /* Scan */
    for(i = 0; i < Hashsize; i++) {
        for(f = hash[i]; f; f = f->hash) {
            marktree(f->proc);
            if(f->lexval != Tid)
                continue;
            for(v = f->v; v; v = v->pop) {
                switch(v->type) {
                case TSTRING:
                    v->string->gcmark = 1;
                    break;
                case TLIST:
                    marklist(v->l);
                    break;

                case TCODE:
                    marktree(v->cc);
                    break;
                }
            }
        }
    }

    /* Free */
    p = &gcl;
    for(m = gcl; m; m = next) {
        next = m->gclink;
        if(m->gcmark == 0) {
            *p = next;
            free(m);	/* Sleazy reliance on my malloc */
        }
        else
            p = &m->gclink;
    }
}
/*e: function [[gc]] */

/*s: function [[gmalloc]] */
void*
gmalloc(long l)
{
    void *p;

    dogc += l;
    p = malloc(l);
    if(p == nil)
        fatal("out of memory");
    return p;
}
/*e: function [[gmalloc]] */

/*s: function [[checkqid]] */
void
checkqid(int f1, int pid)
{
    fdt fd;
    Dir *d1, *d2;
    char buf[128];

    /*s: [[checkqid() return if [[kernel]] */
    if(kernel)
        return;
    /*e: [[checkqid() return if [[kernel]] */

    d1 = dirfstat(f1);
    if(d1 == nil){
        print("checkqid: (qid not checked) dirfstat: %r\n");
        return;
    }

    snprint(buf, sizeof(buf), "/proc/%d/text", pid);
    fd = open(buf, OREAD);
    if(fd < 0 || (d2 = dirfstat(fd)) == nil){
        print("checkqid: (qid not checked) dirstat %s: %r\n", buf);
        free(d1);
        if(fd >= 0)
            close(fd);
        return;
    }

    close(fd);

    if(d1->qid.path != d2->qid.path || d1->qid.vers != d2->qid.vers || d1->qid.type != d2->qid.type){
        print("path %llux %llux vers %lud %lud type %d %d\n",
            d1->qid.path, d2->qid.path, d1->qid.vers, d2->qid.vers, d1->qid.type, d2->qid.type);
        print("warning: image does not match text for pid %d\n", pid);
    }
    free(d1);
    free(d2);
}
/*e: function [[checkqid]] */

/*s: function [[catcher]] */
void
catcher(void *junk, char *s)
{
    USED(junk);

    if(strstr(s, "interrupt")) {
        gotint = true;
        noted(NCONT);
    }
    noted(NDFLT);
}
/*e: function [[catcher]] */

/*s: function [[system]] */
char*
system(void)
{
    char *cpu, *p, *q;
    static char *kernel;

    cpu = getenv("cputype");
    if(cpu == 0) {
        cpu = "mips";
        print("$cputype not set; assuming %s\n", cpu);
    }
    p = getenv("terminal");
    if(p == 0 || (p=strchr(p, ' ')) == 0 || p[1] == ' ' || p[1] == 0) {
        p = "ch";
        print("missing or bad $terminal; assuming %s\n", p);
    }
    else{
        p++;
        q = strchr(p, ' ');
        if(q)
            *q = 0;
    }

    if(kernel != nil)
        free(kernel);
    kernel = smprint("/%s/9%s", cpu, p);

    return kernel;
}
/*e: function [[system]] */

/*s: function [[isnumeric]] */
bool
isnumeric(char *s)
{
    while(*s) {
        if(*s < '0' || *s > '9')
            return false;
        s++;
    }
    return true;
}
/*e: function [[isnumeric]] */

/*s: function [[xfmt]] */
int
xfmt(Fmt *f)
{
    f->flags ^= FmtSharp;
    return _ifmt(f);
}
/*e: function [[xfmt]] */
/*e: acid/main.c */
