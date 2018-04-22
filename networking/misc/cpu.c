/*s: networking/misc/cpu.c */
/*
 * cpu.c - Make a connection to a cpu server
 *
 *	   Invoked by listen as 'cpu -R | -N service net netdir'
 *	    	   by users  as 'cpu [-h system] [-c cmd args ...]'
 */

#include <u.h>
#include <libc.h>
#include <bio.h>
#include <auth.h>
#include <fcall.h>
#include <libsec.h>

/*s: constant [[Maxfdata]] */
#define	Maxfdata 8192
/*e: constant [[Maxfdata]] */
/*s: constant [[MaxStr]] */
#define MaxStr 128
/*e: constant [[MaxStr]] */

void	remoteside(int);
void	fatal(int, char*, ...);
void	lclnoteproc(int);
void	rmtnoteproc(void);
void	catcher(void*, char*);
void	usage(void);
void	writestr(int, char*, char*, int);
int	readstr(int, char*, int);
char	*rexcall(int*, char*, char*);
int	setamalg(char*);
/*s: global [[keyspec]] */
char *keyspec = "";
/*e: global [[keyspec]] */

/*s: global [[notechan]] */
int 	notechan;
/*e: global [[notechan]] */
/*s: global [[exportpid]] */
int	exportpid;
/*e: global [[exportpid]] */
/*s: global [[system]] */
char	*system;
/*e: global [[system]] */
/*s: global [[cflag]] */
int	cflag;
/*e: global [[cflag]] */
/*s: global [[dbg]] */
int	dbg;
/*e: global [[dbg]] */
/*s: global [[user]] */
char	*user;
/*e: global [[user]] */
/*s: global [[patternfile]] */
char	*patternfile;
/*e: global [[patternfile]] */
/*s: global [[origargs]] */
char	*origargs;
/*e: global [[origargs]] */

/*s: global [[srvname]] */
char	*srvname = "ncpu";
/*e: global [[srvname]] */
/*s: global [[exportfs]] */
char	*exportfs = "/bin/exportfs";
/*e: global [[exportfs]] */
/*s: global [[ealgs]] */
char	*ealgs = "rc4_256 sha1";
/*e: global [[ealgs]] */

/*s: global [[msgsize]] */
/* message size for exportfs; may be larger so we can do big graphics in CPU window */
int	msgsize = Maxfdata+IOHDRSZ;
/*e: global [[msgsize]] */

/* authentication mechanisms */
static int	netkeyauth(int);
static int	netkeysrvauth(int, char*);
static int	p9auth(int);
static int	srvp9auth(int, char*);
static int	noauth(int);
static int	srvnoauth(int, char*);

typedef struct AuthMethod AuthMethod;
/*s: global [[authmethod]] */
struct AuthMethod {
    char	*name;			/* name of method */
    int	(*cf)(int);		/* client side authentication */
    int	(*sf)(int, char*);	/* server side authentication */
} authmethod[] =
{
    { "p9",		p9auth,		srvp9auth,},
    { "netkey",	netkeyauth,	netkeysrvauth,},
//	{ "none",	noauth,		srvnoauth,},
    { nil,	nil}
};
/*e: global [[authmethod]] */
/*s: global [[am]] */
AuthMethod *am = authmethod;	/* default is p9 */
/*e: global [[am]] */

/*s: global [[p9authproto]] */
char *p9authproto = "p9any";
/*e: global [[p9authproto]] */

int setam(char*);

/*s: function [[usage]]([[(networking/misc/cpu.c)]]) */
void
usage(void)
{
    fprint(2, "usage: cpu [-h system] [-u user] [-a authmethod] "
        "[-e 'crypt hash'] [-k keypattern] [-P patternfile] "
        "[-c cmd arg ...]\n");
    exits("usage");
}
/*e: function [[usage]]([[(networking/misc/cpu.c)]]) */

/*s: function [[procgetname]] */
/*
 * reading /proc/pid/args yields either "name args" or "name [display args]",
 * so return only args or display args.
 */
static char *
procgetname(void)
{
    int fd, n;
    char *lp, *rp;
    char buf[256];

    snprint(buf, sizeof buf, "#p/%d/args", getpid());
    if((fd = open(buf, OREAD)) < 0)
        return strdup("");
    *buf = '\0';
    n = read(fd, buf, sizeof buf-1);
    close(fd);
    if (n >= 0)
        buf[n] = '\0';
    if ((lp = strchr(buf, '[')) == nil || (rp = strrchr(buf, ']')) == nil) {
        lp = strchr(buf, ' ');
        if (lp == nil)
            return strdup("");
        else
            return strdup(lp+1);
    }
    *rp = '\0';
    return strdup(lp+1);
}
/*e: function [[procgetname]] */

/*s: function [[procsetname]]([[(networking/misc/cpu.c)]]) */
/*
 * based on libthread's threadsetname, but drags in less library code.
 * actually just sets the arguments displayed.
 */
void
procsetname(char *fmt, ...)
{
    int fd;
    char *cmdname;
    char buf[128];
    va_list arg;

    va_start(arg, fmt);
    cmdname = vsmprint(fmt, arg);
    va_end(arg);
    if (cmdname == nil)
        return;
    snprint(buf, sizeof buf, "#p/%d/args", getpid());
    if((fd = open(buf, OWRITE)) >= 0){
        write(fd, cmdname, strlen(cmdname)+1);
        close(fd);
    }
    free(cmdname);
}
/*e: function [[procsetname]]([[(networking/misc/cpu.c)]]) */

/*s: function [[main]]([[(networking/misc/cpu.c)]]) */
void
main(int argc, char **argv)
{
    char dat[MaxStr], buf[MaxStr], cmd[MaxStr], *p, *err;
    int ac, fd, ms, data;
    char *av[10];

    quotefmtinstall();
    origargs = procgetname();
    /* see if we should use a larger message size */
    fd = open("/dev/draw", OREAD);
    if(fd > 0){
        ms = iounit(fd);
        if(msgsize < ms+IOHDRSZ)
            msgsize = ms+IOHDRSZ;
        close(fd);
    }

    user = getuser();
    if(user == nil)
        fatal(1, "can't read user name");
    ARGBEGIN{
    case 'a':
        p = EARGF(usage());
        if(setam(p) < 0)
            fatal(0, "unknown auth method %s", p);
        break;
    case 'e':
        ealgs = EARGF(usage());
        if(*ealgs == 0 || strcmp(ealgs, "clear") == 0)
            ealgs = nil;
        break;
    case 'd':
        dbg++;
        break;
    case 'f':
        /* ignored but accepted for compatibility */
        break;
    case 'O':
        p9authproto = "p9sk2";
        remoteside(1);				/* From listen */
        break;
    case 'R':				/* From listen */
        remoteside(0);
        break;
    case 'h':
        system = EARGF(usage());
        break;
    case 'c':
        cflag++;
        cmd[0] = '!';
        cmd[1] = '\0';
        while(p = ARGF()) {
            strcat(cmd, " ");
            strcat(cmd, p);
        }
        break;
    case 'k':
        keyspec = smprint("%s %s", keyspec, EARGF(usage()));
        break;
    case 'P':
        patternfile = EARGF(usage());
        break;
    case 'u':
        user = EARGF(usage());
        keyspec = smprint("%s user=%s", keyspec, user);
        break;
    default:
        usage();
    }ARGEND;


    if(argc != 0)
        usage();

    if(system == nil) {
        p = getenv("cpu");
        if(p == 0)
            fatal(0, "set $cpu");
        system = p;
    }

    if(err = rexcall(&data, system, srvname))
        fatal(1, "%s: %s", err, system);

    procsetname("%s", origargs);
    /* Tell the remote side the command to execute and where our working directory is */
    if(cflag)
        writestr(data, cmd, "command", 0);
    if(getwd(dat, sizeof(dat)) == 0)
        writestr(data, "NO", "dir", 0);
    else
        writestr(data, dat, "dir", 0);

    /* start up a process to pass along notes */
    lclnoteproc(data);

    /* 
     *  Wait for the other end to execute and start our file service
     *  of /mnt/term
     */
    if(readstr(data, buf, sizeof(buf)) < 0)
        fatal(1, "waiting for FS: %r");
    if(strncmp("FS", buf, 2) != 0) {
        print("remote cpu: %s", buf);
        exits(buf);
    }

    /* Begin serving the gnot namespace */
    close(0);
    dup(data, 0);
    close(data);

    sprint(buf, "%d", msgsize);
    ac = 0;
    av[ac++] = exportfs;
    av[ac++] = "-m";
    av[ac++] = buf;
    if(dbg)
        av[ac++] = "-d";
    if(patternfile != nil){
        av[ac++] = "-P";
        av[ac++] = patternfile;
    }
    av[ac] = nil;
    exec(exportfs, av);
    fatal(1, "starting exportfs");
}
/*e: function [[main]]([[(networking/misc/cpu.c)]]) */

/*s: function [[fatal]]([[(networking/misc/cpu.c)]]) */
void
fatal(int syserr, char *fmt, ...)
{
    Fmt f;
    char *str;
    va_list arg;

    fmtstrinit(&f);
    fmtprint(&f, "cpu: ");
    va_start(arg, fmt);
    fmtvprint(&f, fmt, arg);
    va_end(arg);
    if(syserr)
        fmtprint(&f, ": %r");
    str = fmtstrflush(&f);

    fprint(2, "%s\n", str);
    syslog(0, "cpu", str);
    exits(str);
}
/*e: function [[fatal]]([[(networking/misc/cpu.c)]]) */

/*s: global [[negstr]] */
char *negstr = "negotiating authentication method";
/*e: global [[negstr]] */

/*s: global [[bug]] */
char bug[256];
/*e: global [[bug]] */

/*s: function [[old9p]] */
int
old9p(int fd)
{
    int p[2];

    if(pipe(p) < 0)
        fatal(1, "pipe");

    switch(rfork(RFPROC|RFFDG|RFNAMEG)) {
    case -1:
        fatal(1, "rfork srvold9p");
    case 0:
        if(fd != 1){
            dup(fd, 1);
            close(fd);
        }
        if(p[0] != 0){
            dup(p[0], 0);
            close(p[0]);
        }
        close(p[1]);
        if(0){
            fd = open("/sys/log/cpu", OWRITE);
            if(fd != 2){
                dup(fd, 2);
                close(fd);
            }
            execl("/bin/srvold9p", "srvold9p", "-ds", nil);
        } else
            execl("/bin/srvold9p", "srvold9p", "-s", nil);
        fatal(1, "exec srvold9p");
    default:
        close(fd);
        close(p[0]);
    }
    return p[1];	
}
/*e: function [[old9p]] */

/*s: function [[remoteside]] */
/* Invoked with stdin, stdout and stderr connected to the network connection */
void
remoteside(int old)
{
    char user[MaxStr], home[MaxStr], buf[MaxStr], xdir[MaxStr], cmd[MaxStr];
    int i, n, fd, badchdir, gotcmd;

    rfork(RFENVG);
    putenv("service", "cpu");
    fd = 0;

    /* negotiate authentication mechanism */
    n = readstr(fd, cmd, sizeof(cmd));
    if(n < 0)
        fatal(1, "authenticating");
    if(setamalg(cmd) < 0){
        writestr(fd, "unsupported auth method", nil, 0);
        fatal(1, "bad auth method %s", cmd);
    } else
        writestr(fd, "", "", 1);

    fd = (*am->sf)(fd, user);
    if(fd < 0)
        fatal(1, "srvauth");

    /* Set environment values for the user */
    putenv("user", user);
    sprint(home, "/usr/%s", user);
    putenv("home", home);

    /* Now collect invoking cpu's current directory or possibly a command */
    gotcmd = 0;
    if(readstr(fd, xdir, sizeof(xdir)) < 0)
        fatal(1, "dir/cmd");
    if(xdir[0] == '!') {
        strcpy(cmd, &xdir[1]);
        gotcmd = 1;
        if(readstr(fd, xdir, sizeof(xdir)) < 0)
            fatal(1, "dir");
    }

    /* Establish the new process at the current working directory of the
     * gnot */
    badchdir = 0;
    if(strcmp(xdir, "NO") == 0)
        chdir(home);
    else if(chdir(xdir) < 0) {
        badchdir = 1;
        chdir(home);
    }

    /* Start the gnot serving its namespace */
    writestr(fd, "FS", "FS", 0);
    writestr(fd, "/", "exportfs dir", 0);

    n = read(fd, buf, sizeof(buf));
    if(n != 2 || buf[0] != 'O' || buf[1] != 'K')
        exits("remote tree");

    if(old)
        fd = old9p(fd);

    /* make sure buffers are big by doing fversion explicitly; pick a huge number; other side will trim */
    strcpy(buf, VERSION9P);
    if(fversion(fd, 64*1024, buf, sizeof buf) < 0)
        exits("fversion failed");
    if(mount(fd, -1, "/mnt/term", MCREATE|MREPL, "") < 0)
        exits("mount failed");

    close(fd);

    /* the remote noteproc uses the mount so it must follow it */
    rmtnoteproc();

    for(i = 0; i < 3; i++)
        close(i);

    if(open("/mnt/term/dev/cons", OREAD) != 0)
        exits("open stdin");
    if(open("/mnt/term/dev/cons", OWRITE) != 1)
        exits("open stdout");
    dup(1, 2);

    if(badchdir)
        print("cpu: failed to chdir to '%s'\n", xdir);

    if(gotcmd)
        execl("/bin/rc", "rc", "-lc", cmd, nil);
    else
        execl("/bin/rc", "rc", "-li", nil);
    fatal(1, "exec shell");
}
/*e: function [[remoteside]] */

/*s: function [[rexcall]] */
char*
rexcall(int *fd, char *host, char *service)
{
    char *na;
    char dir[MaxStr];
    char err[ERRMAX];
    char msg[MaxStr];
    int n;

    na = netmkaddr(host, 0, service);
    procsetname("dialing %s", na);
    if((*fd = dial(na, 0, dir, 0)) < 0)
        return "can't dial";

    /* negotiate authentication mechanism */
    if(ealgs != nil)
        snprint(msg, sizeof(msg), "%s %s", am->name, ealgs);
    else
        snprint(msg, sizeof(msg), "%s", am->name);
    procsetname("writing %s", msg);
    writestr(*fd, msg, negstr, 0);
    procsetname("awaiting auth method");
    n = readstr(*fd, err, sizeof err);
    if(n < 0)
        return negstr;
    if(*err){
        werrstr(err);
        return negstr;
    }

    /* authenticate */
    procsetname("%s: auth via %s", origargs, am->name);
    *fd = (*am->cf)(*fd);
    if(*fd < 0)
        return "can't authenticate";
    return 0;
}
/*e: function [[rexcall]] */

/*s: function [[writestr]] */
void
writestr(int fd, char *str, char *thing, int ignore)
{
    int l, n;

    l = strlen(str);
    n = write(fd, str, l+1);
    if(!ignore && n < 0)
        fatal(1, "writing network: %s", thing);
}
/*e: function [[writestr]] */

/*s: function [[readstr]] */
int
readstr(int fd, char *str, int len)
{
    int n;

    while(len) {
        n = read(fd, str, 1);
        if(n < 0) 
            return -1;
        if(*str == '\0')
            return 0;
        str++;
        len--;
    }
    return -1;
}
/*e: function [[readstr]] */

/*s: function [[readln]] */
static int
readln(char *buf, int n)
{
    int i;
    char *p;

    n--;	/* room for \0 */
    p = buf;
    for(i=0; i<n; i++){
        if(read(0, p, 1) != 1)
            break;
        if(*p == '\n' || *p == '\r')
            break;
        p++;
    }
    *p = '\0';
    return p-buf;
}
/*e: function [[readln]] */

/*s: function [[netkeyauth]] */
/*
 *  user level challenge/response
 */
static int
netkeyauth(int fd)
{
    char chall[32];
    char resp[32];

    strecpy(chall, chall+sizeof chall, getuser());
    print("user[%s]: ", chall);
    if(readln(resp, sizeof(resp)) < 0)
        return -1;
    if(*resp != 0)
        strcpy(chall, resp);
    writestr(fd, chall, "challenge/response", 1);

    for(;;){
        if(readstr(fd, chall, sizeof chall) < 0)
            break;
        if(*chall == 0)
            return fd;
        print("challenge: %s\nresponse: ", chall);
        if(readln(resp, sizeof(resp)) < 0)
            break;
        writestr(fd, resp, "challenge/response", 1);
    }
    return -1;
}
/*e: function [[netkeyauth]] */

/*s: function [[netkeysrvauth]] */
static int
netkeysrvauth(int fd, char *user)
{
    char response[32];
    Chalstate *ch;
    int tries;
    AuthInfo *ai;

    if(readstr(fd, user, 32) < 0)
        return -1;

    ai = nil;
    ch = nil;
    for(tries = 0; tries < 10; tries++){
        if((ch = auth_challenge("proto=p9cr role=server user=%q", user)) == nil)
            return -1;
        writestr(fd, ch->chal, "challenge", 1);
        if(readstr(fd, response, sizeof response) < 0)
            return -1;
        ch->resp = response;
        ch->nresp = strlen(response);
        if((ai = auth_response(ch)) != nil)
            break;
    }
    auth_freechal(ch);
    if(ai == nil)
        return -1;
    writestr(fd, "", "challenge", 1);
    if(auth_chuid(ai, 0) < 0)
        fatal(1, "newns");
    auth_freeAI(ai);
    return fd;
}
/*e: function [[netkeysrvauth]] */

/*s: function [[mksecret]] */
static void
mksecret(char *t, uchar *f)
{
    sprint(t, "%2.2ux%2.2ux%2.2ux%2.2ux%2.2ux%2.2ux%2.2ux%2.2ux%2.2ux%2.2ux",
        f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], f[8], f[9]);
}
/*e: function [[mksecret]] */

/*s: function [[p9auth]] */
/*
 *  plan9 authentication followed by rc4 encryption
 */
static int
p9auth(int fd)
{
    uchar key[16];
    uchar digest[SHA1dlen];
    char fromclientsecret[21];
    char fromserversecret[21];
    int i;
    AuthInfo *ai;

    procsetname("%s: auth_proxy proto=%q role=client %s",
        origargs, p9authproto, keyspec);
    ai = auth_proxy(fd, auth_getkey, "proto=%q role=client %s", p9authproto, keyspec);
    if(ai == nil)
        return -1;
    memmove(key+4, ai->secret, ai->nsecret);
    if(ealgs == nil)
        return fd;

    /* exchange random numbers */
    srand(truerand());
    for(i = 0; i < 4; i++)
        key[i] = rand();
    procsetname("writing p9 key");
    if(write(fd, key, 4) != 4)
        return -1;
    procsetname("reading p9 key");
    if(readn(fd, key+12, 4) != 4)
        return -1;

    /* scramble into two secrets */
    sha1(key, sizeof(key), digest, nil);
    mksecret(fromclientsecret, digest);
    mksecret(fromserversecret, digest+10);

    /* set up encryption */
    procsetname("pushssl");
    i = pushssl(fd, ealgs, fromclientsecret, fromserversecret, nil);
    if(i < 0)
        werrstr("can't establish ssl connection: %r");
    return i;
}
/*e: function [[p9auth]] */

/*s: function [[noauth]] */
static int
noauth(int fd)
{
    ealgs = nil;
    return fd;
}
/*e: function [[noauth]] */

/*s: function [[srvnoauth]] */
static int
srvnoauth(int fd, char *user)
{
    strecpy(user, user+MaxStr, getuser());
    ealgs = nil;
    newns(user, nil);
    return fd;
}
/*e: function [[srvnoauth]] */

/*s: function [[loghex]] */
void
loghex(uchar *p, int n)
{
    char buf[100];
    int i;

    for(i = 0; i < n; i++)
        sprint(buf+2*i, "%2.2ux", p[i]);
    syslog(0, "cpu", buf);
}
/*e: function [[loghex]] */

/*s: function [[srvp9auth]] */
static int
srvp9auth(int fd, char *user)
{
    uchar key[16];
    uchar digest[SHA1dlen];
    char fromclientsecret[21];
    char fromserversecret[21];
    int i;
    AuthInfo *ai;

    ai = auth_proxy(0, nil, "proto=%q role=server %s", p9authproto, keyspec);
    if(ai == nil)
        return -1;
    if(auth_chuid(ai, nil) < 0)
        return -1;
    strecpy(user, user+MaxStr, ai->cuid);
    memmove(key+4, ai->secret, ai->nsecret);

    if(ealgs == nil)
        return fd;

    /* exchange random numbers */
    srand(truerand());
    for(i = 0; i < 4; i++)
        key[i+12] = rand();
    if(readn(fd, key, 4) != 4)
        return -1;
    if(write(fd, key+12, 4) != 4)
        return -1;

    /* scramble into two secrets */
    sha1(key, sizeof(key), digest, nil);
    mksecret(fromclientsecret, digest);
    mksecret(fromserversecret, digest+10);

    /* set up encryption */
    i = pushssl(fd, ealgs, fromserversecret, fromclientsecret, nil);
    if(i < 0)
        werrstr("can't establish ssl connection: %r");
    return i;
}
/*e: function [[srvp9auth]] */

/*s: function [[setam]] */
/*
 *  set authentication mechanism
 */
int
setam(char *name)
{
    for(am = authmethod; am->name != nil; am++)
        if(strcmp(am->name, name) == 0)
            return 0;
    am = authmethod;
    return -1;
}
/*e: function [[setam]] */

/*s: function [[setamalg]] */
/*
 *  set authentication mechanism and encryption/hash algs
 */
int
setamalg(char *s)
{
    ealgs = strchr(s, ' ');
    if(ealgs != nil)
        *ealgs++ = 0;
    return setam(s);
}
/*e: function [[setamalg]] */

/*s: global [[rmtnotefile]] */
char *rmtnotefile = "/mnt/term/dev/cpunote";
/*e: global [[rmtnotefile]] */

/*s: function [[rmtnoteproc]] */
/*
 *  loop reading /mnt/term/dev/note looking for notes.
 *  The child returns to start the shell.
 */
void
rmtnoteproc(void)
{
    int n, fd, pid, notepid;
    char buf[256];

    /* new proc returns to start shell */
    pid = rfork(RFPROC|RFFDG|RFNOTEG|RFNAMEG|RFMEM);
    switch(pid){
    case -1:
        syslog(0, "cpu", "cpu -R: can't start noteproc: %r");
        return;
    case 0:
        return;
    }

    /* new proc reads notes from other side and posts them to shell */
    switch(notepid = rfork(RFPROC|RFFDG|RFMEM)){
    case -1:
        syslog(0, "cpu", "cpu -R: can't start wait proc: %r");
        _exits(0);
    case 0:
        fd = open(rmtnotefile, OREAD);
        if(fd < 0){
            syslog(0, "cpu", "cpu -R: can't open %s", rmtnotefile);
            _exits(0);
        }
    
        for(;;){
            n = read(fd, buf, sizeof(buf)-1);
            if(n <= 0){
                postnote(PNGROUP, pid, "hangup");
                _exits(0);
            }
            buf[n] = 0;
            postnote(PNGROUP, pid, buf);
        }
    }

    /* original proc waits for shell proc to die and kills note proc */
    for(;;){
        n = waitpid();
        if(n < 0 || n == pid)
            break;
    }
    postnote(PNPROC, notepid, "kill");
    _exits(0);
}
/*e: function [[rmtnoteproc]] */

/*s: enum [[_anon_ (networking/misc/cpu.c)]] */
enum
{
    Qdir,
    Qcpunote,

    Nfid = 32,
};
/*e: enum [[_anon_ (networking/misc/cpu.c)]] */

/*s: global [[fstab]] */
struct {
    char	*name;
    Qid	qid;
    ulong	perm;
} fstab[] =
{
    [Qdir]		{ ".",		{Qdir, 0, QTDIR},	DMDIR|0555	},
    [Qcpunote]	{ "cpunote",	{Qcpunote, 0},		0444		},
};
/*e: global [[fstab]] */

typedef struct Note Note;
/*s: struct [[Note]] */
struct Note
{
    Note *next;
    char msg[ERRMAX];
};
/*e: struct [[Note]] */

typedef struct Request Request;
/*s: struct [[Request]] */
struct Request
{
    Request *next;
    Fcall f;
};
/*e: struct [[Request]] */

typedef struct Fid Fid;
/*s: struct [[Fid]] */
struct Fid
{
    int	fid;
    int	file;
    int	omode;
};
/*e: struct [[Fid]] */
/*s: global [[fids]] */
Fid fids[Nfid];
/*e: global [[fids]] */

/*s: global [[nfs]] */
struct {
    Lock;
    Note *nfirst, *nlast;
    Request *rfirst, *rlast;
} nfs;
/*e: global [[nfs]] */

/*s: function [[fsreply]] */
int
fsreply(int fd, Fcall *f)
{
    uchar buf[IOHDRSZ+Maxfdata];
    int n;

    if(dbg)
        fprint(2, "notefs: <-%F\n", f);
    n = convS2M(f, buf, sizeof buf);
    if(n > 0){
        if(write(fd, buf, n) != n){
            close(fd);
            return -1;
        }
    }
    return 0;
}
/*e: function [[fsreply]] */

/*s: function [[kick]] */
/* match a note read request with a note, reply to the request */
int
kick(int fd)
{
    Request *rp;
    Note *np;
    int rv;

    for(;;){
        lock(&nfs);
        rp = nfs.rfirst;
        np = nfs.nfirst;
        if(rp == nil || np == nil){
            unlock(&nfs);
            break;
        }
        nfs.rfirst = rp->next;
        nfs.nfirst = np->next;
        unlock(&nfs);

        rp->f.type = Rread;
        rp->f.count = strlen(np->msg);
        rp->f.data = np->msg;
        rv = fsreply(fd, &rp->f);
        free(rp);
        free(np);
        if(rv < 0)
            return -1;
    }
    return 0;
}
/*e: function [[kick]] */

/*s: function [[flushreq]] */
void
flushreq(int tag)
{
    Request **l, *rp;

    lock(&nfs);
    for(l = &nfs.rfirst; *l != nil; l = &(*l)->next){
        rp = *l;
        if(rp->f.tag == tag){
            *l = rp->next;
            unlock(&nfs);
            free(rp);
            return;
        }
    }
    unlock(&nfs);
}
/*e: function [[flushreq]] */

/*s: function [[getfid]] */
Fid*
getfid(int fid)
{
    int i, freefid;

    freefid = -1;
    for(i = 0; i < Nfid; i++){
        if(freefid < 0 && fids[i].file < 0)
            freefid = i;
        if(fids[i].fid == fid)
            return &fids[i];
    }
    if(freefid >= 0){
        fids[freefid].fid = fid;
        return &fids[freefid];
    }
    return nil;
}
/*e: function [[getfid]] */

/*s: function [[fsstat]] */
int
fsstat(int fd, Fid *fid, Fcall *f)
{
    Dir d;
    uchar statbuf[256];

    memset(&d, 0, sizeof(d));
    d.name = fstab[fid->file].name;
    d.uid = user;
    d.gid = user;
    d.muid = user;
    d.qid = fstab[fid->file].qid;
    d.mode = fstab[fid->file].perm;
    d.atime = d.mtime = time(0);
    f->stat = statbuf;
    f->nstat = convD2M(&d, statbuf, sizeof statbuf);
    return fsreply(fd, f);
}
/*e: function [[fsstat]] */

/*s: function [[fsread]] */
int
fsread(int fd, Fid *fid, Fcall *f)
{
    Dir d;
    uchar buf[256];
    Request *rp;

    switch(fid->file){
    default:
        return -1;
    case Qdir:
        if(f->offset == 0 && f->count >0){
            memset(&d, 0, sizeof(d));
            d.name = fstab[Qcpunote].name;
            d.uid = user;
            d.gid = user;
            d.muid = user;
            d.qid = fstab[Qcpunote].qid;
            d.mode = fstab[Qcpunote].perm;
            d.atime = d.mtime = time(0);
            f->count = convD2M(&d, buf, sizeof buf);
            f->data = (char*)buf;
        } else
            f->count = 0;
        return fsreply(fd, f);
    case Qcpunote:
        rp = mallocz(sizeof(*rp), 1);
        if(rp == nil)
            return -1;
        rp->f = *f;
        lock(&nfs);
        if(nfs.rfirst == nil)
            nfs.rfirst = rp;
        else
            nfs.rlast->next = rp;
        nfs.rlast = rp;
        unlock(&nfs);
        return kick(fd);;
    }
}
/*e: function [[fsread]] */

/*s: global [[Eperm]] */
char Eperm[] = "permission denied";
/*e: global [[Eperm]] */
/*s: global [[Enofile]] */
char Enofile[] = "out of files";
/*e: global [[Enofile]] */
/*s: global [[Enotdir]] */
char Enotdir[] = "not a directory";
/*e: global [[Enotdir]] */

/*s: function [[notefs]] */
void
notefs(int fd)
{
    uchar buf[IOHDRSZ+Maxfdata];
    int i, n, ncpunote;
    Fcall f;
    Qid wqid[MAXWELEM];
    Fid *fid, *nfid;
    int doreply;

    rfork(RFNOTEG);
    fmtinstall('F', fcallfmt);

    for(n = 0; n < Nfid; n++){
        fids[n].file = -1;
        fids[n].omode = -1;
    }

    ncpunote = 0;
    for(;;){
        n = read9pmsg(fd, buf, sizeof(buf));
        if(n <= 0){
            if(dbg)
                fprint(2, "read9pmsg(%d) returns %d: %r\n", fd, n);
            break;
        }
        if(convM2S(buf, n, &f) <= BIT16SZ)
            break;
        if(dbg)
            fprint(2, "notefs: ->%F\n", &f);
        doreply = 1;
        fid = getfid(f.fid);
        if(fid == nil){
nofids:
            f.type = Rerror;
            f.ename = Enofile;
            fsreply(fd, &f);
            continue;
        }
        switch(f.type++){
        default:
            f.type = Rerror;
            f.ename = "unknown type";
            break;
        case Tflush:
            flushreq(f.oldtag);
            break;
        case Tversion:
            if(f.msize > IOHDRSZ+Maxfdata)
                f.msize = IOHDRSZ+Maxfdata;
            break;
        case Tauth:
            f.type = Rerror;
            f.ename = "authentication not required";
            break;
        case Tattach:
            f.qid = fstab[Qdir].qid;
            fid->file = Qdir;
            break;
        case Twalk:
            nfid = nil;
            if(f.newfid != f.fid){
                nfid = getfid(f.newfid);
                if(nfid == nil)
                    goto nofids;
                nfid->file = fid->file;
                fid = nfid;
            }
            for(i=0; i<f.nwname && i<MAXWELEM; i++){
                if(fid->file != Qdir){
                    f.type = Rerror;
                    f.ename = Enotdir;
                    break;
                }
                if(strcmp(f.wname[i], "..") == 0){
                    wqid[i] = fstab[Qdir].qid;
                    continue;
                }
                if(strcmp(f.wname[i], "cpunote") != 0){
                    if(i == 0){
                        f.type = Rerror;
                        f.ename = "file does not exist";
                    }
                    break;
                }
                fid->file = Qcpunote;
                wqid[i] = fstab[Qcpunote].qid;
            }
            if(nfid != nil && (f.type == Rerror || i < f.nwname))
                nfid ->file = -1;
            if(f.type != Rerror){
                f.nwqid = i;
                for(i=0; i<f.nwqid; i++)
                    f.wqid[i] = wqid[i];
            }
            break;
        case Topen:
            if(f.mode != OREAD){
                f.type = Rerror;
                f.ename = Eperm;
                break;
            }
            fid->omode = f.mode;
            if(fid->file == Qcpunote)
                ncpunote++;
            f.qid = fstab[fid->file].qid;
            f.iounit = 0;
            break;
        case Tread:
            if(fsread(fd, fid, &f) < 0)
                goto err;
            doreply = 0;
            break;
        case Tclunk:
            if(fid->omode != -1 && fid->file == Qcpunote){
                ncpunote--;
                if(ncpunote == 0)	/* remote side is done */
                    goto err;
            }
            fid->file = -1;
            fid->omode = -1;
            break;
        case Tstat:
            if(fsstat(fd, fid, &f) < 0)
                goto err;
            doreply = 0;
            break;
        case Tcreate:
        case Twrite:
        case Tremove:
        case Twstat:
            f.type = Rerror;
            f.ename = Eperm;
            break;
        }
        if(doreply)
            if(fsreply(fd, &f) < 0)
                break;
    }
err:
    if(dbg)
        fprint(2, "notefs exiting: %r\n");
    werrstr("success");
    postnote(PNGROUP, exportpid, "kill");
    if(dbg)
        fprint(2, "postnote PNGROUP %d: %r\n", exportpid);
    close(fd);
}
/*e: function [[notefs]] */

/*s: global [[notebuf]] */
char 	notebuf[ERRMAX];
/*e: global [[notebuf]] */

/*s: function [[catcher]] */
void
catcher(void*, char *text)
{
    int n;

    n = strlen(text);
    if(n >= sizeof(notebuf))
        n = sizeof(notebuf)-1;
    memmove(notebuf, text, n);
    notebuf[n] = '\0';
    noted(NCONT);
}
/*e: function [[catcher]] */

/*s: function [[lclnoteproc]] */
/*
 *  mount in /dev a note file for the remote side to read.
 */
void
lclnoteproc(int netfd)
{
    Waitmsg *w;
    Note *np;
    int pfd[2];
    int pid;

    if(pipe(pfd) < 0){
        fprint(2, "cpu: can't start note proc: pipe: %r\n");
        return;
    }

    /* new proc mounts and returns to start exportfs */
    switch(pid = rfork(RFPROC|RFNAMEG|RFFDG|RFMEM)){
    default:
        exportpid = pid;
        break;
    case -1:
        fprint(2, "cpu: can't start note proc: rfork: %r\n");
        return;
    case 0:
        close(pfd[0]);
        if(mount(pfd[1], -1, "/dev", MBEFORE, "") < 0)
            fprint(2, "cpu: can't mount note proc: %r\n");
        close(pfd[1]);
        return;
    }

    close(netfd);
    close(pfd[1]);

    /* new proc listens for note file system rpc's */
    switch(rfork(RFPROC|RFNAMEG|RFMEM)){
    case -1:
        fprint(2, "cpu: can't start note proc: rfork1: %r\n");
        _exits(0);
    case 0:
        notefs(pfd[0]);
        _exits(0);
    }

    /* original proc waits for notes */
    notify(catcher);
    w = nil;
    for(;;) {
        *notebuf = 0;
        free(w);
        w = wait();
        if(w == nil) {
            if(*notebuf == 0)
                break;
            np = mallocz(sizeof(Note), 1);
            if(np != nil){
                strcpy(np->msg, notebuf);
                lock(&nfs);
                if(nfs.nfirst == nil)
                    nfs.nfirst = np;
                else
                    nfs.nlast->next = np;
                nfs.nlast = np;
                unlock(&nfs);
                kick(pfd[0]);
            }
            unlock(&nfs);
        } else if(w->pid == exportpid)
            break;
    }

    if(w == nil)
        exits(nil);
    exits(0);
/*	exits(w->msg); */
}
/*e: function [[lclnoteproc]] */
/*e: networking/misc/cpu.c */
