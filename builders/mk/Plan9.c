/*s: mk/Plan9.c */
#include	"mk.h"

// could be in utils.c
/*s: function [[maketmp]] */
char*
maketmp(void)
{
    static char temp[] = "/tmp/mkargXXXXXX";

    mktemp(temp);
    return temp;
}
/*e: function [[maketmp]] */

// could be in env.c
/*s: function [[encodenulls]] */
/* break string of values into words at 01's or nulls*/
static Word *
encodenulls(char *s, int n)
{
    Word *head, *lastw;
    char *cp;

    head = lastw = nil;
    while (n-- > 0) {
        for (cp = s; *cp && *cp != '\0'; cp++)
                n--;
        *cp = '\0';

        // add_list(newword(s), head)
        if (lastw) {
            lastw->next = newword(s);
            lastw = lastw->next;
        } else
            head = lastw = newword(s);

        s = cp+1;
    }
    if (!head)
        head = newword("");
    return head;
}
/*e: function [[encodenulls]] */
/*s: function [[readenv]] */
void
readenv(void)
{
    fdt envdir;
    fdt envfile;
    Dir *e;
    int i, n, len, len2;
    char *p;
    char name[1024];
    Word *w;

    rfork(RFENVG);	/*  use copy of the current environment variables */

    envdir = open("/env", OREAD);
    /*s: [[readenv()]] sanity check envdir */
    if(envdir < 0)
        return;
    /*e: [[readenv()]] sanity check envdir */
    while((n = dirread(envdir, &e)) > 0){
        for(i = 0; i < n; i++){
            len = e[i].length;
            /*s: [[readenv()]] skip some names */
            /* don't import funny names, NULL values,
             * or internal mk variables
             */
            if(len <= 0 
               || *shname(e[i].name) != '\0' 
               || symlook(e[i].name, S_INTERNAL, nil))
                continue;
            /*e: [[readenv()]] skip some names */

            snprint(name, sizeof name, "/env/%s", e[i].name);
            envfile = open(name, OREAD);
            /*s: [[readenv()]] sanity check envfile */
            if(envfile < 0)
                continue;
            /*e: [[readenv()]] sanity check envfile */
            p = Malloc(len+1);
            len2 = read(envfile, p, len);
            /*s: [[readenv()]] sanity check len2 */
            if(len2 != len){
                perror(name);
                close(envfile);
                continue;
            }
            /*e: [[readenv()]] sanity check len2 */
            close(envfile);
            /*s: [[readenv()]] add null terminator character at end of [[p]] */
            if (p[len-1] == '\0')
                len--;
            else
                p[len] = '\0';
            /*e: [[readenv()]] add null terminator character at end of [[p]] */
            w = encodenulls(p, len);
            free(p);
            p = strdup(e[i].name);

            // populating symbol table
            setvar(p, (void *) w);
        }
        free(e);
    }
    close(envdir);
}
/*e: function [[readenv]] */
/*s: function [[exportenv]] */
/* as well as 01's, change blanks to nulls, so that rc will
 * treat the words as separate arguments
 */
void
exportenv(ShellEnvVar *e)
{
    int n;
    fdt f;
    bool first;
    Word *w;
    char name[256];
    /*s: [[exportenv()]] other locals */
    Symtab *sym;
    bool hasvalue;
    /*e: [[exportenv()]] other locals */

    for(;e->name; e++){
        /*s: [[exportenv()]] skip entry if not a user variable and no value */
        hasvalue = !empty_words(e->values);
        sym = symlook(e->name, S_VAR, nil);
        if(sym == nil && !hasvalue)	/* non-existant null symbol */
            continue;
        /*e: [[exportenv()]] skip entry if not a user variable and no value */
        // else
        snprint(name, sizeof name, "/env/%s", e->name);
        /*s: [[exportenv()]] if existing symbol but no value, remove from env */
        if (sym != nil && !hasvalue) {	/* Remove from environment */
            /* we could remove it from the symbol table
             * too, but we're in the child copy, and it
             * would still remain in the parent's table.
             */
            remove(name);
            freewords(e->values);
            e->values = nil;		/* memory leak */
            continue;
        }
        /*e: [[exportenv()]] if existing symbol but no value, remove from env */
        // else
        f = create(name, OWRITE, 0666L);
        /*s: [[exportenv()]] sanity check f */
        if(f < 0) {
            fprint(STDERR, "can't create %s, f=%d\n", name, f);
            perror(name);
            continue;
        }
        /*e: [[exportenv()]] sanity check f */
        first = true;
        for (w = e->values; w; w = w->next) {
            n = strlen(w->s);
            if (n) {
                /*s: [[exportenv()]] write null separator */
                if(first)
                    first = false;
                else{
                    if (write (f, "\000", 1) != 1)
                        perror(name);
                }
                /*e: [[exportenv()]] write null separator */
                if (write(f, w->s, n) != n)
                    perror(name);
            }
        }
        close(f);
    }
}
/*e: function [[exportenv]] */

// could be in file.c
/*s: function [[chgtime]] */
int
chgtime(char *name)
{
    Dir sbuf;

    if(access(name, AEXIST) >= 0) {
        nulldir(&sbuf);
        sbuf.mtime = time((long *)nil);
        return dirwstat(name, &sbuf);
    }
    return close(create(name, OWRITE, 0666));
}
/*e: function [[chgtime]] */
/*s: function [[dirtime]] */
void
dirtime(char *dir, char *path)
{
    int i, fd, n;
    ulong mtime;
    Dir *d;
    char buf[4096];

    fd = open(dir, OREAD);
    if(fd >= 0){
        while((n = dirread(fd, &d)) > 0){
            for(i=0; i<n; i++){
                mtime = d[i].mtime;
                /* defensive driving: this does happen */
                if(mtime == 0)
                    mtime = 1;
                snprint(buf, sizeof buf, "%s%s", path,
                    d[i].name);
                if(symlook(buf, S_TIME, 0) == nil)
                    symlook(strdup(buf), S_TIME,
                        (void*)mtime)->u.value = mtime;
            }
            free(d);
        }
        close(fd);
    }
}
/*e: function [[dirtime]] */
/*s: function [[bulkmtime]] */
void
bulkmtime(char *dir)
{
    char buf[4096];
    char *ss, *s, *sym;

    if(dir){
        sym = dir;
        s = dir;
        if(strcmp(dir, "/") == 0)
            strecpy(buf, buf + sizeof buf - 1, dir);
        else
            snprint(buf, sizeof buf, "%s/", dir);
    }else{
        s = ".";
        sym = "";
        buf[0] = 0;
    }
    if(symlook(sym, S_BULKED, 0))
        return;
    // else
    ss = strdup(sym);
    symlook(ss, S_BULKED, (void*)ss);
    dirtime(s, buf);
}
/*e: function [[bulkmtime]] */
/*s: function [[mkmtime]] */
ulong
mkmtime(char *name, bool force)
{
    Dir *d;
    ulong t;
    /*s: [[mkmtime]] locals */
    //char *s, *ss;
    //char carry;
    //Symtab *sym;
    /*x: [[mkmtime]] locals */
    char buf[4096];
    /*e: [[mkmtime]] locals */

    /*s: [[mkmtime()]] bulk dir optimisation */
    /*s: [[mkmtime()]] cleanup name */
    strecpy(buf, buf + sizeof buf - 1, name);
    cleanname(buf);
    name = buf;
    /*e: [[mkmtime()]] cleanup name */
    USED(force);
    //TODO    s = utfrrune(name, '/');
    //TODO    if(s == name)
    //TODO        s++;
    //TODO    if(s){
    //TODO        ss = name;
    //TODO        carry = *s;
    //TODO        *s = '\0';
    //TODO    }else{
    //TODO        ss = nil;
    //TODO        carry = '\0';
    //TODO    }
    //TODO    if(carry)
    //TODO        *s = carry;
    //TODO
    //TODO bulkmtime(ss);
    //TODO if(!force){
    //TODO     sym = symlook(name, S_TIME, 0);
    //TODO     if(sym)
    //TODO         return sym->u.value;
    //TODO     return 0;
    //TODO }
    /*e: [[mkmtime()]] bulk dir optimisation */
    d = dirstat(name);
    /*s: [[mkmtime()]] check if inexistent file */
    if(d == nil)
        return 0;
    /*e: [[mkmtime()]] check if inexistent file */
    t = d->mtime;
    free(d);

    return t;
}
/*e: function [[mkmtime]] */

// could be in run.c
/*s: global [[shell]] */
char 	*shell =	"/bin/rc";
/*e: global [[shell]] */
/*s: global [[shellname]] */
char 	*shellname =	"rc";
/*e: global [[shellname]] */

// could be in run.c
/*s: function [[execsh]] */
int
execsh(char *shargs, char *shinput, Bufblock *buf, ShellEnvVar *e)
{
    int pid1, pid2;
    fdt in[2]; // pipe descriptors
    int err;
    /*s: [[execsh()]] other locals */
    char *endshinput;
    /*x: [[execsh()]] other locals */
    fdt out[2];
    /*x: [[execsh()]] other locals */
    int tot, n;
    /*e: [[execsh()]] other locals */

    /*s: [[execsh()]] if buf then create pipe to save output */
    if(buf && pipe(out) < 0){
        perror("pipe");
        Exit();
    }
    /*e: [[execsh()]] if buf then create pipe to save output */

    pid1 = rfork(RFPROC|RFFDG|RFENVG);
    /*s: [[execsh()]] sanity check pid rfork */
    if(pid1 < 0){
        perror("mk rfork");
        Exit();
    }
    /*e: [[execsh()]] sanity check pid rfork */
    // child
    if(pid1 == 0){
        /*s: [[execsh()]] in child, if buf, close one side of pipe */
        if(buf)
            close(out[0]);
        /*e: [[execsh()]] in child, if buf, close one side of pipe */
        err = pipe(in);
        /*s: [[execsh()]] sanity check err pipe */
        if(err < 0){
            perror("pipe");
            Exit();
        }
        /*e: [[execsh()]] sanity check err pipe */
        pid2 = fork();
        /*s: [[execsh()]] sanity check pid fork */
        if(pid2 < 0){
            perror("mk fork");
            Exit();
        }
        /*e: [[execsh()]] sanity check pid fork */
        // parent of grandchild, the shell interpreter
        if(pid2 != 0){
            // input must come from the pipe
            dup(in[0], STDIN);
            /*s: [[execsh()]] in child, if buf, dup and close */
            if(buf){
                // output now goes in the pipe
                dup(out[1], STDOUT);
                close(out[1]);
            }
            /*e: [[execsh()]] in child, if buf, dup and close */
            close(in[0]);
            close(in[1]);
            /*s: [[execsh()]] in child, export environment before exec */
            if (e)
                exportenv(e);
            /*e: [[execsh()]] in child, export environment before exec */
            if(shflags)
                execl(shell, shellname, shflags, shargs, nil);
            else
                execl(shell, shellname, shargs, nil);
            // should not be reached
            perror(shell);
            _exits("exec");
        }
        // else, grandchild, feeding the shell with recipe, through a pipe
        /*s: [[execsh()]] in grandchild, if buf, close other side of pipe */
        if(buf)
            close(out[1]);
        /*e: [[execsh()]] in grandchild, if buf, close other side of pipe */
        close(in[0]);
        // feed the shell
        /*s: [[execsh()]] in grandchild, write cmd in pipe */
        endshinput = shinput + strlen(shinput);
        while(shinput < endshinput){
            n = write(in[1], shinput, endshinput - shinput);
            if(n < 0)
                break;
            shinput += n;
        }
        /*e: [[execsh()]] in grandchild, write cmd in pipe */
        close(in[1]); // will flush
        _exits(nil);
    }
    // parent
    /*s: [[execsh()]] in parent, if buf, close other side of pipe and read output */
    if(buf){
        close(out[1]);
        tot = 0;
        for(;;){
            if (buf->current >= buf->end)
                growbuf(buf);
            n = read(out[0], buf->current, buf->end-buf->current);
            if(n <= 0)
                break;
            buf->current += n;
            tot += n;
        }
        if (tot && buf->current[-1] == '\n')
            buf->current--;
        close(out[0]);
    }
    /*e: [[execsh()]] in parent, if buf, close other side of pipe and read output */
    return pid1;
}
/*e: function [[execsh]] */
/*s: function [[waitfor]] */
int
xwaitfor(char *msg)
{
    Waitmsg *w;
    int pid;

    // blocking call, wait for any children
    w = wait();
    // no more children
    if(w == nil)
        return -1;
    strecpy(msg, msg+ERRMAX, w->msg);
    pid = w->pid;
    free(w);
    return pid;
}
/*e: function [[waitfor]] */

// could be in run.c
/*s: function [[Exit]] */
void
Exit(void)
{
    while(waitpid() >= 0)
        ;
    exits("error");
}
/*e: function [[Exit]] */

// back in run.c
extern void killchildren(char *msg);

/*s: function [[notifyf]] */
int
notifyf(void *a, char *msg)
{
    /*s: [[notifyf()]] sanity check not too many notes */
    static int nnote;

    USED(a);
    if(++nnote > 100){	/* until andrew fixes his program */
        fprint(STDERR, "mk: too many notes\n");
        notify(0);
        abort();
    }
    /*e: [[notifyf()]] sanity check not too many notes */
    if(strcmp(msg, "interrupt")!=0 && strcmp(msg, "hangup")!=0)
        return 0;
    killchildren(msg);
    return -1;
}
/*e: function [[notifyf]] */
/*s: function [[catchnotes]] */
void
catchnotes()
{
    atnotify(notifyf, 1);
}
/*e: function [[catchnotes]] */
/*s: function [[expunge]] */
void
expunge(int pid, char *msg)
{
    postnote(PNPROC, pid, msg);
}
/*e: function [[expunge]] */

// could be in run.c
/*s: function [[pipecmd]] */
int
pipecmd(char *cmd, ShellEnvVar *e, int *fd)
{
    int pid;
    fdt pfd[2];

    if(DEBUG(D_EXEC))
        fprint(STDOUT, "pipecmd='%s'\n", cmd);/**/

    if(fd && pipe(pfd) < 0){
        perror("pipe");
        Exit();
    }
    pid = rfork(RFPROC|RFFDG|RFENVG);
    if(pid < 0){
        perror("mk fork");
        Exit();
    }
    if(pid == 0){
        if(fd){
            close(pfd[0]);
            dup(pfd[1], 1);
            close(pfd[1]);
        }
        if(e)
            exportenv(e);
        if(shflags)
            execl(shell, shellname, shflags, "-c", cmd, nil);
        else
            execl(shell, shellname, "-c", cmd, nil);
        perror(shell);
        _exits("exec");
    }
    if(fd){
        close(pfd[1]);
        *fd = pfd[0];
    }
    return pid;
}
/*e: function [[pipecmd]] */

// could be in graph.c
/*s: function [[rcopy]] */
void
rcopy(char **to, Resub *match, int n)
{
    int c;
    char *p;

    *to = match->s.sp;		/* stem0 matches complete target */
    for(to++, match++; --n > 0; to++, match++){
        if(match->s.sp && match->e.ep){
            p = match->e.ep;
            c = *p;
            *p = 0;
            *to = strdup(match->s.sp);
            *p = c;
        }
        else
            *to = 0;
    }
}
/*e: function [[rcopy]] */

/*e: mk/Plan9.c */
