/*s: tracers/strace.c */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <thread.h>

// System calls tracer

// was called ratrace, maybe in hommage to Ed Wood "Rat Race" movie
/*s: enum _anon_ */
// but most people knows better 'strace'

enum {
    Stacksize	= 8*1024,
    Bufsize		= 8*1024,
};
/*e: enum _anon_ */

/*s: global out */
Channel *out;
/*e: global out */
/*s: global quit */
Channel *quit;
/*e: global quit */
/*s: global forkc */
Channel *forkc;
/*e: global forkc */
/*s: global nread */
int nread = 0;
/*e: global nread */

typedef struct Str Str;
/*s: struct Str */
struct Str {
    char	*buf;
    int	len;
};
/*e: struct Str */

/*s: function die */
void
die(char *s)
{
    fprint(2, "%s\n", s);
    exits(s);
}
/*e: function die */

/*s: function cwrite */
void
cwrite(int fd, char *path, char *cmd, int len)
{
    werrstr("");
    if (write(fd, cmd, len) < len) {
        fprint(2, "cwrite: %s: failed writing %d bytes: %r\n",
            path, len);
        sendp(quit, nil);
        threadexits(nil);
    }
}
/*e: function cwrite */

/*s: function newstr */
Str *
newstr(void)
{
    Str *s;

    s = mallocz(sizeof(Str) + Bufsize, 1);
    if (s == nil)
        sysfatal("malloc");
    s->buf = (char *)&s[1];
    return s;
}
/*e: function newstr */

/*s: function reader */
void
reader(void *v)
{
    int cfd, tfd, forking = 0, exiting, pid, newpid;
    char *ctl, *truss;
    Str *s;
    static char start[] = "start";
    static char waitstop[] = "waitstop";

    pid = (int)(uintptr)v;

    ctl = smprint("/proc/%d/ctl", pid);
    if ((cfd = open(ctl, OWRITE)) < 0)
        die(smprint("%s: %r", ctl));

    truss = smprint("/proc/%d/syscall", pid);
    if ((tfd = open(truss, OREAD)) < 0)
        die(smprint("%s: %r", truss));

    /* child was stopped by hang msg earlier */
    cwrite(cfd, ctl, waitstop, sizeof waitstop - 1);
    // useful? if it was stopped, then why need waitstop?
    // because the fork() has been done but maybe the child
    // has not yet reached the exec() and got actually stopped!

    cwrite(cfd, ctl, "startsyscall", 12);
    s = newstr();
    exiting = 0;
    while((s->len = pread(tfd, s->buf, Bufsize - 1, 0)) >= 0){
        if (forking && s->buf[1] == '=' && s->buf[3] != '-') {
            forking = 0;
            newpid = strtol(&s->buf[3], 0, 0);
            sendp(forkc, (void*)newpid);
            procrfork(reader, (void*)newpid, Stacksize, 0);
        }

        /*
         * There are three tests here and they (I hope) guarantee
         * no false positives.
         */
        if (strstr(s->buf, " Rfork") != nil) {
            char *a[8];
            char *rf;

            rf = strdup(s->buf);
           if (tokenize(rf, a, 8) == 5 &&
                strtoul(a[4], 0, 16) & RFPROC)
                forking = 1;
            free(rf);
        } else if (strstr(s->buf, " Exits") != nil)
            exiting = 1;

        sendp(out, s);	/* print line from /proc/$child/syscall */
        if (exiting) {
            s = newstr();
            strcpy(s->buf, "\n");
            sendp(out, s);
            break;
        }

        /* flush syscall trace buffer */
        cwrite(cfd, ctl, "startsyscall", 12);
        s = newstr();
    }

    sendp(quit, nil);
    threadexitsall(nil);
}
/*e: function reader */

/*s: function writer */
void
writer(void *)
{
    int newpid;
    Str *s;

    // TODO use better initializer?
    Alt a[4];

    a[0].op = CHANRCV;
    a[0].c = quit;
    a[0].v = nil;

    a[1].op = CHANRCV;
    a[1].c = out;
    a[1].v = &s;

    a[2].op = CHANRCV;
    a[2].c = forkc;
    a[2].v = &newpid;

    a[3].op = CHANEND;

    for(;;)
        switch(alt(a)){
        case 0:			/* quit */
            nread--;
            if(nread <= 0)
                goto done;
            break;
        case 1:			/* out */
            /* it's a nice null terminated thing */
            fprint(2, "%s", s->buf);
            free(s);
            break;
        case 2:			/* forkc */
            // procrfork(reader, (void*)newpid, Stacksize, 0);
            nread++;
            break;
        }
done:
    exits(nil);
}
/*e: function writer */

/*s: function usage (tracers/strace.c) */
void
usage(void)
{
    fprint(2, "Usage: strace [-c cmd [arg...]] | [pid]\n");
    exits("usage");
}
/*e: function usage (tracers/strace.c) */

/*s: function hang */
void
hang(void)
{
    int me;
    char *myctl;
    static char hang[] = "hang";

    myctl = smprint("/proc/%d/ctl", getpid());
    me = open(myctl, OWRITE);
    if (me < 0)
        sysfatal("can't open %s: %r", myctl);
    cwrite(me, myctl, hang, sizeof hang - 1);
    close(me);
    free(myctl);
}
/*e: function hang */

/*s: function threadmain */
void
threadmain(int argc, char **argv)
{
    int pid;
    char *cmd = nil;
    char **args = nil;

    /*
     * don't bother with fancy arg processing, because it picks up options
     * for the command you are starting.  Just check for -c as argv[1]
     * and then take it from there.
     */
    if (argc < 2)
        usage();
    while (argv[1][0] == '-') {
        switch(argv[1][1]) {
        case 'c':
            if (argc < 3)
                usage();
            cmd = strdup(argv[2]);
            args = &argv[2];
            break;
        default:
            usage();
        }
        ++argv;
        --argc;
    }

    /* run a command? */
    if(cmd) {
        pid = fork();
        if (pid < 0)
            sysfatal("fork failed: %r");
        if(pid == 0) {
            hang();
            exec(cmd, args);
            if(cmd[0] != '/')
                exec(smprint("/bin/%s", cmd), args);
            sysfatal("exec %s failed: %r", cmd);
        }
    } else {
        if(argc != 2)
            usage();
        pid = atoi(argv[1]);
        //TODO? send a 'stop' to its ctl file?
    }

    out   = chancreate(sizeof(char*), 0);
    quit  = chancreate(sizeof(char*), 0);
    forkc = chancreate(sizeof(ulong *), 0);
    nread++;
    procrfork(writer, nil, Stacksize, 0);
    reader((void*)pid);
}
/*e: function threadmain */
/*e: tracers/strace.c */
