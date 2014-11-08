/*s: mk/plan9.c */
#include	"mk.h"

/*s: global shell */
char 	*shell =	"/bin/rc";
/*e: global shell */
/*s: global shellname */
char 	*shellname =	"rc";
/*e: global shellname */

static	Word	*encodenulls(char*, int);

/*s: function readenv */
void
readenv(void)
{
    char *p;
    int envf, f;
    Dir *e;
    char nam[1024];
    int i, n, len;
    Word *w;

    rfork(RFENVG);	/*  use copy of the current environment variables */

    envf = open("/env", OREAD);
    if(envf < 0)
        return;
    while((n = dirread(envf, &e)) > 0){
        for(i = 0; i < n; i++){
            len = e[i].length;
                /* don't import funny names, NULL values,
                 * or internal mk variables
                 */
            if(len <= 0 || *shname(e[i].name) != '\0')
                continue;
            if (symlook(e[i].name, S_INTERNAL, 0))
                continue;
            snprint(nam, sizeof nam, "/env/%s", e[i].name);
            f = open(nam, OREAD);
            if(f < 0)
                continue;
            p = Malloc(len+1);
            if(read(f, p, len) != len){
                perror(nam);
                close(f);
                continue;
            }
            close(f);
            if (p[len-1] == 0)
                len--;
            else
                p[len] = 0;
            w = encodenulls(p, len);
            free(p);
            p = strdup(e[i].name);
            setvar(p, (void *) w);
            symlook(p, S_EXPORTED, (void*)"")->u.ptr = "";
        }
        free(e);
    }
    close(envf);
}
/*e: function readenv */

/*s: function encodenulls */
/* break string of values into words at 01's or nulls*/
static Word *
encodenulls(char *s, int n)
{
    Word *w, *head;
    char *cp;

    head = w = 0;
    while (n-- > 0) {
        for (cp = s; *cp && *cp != '\0'; cp++)
                n--;
        *cp = 0;
        if (w) {
            w->next = newword(s);
            w = w->next;
        } else
            head = w = newword(s);
        s = cp+1;
    }
    if (!head)
        head = newword("");
    return head;
}
/*e: function encodenulls */

/*s: function exportenv */
/* as well as 01's, change blanks to nulls, so that rc will
 * treat the words as separate arguments
 */
void
exportenv(Envy *e)
{
    int f, n, hasvalue, first;
    Word *w;
    Symtab *sy;
    char nam[256];

    for(;e->name; e++){
        sy = symlook(e->name, S_VAR, 0);
        if (e->values == 0 || e->values->s == 0 || e->values->s[0] == 0)
            hasvalue = 0;
        else
            hasvalue = 1;
        if(sy == 0 && !hasvalue)	/* non-existant null symbol */
            continue;
        snprint(nam, sizeof nam, "/env/%s", e->name);
        if (sy != 0 && !hasvalue) {	/* Remove from environment */
                /* we could remove it from the symbol table
                 * too, but we're in the child copy, and it
                 * would still remain in the parent's table.
                 */
            remove(nam);
            delword(e->values);
            e->values = 0;		/* memory leak */
            continue;
        }
    
        f = create(nam, OWRITE, 0666L);
        if(f < 0) {
            fprint(STDERR, "can't create %s, f=%d\n", nam, f);
            perror(nam);
            continue;
        }
        first = 1;
        for (w = e->values; w; w = w->next) {
            n = strlen(w->s);
            if (n) {
                if(first)
                    first = 0;
                else{
                    if (write (f, "\0", 1) != 1)
                        perror(nam);
                }
                if (write(f, w->s, n) != n)
                    perror(nam);
            }
        }
        close(f);
    }
}
/*e: function exportenv */

/*s: function waitfor */
int
waitfor(char *msg)
{
    Waitmsg *w;
    int pid;

    if((w=wait()) == nil)
        return -1;
    strecpy(msg, msg+ERRMAX, w->msg);
    pid = w->pid;
    free(w);
    return pid;
}
/*e: function waitfor */

/*s: function expunge */
void
expunge(int pid, char *msg)
{
    postnote(PNPROC, pid, msg);
}
/*e: function expunge */

/*s: function execsh */
int
execsh(char *args, char *cmd, Bufblock *buf, Envy *e)
{
    char *p;
    int tot, n, pid, in[2], out[2];

    if(buf && pipe(out) < 0){
        perror("pipe");
        Exit();
    }
    pid = rfork(RFPROC|RFFDG|RFENVG);
    if(pid < 0){
        perror("mk rfork");
        Exit();
    }
    if(pid == 0){
        if(buf)
            close(out[0]);
        if(pipe(in) < 0){
            perror("pipe");
            Exit();
        }
        pid = fork();
        if(pid < 0){
            perror("mk fork");
            Exit();
        }
        if(pid != 0){
            dup(in[0], 0);
            if(buf){
                dup(out[1], 1);
                close(out[1]);
            }
            close(in[0]);
            close(in[1]);
            if (e)
                exportenv(e);
            if(shflags)
                execl(shell, shellname, shflags, args, nil);
            else
                execl(shell, shellname, args, nil);
            perror(shell);
            _exits("exec");
        }
        close(out[1]);
        close(in[0]);
        p = cmd+strlen(cmd);
        while(cmd < p){
            n = write(in[1], cmd, p-cmd);
            if(n < 0)
                break;
            cmd += n;
        }
        close(in[1]);
        _exits(0);
    }
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
    return pid;
}
/*e: function execsh */

/*s: function pipecmd */
int
pipecmd(char *cmd, Envy *e, int *fd)
{
    int pid, pfd[2];

    if(DEBUG(D_EXEC))
        fprint(1, "pipecmd='%s'\n", cmd);/**/

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
/*e: function pipecmd */

/*s: function Exit */
void
Exit(void)
{
    while(waitpid() >= 0)
        ;
    exits("error");
}
/*e: function Exit */

/*s: function notifyf */
int
notifyf(void *a, char *msg)
{
    static int nnote;

    USED(a);
    if(++nnote > 100){	/* until andrew fixes his program */
        fprint(STDERR, "mk: too many notes\n");
        notify(0);
        abort();
    }
    if(strcmp(msg, "interrupt")!=0 && strcmp(msg, "hangup")!=0)
        return 0;
    killchildren(msg);
    return -1;
}
/*e: function notifyf */

/*s: function catchnotes */
void
catchnotes()
{
    atnotify(notifyf, 1);
}
/*e: function catchnotes */

/*s: function maketmp */
char*
maketmp(void)
{
    static char temp[] = "/tmp/mkargXXXXXX";

    mktemp(temp);
    return temp;
}
/*e: function maketmp */

/*s: function chgtime */
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
/*e: function chgtime */

/*s: function rcopy */
void
rcopy(char **to, Resub *match, int n)
{
    int c;
    char *p;

    *to = match->sp;		/* stem0 matches complete target */
    for(to++, match++; --n > 0; to++, match++){
        if(match->sp && match->ep){
            p = match->ep;
            c = *p;
            *p = 0;
            *to = strdup(match->sp);
            *p = c;
        }
        else
            *to = 0;
    }
}
/*e: function rcopy */

/*s: function dirtime */
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
/*e: function dirtime */

/*s: function bulkmtime */
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
    ss = strdup(sym);
    symlook(ss, S_BULKED, (void*)ss);
    dirtime(s, buf);
}
/*e: function bulkmtime */

/*s: function mkmtime */
ulong
mkmtime(char *name, bool force)
{
    Dir *d;
    char buf[4096];
    ulong t;
    /*s: [[mkmtime]] locals */
    //char *s, *ss;
    //char carry;
    //Symtab *sym;
    /*e: [[mkmtime]] locals */

    strecpy(buf, buf + sizeof buf - 1, name);
    cleanname(buf);
    name = buf;

    /*s: [[mkmtime()]] bulk dir optimisation */
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
    //TODO        carry = 0;
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

    if((d = dirstat(name)) == nil)
        return 0;

    t = d->mtime;
    free(d);

    return t;
}
/*e: function mkmtime */

/*e: mk/plan9.c */
