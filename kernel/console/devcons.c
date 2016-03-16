/*s: devcons.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

#include    <pool.h>

// used also by edf.c
/*s: global panicking */
bool panicking;
/*e: global panicking */

/*s: hook screenputs */
void    (*screenputs)(char*, int) = nil;
/*e: hook screenputs */
/*s: hook consdebug */
void    (*consdebug)(void) = nil; // for rdb
/*e: hook consdebug */

/*s: global kbdq */
Queue*  kbdq;           /* unprocessed console input */
/*e: global kbdq */
/*s: global lineq */
Queue*  lineq;          /* processed console input */
/*e: global lineq */
/*s: global serialoq */
Queue*  serialoq;       /* serial console output */
/*e: global serialoq */
/*s: global kprintoq */
Queue*  kprintoq;       /* console output, for /dev/kprint */
/*e: global kprintoq */

/*s: global kprintinuse */
ulong   kprintinuse;        /* test and set whether /dev/kprint is open */
/*e: global kprintinuse */
/*s: global iprintscreenputs */
bool iprintscreenputs = true;
/*e: global iprintscreenputs */

/*s: struct ConsKbd */
struct ConsKbd
{
    /* a place to save up characters at interrupt time before dumping them in the queue */
    char    istage[1024];
    // pointers into istage to implement a circular buffer
    char    *iw; // write
    char    *ir; // read
    char    *ie; // end
    ILock    lockputc;

    /*s: [[ConsKbd]] other fields */
        char    line[1024]; /* current input line */
        int x;      /* index into line */
    /*x: [[ConsKbd]] other fields */
    bool raw;        /* true if we shouldn't process input */
    Ref ctl;        /* number of opens to the control file */
    /*x: [[ConsKbd]] other fields */
    bool ctlpoff; // ^P will not reboot if true
    /*e: [[ConsKbd]] other fields */
    // extra
    QLock;
};
/*e: struct ConsKbd */

/*s: global kbd */
static struct ConsKbd kbd = 
{
    .iw = kbd.istage,
    .ir = kbd.istage,
    .ie = kbd.istage + sizeof(kbd.istage),
};
/*e: global kbd */

/*s: global fasthz */
vlong   fasthz;
/*e: global fasthz */

/*s: devcons.c forward decl */
static void seedrand(void);
static int  readtime(ulong, char*, int);
static int  readbintime(char*, int);
static int  writetime(char*, int);
static int  writebintime(char*, int);
/*e: devcons.c forward decl */

/*s: function kbdqinit */
void
kbdqinit(void)
{
    kbdq = qopen(4*1024, 0, 0, 0);
    if(kbdq == nil)
        panic("kbdinit");
    qnoblock(kbdq, true);
}
/*e: function kbdqinit */

/*s: function lineqinit */
void
lineqinit(void)
{

    lineq = qopen(2*1024, 0, nil, nil);
    if(lineq == nil)
        panic("lineqinit");
    qnoblock(lineq, true);
}
/*e: function lineqinit */

/*s: function consactive */
int
consactive(void)
{
    if(serialoq)
        return qlen(serialoq) > 0;
    return 0;
}
/*e: function consactive */

/*s: function prflush */
void
prflush(void)
{
    ulong now;

    now = cpu->ticks;
    while(consactive())
        if(cpu->ticks - now >= HZ)
            break;
}
/*e: function prflush */

/*s: struct KMesg */
/*
 * Log console output so it can be retrieved via /dev/kmesg.
 * This is good for catching boot-time messages after the fact.
 */
struct KMesg {
    Lock lk;
    char buf[KMESGSIZE];
    uint n;
};
/*e: struct KMesg */

/*s: global kmesg */
struct KMesg kmesg;
/*e: global kmesg */

/*s: function kmesgputs */
static void
kmesgputs(char *str, int n)
{
    uint nn, d;

    ilock(&kmesg.lk);
    /* take the tail of huge writes */
    if(n > sizeof kmesg.buf){
        d = n - sizeof kmesg.buf;
        str += d;
        n -= d;
    }

    /* slide the buffer down to make room */
    nn = kmesg.n;
    if(nn + n >= sizeof kmesg.buf){
        d = nn + n - sizeof kmesg.buf;
        if(d)
            memmove(kmesg.buf, kmesg.buf+d, sizeof kmesg.buf-d);
        nn -= d;
    }

    /* copy the data in */
    memmove(kmesg.buf+nn, str, n);
    nn += n;
    kmesg.n = nn;
    iunlock(&kmesg.lk);
}
/*e: function kmesgputs */

/*s: function putstrn0 */
/*
 *   Print a string on the console.
 */
static void
putstrn0(char *str, int n, bool usewrite)
{
    int m;
    char *t;

    if(!islo())
        usewrite = false;

    /*s: [[putstrn0()]] kmesg handling */
    /*
     *  how many different output devices do we need?
     */
    kmesgputs(str, n);
    /*e: [[putstrn0()]] kmesg handling */
    /*s: [[putstrn0()]] if kprint */
        /*
         *  if someone is reading /dev/kprint,
         *  put the message there.
         *  if not and there's an attached bit mapped display,
         *  put the message there.
         *
         *  if there's a serial line being used as a console,
         *  put the message there.
         */
        if(kprintoq != nil && !qisclosed(kprintoq)){
            if(usewrite)
                qwrite(kprintoq, str, n);
            else
                qiwrite(kprintoq, str, n);
        }
    /*e: [[putstrn0()]] if kprint */
    else if(screenputs != nil)
        screenputs(str, n);

    /*s: [[putstrn0()]] serialoq handling */
        /*
         *   Convert \n to \r\n for serial
         *   line consoles.  Locking of the queues is left up to the screen
         *   or uart code.  Multi-line messages to serial consoles may get
         *   interspersed with other messages.
         */

        if(serialoq == nil){
            uartputs(str, n);
            return;
        }

        while(n > 0) {
            t = memchr(str, '\n', n);
            if(t && !kbd.raw) {
                m = t-str;
                if(usewrite){
                    qwrite(serialoq, str, m);
                    qwrite(serialoq, "\r\n", 2);
                } else {
                    qiwrite(serialoq, str, m);
                    qiwrite(serialoq, "\r\n", 2);
                }
                n -= m+1;
                str = t+1;
            } else {
                if(usewrite)
                    qwrite(serialoq, str, n);
                else
                    qiwrite(serialoq, str, n);
                break;
            }
        }
    /*e: [[putstrn0()]] serialoq handling */
}
/*e: function putstrn0 */

/*s: function putstrn */
void
putstrn(char *str, int n)
{
    putstrn0(str, n, false);
}
/*e: function putstrn */

/*s: function print */
bool noprint; // to debug?

int
devcons_print(char *fmt, ...)
{
    int n;
    va_list arg;
    char buf[PRINTSIZE];

    if(noprint)
        return -1;

    va_start(arg, fmt);
    n = vseprint(buf, buf+sizeof(buf), fmt, arg) - buf;
    va_end(arg);
    putstrn(buf, n);

    return n;
}
/*e: function print */

/*s: global iprintlock */
/*
 * Want to interlock iprints to avoid interlaced output on 
 * multiprocessor, but don't want to deadlock if one processor
 * dies during print and another has something important to say.
 * Make a good faith effort.
 */
static Lock iprintlock;
/*e: global iprintlock */

/*s: function iprintcanlock */
static bool
iprintcanlock(Lock *l)
{
    int i;
    
    for(i=0; i<1000; i++){
        if(canlock(l))
            return true;
        if(l->cpu == CPUS(cpu->cpuno))
            return false;
        microdelay(100);
    }
    return false;
}
/*e: function iprintcanlock */

/*s: function iprint */
int
devcons_iprint(char *fmt, ...)
{
    int n, s, locked;
    va_list arg;
    char buf[PRINTSIZE];

    s = splhi();

    va_start(arg, fmt);
    n = vseprint(buf, buf+sizeof(buf), fmt, arg) - buf;
    va_end(arg);

    locked = iprintcanlock(&iprintlock);

    kmesgputs(buf, n); // addon
    if(screenputs != nil && iprintscreenputs)
        screenputs(buf, n);
    uartputs(buf, n);

    if(locked)
        unlock(&iprintlock);

    splx(s);

    return n;
}
/*e: function iprint */

/*s: function panic */
void
devcons_panic(char *fmt, ...)
{
    int n, s;
    va_list arg;
    char buf[PRINTSIZE];

    /*s: [[panic()]] reset kprintoq */
        kprintoq = nil; /* don't try to write to /dev/kprint */
    /*e: [[panic()]] reset kprintoq */

    if(panicking)
        for(;;);
    panicking = true;

    s = splhi();
    strcpy(buf, "panic: ");
    va_start(arg, fmt);
    n = vseprint(buf+strlen(buf), buf+sizeof(buf), fmt, arg) - buf;
    va_end(arg);
    iprint("%s\n", buf);
    /*s: [[panic()]] run consdebug hook */
        if(consdebug)
            (*consdebug)();
    /*e: [[panic()]] run consdebug hook */
    splx(s);
    prflush();
    buf[n] = '\n';
    putstrn(buf, n+1);
    //TODO: put in comment for now because already got some panic with
    // lapic, but it seems to work still :)
    //dumpstack();
    //exit(1);
}
/*e: function panic */

/*s: function sysfatal */
/* libmp at least contains a few calls to sysfatal; simulate with panic */
// note that this is not a system call, even though it's prefixed with sys
//@Scheck: no dead, override also sysfatal from libc/9sys/sysfatal.c
void sysfatal(char *fmt, ...)
{
    char err[256];
    va_list arg;

    va_start(arg, fmt);
    vseprint(err, err + sizeof err, fmt, arg);
    va_end(arg);
    panic("sysfatal: %s", err);
}
/*e: function sysfatal */

/*s: function _assert */
void
devcons__assert(char *fmt)
{
    panic("assert failed at %#p: %s", getcallerpc(&fmt), fmt);
}
/*e: function _assert */

/*s: function pprint */
int
devcons_pprint(char *fmt, ...)
{
    int n;
    Chan *c;
    va_list arg;
    char buf[2*PRINTSIZE];

    if(up == nil || up->fgrp == nil)
        return 0;

    c = up->fgrp->fd[2]; // stderr
    if(c==nil || (c->mode!=OWRITE && c->mode!=ORDWR))
        return 0;

    n = snprint(buf, sizeof buf, "%s %lud: ", up->text, up->pid);
    va_start(arg, fmt);
    n = vseprint(buf+n, buf+sizeof(buf), fmt, arg) - buf;
    va_end(arg);

    if(waserror())
        return 0;
    devtab[c->type]->write(c, buf, n, c->offset);
    poperror();

    lock(c);
    c->offset += n;
    unlock(c);

    return n;
}
/*e: function pprint */

/*s: function echoscreen */
static void
echoscreen(char *buf, int n)
{
    char *e, *p;
    char ebuf[128];
    int x;

    p = ebuf;
    e = ebuf + sizeof(ebuf) - 4;
    while(n-- > 0){
        if(p >= e){
            screenputs(ebuf, p - ebuf);
            p = ebuf;
        }
        x = *buf++;
        if(x == 0x15){ // ??
            *p++ = '^';
            *p++ = 'U';
            *p++ = '\n';
        } else
            *p++ = x;
    }
    if(p != ebuf)
        screenputs(ebuf, p - ebuf);
}
/*e: function echoscreen */

/*s: function echoserialoq */
static void
echoserialoq(char *buf, int n)
{
    char *e, *p;
    char ebuf[128];
    int x;

    p = ebuf;
    e = ebuf + sizeof(ebuf) - 4;
    while(n-- > 0){
        if(p >= e){
            qiwrite(serialoq, ebuf, p - ebuf);
            p = ebuf;
        }
        x = *buf++;
        if(x == '\n'){
            *p++ = '\r';
            *p++ = '\n';
        } else if(x == 0x15){
            *p++ = '^';
            *p++ = 'U';
            *p++ = '\n';
        } else
            *p++ = x;
    }
    if(p != ebuf)
        qiwrite(serialoq, ebuf, p - ebuf);
}
/*e: function echoserialoq */

/*s: function echo */
static void
echo(char *buf, int n)
{
    /*s: [[echo()]] locals */
    static int ctrlt;
    void* tmp;
    int x;
    char *e, *p;
    /*e: [[echo()]] locals */

    if(n == 0)
        return;

    /*s: [[echo()]] special keys handler */
        e = buf+n;
        for(p = buf; p < e; p++){
            switch(*p){

           /*s: [[echo()]] special key C-p */
           case 0x10:  /* ^P */
               if(cpuserver && !kbd.ctlpoff){
                   active.exiting = true;
                   return;
               }
               break;
           /*e: [[echo()]] special key C-p */
 
            case 0x14:  /* ^T */
                ctrlt++;
                if(ctrlt > 2)
                    ctrlt = 2;
                continue;
            }
            if(ctrlt != 2)
                continue;

            /* ^T escapes */
            ctrlt = 0;
            switch(*p){
            case 's':
                dumpstack();
                return;
            case 'S':
                x = splhi();
                dumpstack();
                procdump();
                splx(x);
                return;
            case 'x':
                xsummary();
                tmp = xalloc(1000);
                xalloc(1000);
                xfree(tmp);
                xsummary();
                return;
            case 'X':
                xsummary();
                ixsummary();
                mallocsummary();
                memorysummary();
                pagersummary();
                return;
            case 'm':
                memorysummary();
                return;
            case 'p':
                x = spllo();
                procdump();
                splx(x);
                return;
            case 'q':
                scheddump();
                return;
            case 'k':
                killbig("^t ^t k");
                return;
            case 'r':
                exit(0);
                return;
            /*s: [[echo()]] C-t C-t special keys handler other cases */
                    case 'd':
                        if(consdebug == nil)
                            consdebug = rdb;
                        else
                            consdebug = nil;
                        print("consdebug now %#p\n", consdebug);
                        return;
                    case 'D':
                        if(consdebug == nil)
                            consdebug = rdb;
                        consdebug();
                        return;
            /*e: [[echo()]] C-t C-t special keys handler other cases */
            }
        }
    /*e: [[echo()]] special keys handler */

    qproduce(kbdq, buf, n); //!! add in kbd queue to be read from /dev/cons

    /*s: [[echo()]] return before any echoscreen if raw mode */
    if(kbd.raw)
        return;
    /*e: [[echo()]] return before any echoscreen if raw mode */

    /*s: [[echo()]] hooks */
    if(screenputs != nil)
       echoscreen(buf, n);
    /*x: [[echo()]] hooks */
    kmesgputs(buf, n);
    /*x: [[echo()]] hooks */
    if(serialoq)
        echoserialoq(buf, n);
    /*e: [[echo()]] hooks */
}
/*e: function echo */

/*s: function kbdcr2nl */
/*
 *  Called by a uart interrupt for console input.
 *
 *  turn '\r' into '\n' before putting it into the queue.
 */
int
kbdcr2nl(Queue*, int ch)
{
    char *next;

    ilock(&kbd.lockputc);       /* just a mutex */
    if(ch == '\r' && !kbd.raw)
        ch = '\n';
    next = kbd.iw+1;
    if(next >= kbd.ie)
        next = kbd.istage;
    if(next != kbd.ir){
        *kbd.iw = ch;
        kbd.iw = next;
    }
    iunlock(&kbd.lockputc);
    return 0;
}
/*e: function kbdcr2nl */

/*s: function kbdputc */
/*
 *  Put character, possibly a rune, into kbd at interrupt time.
 *  Called at interrupt time to process a character.
 */
void
kbdputc(Rune ch)
{
    int i, n;
    char buf[UTFmax]; // pad's fourth bugfix :)
    Rune r;
    char *next;

    /*s: [[kbdputc()]] debugging */
    extern bool kdebug;

    if(kdebug)
        print("kbdputc(0x%x)\n", ch);
    /*e: [[kbdputc()]] debugging */

    if(kbd.ir == nil)
        return;       /* in case we're not inited yet */
    
    ilock(&kbd.lockputc);       /* just a mutex */
    r = ch;
    n = runetochar(buf, &r);
    for(i = 0; i < n; i++){
        next = kbd.iw+1;
        // circular buffer
        if(next >= kbd.ie)
            next = kbd.istage;
        if(next == kbd.ir) // full
            break;
        *kbd.iw = buf[i];
        kbd.iw = next;
    }
    iunlock(&kbd.lockputc);
    return;
}
/*e: function kbdputc */

/*s: clock callback kbdputcclock */
/*
 *  we save up input characters till clock time to reduce
 *  per character interrupt overhead.
 */
static void
kbdputcclock(void)
{
    char *iw;

    /* this amortizes cost of qproduce */
    if(kbd.iw != kbd.ir){
        iw = kbd.iw;
        if(iw < kbd.ir){
            echo(kbd.ir, kbd.ie-kbd.ir);
            kbd.ir = kbd.istage;
        }
        if(kbd.ir != iw){
            echo(kbd.ir, iw-kbd.ir);
            kbd.ir = iw;
        }
    }
}
/*e: clock callback kbdputcclock */

/*s: devcons.c enum Qxxx */
enum{
    Qdir,

    Qcons,
    Qconsctl,

    /*s: devcons.c enum Qxxx cases */
        Qbintime,
        Qcputime,
        Qnull,
        Qpgrpid,
        Qpid,
        Qppid,
        Qrandom,
        Qswap,
        Qtime,
        Quser,
        Qzero,
    /*x: devcons.c enum Qxxx cases */
    Qkmesg,
    /*x: devcons.c enum Qxxx cases */
        Qkprint,
    /*e: devcons.c enum Qxxx cases */
};
/*e: devcons.c enum Qxxx */

enum
{
    /*s: constant VLNUMSIZE */
        VLNUMSIZE=  22,
    /*e: constant VLNUMSIZE */
};

/*s: global consdir */
static Dirtab consdir[]={
    ".",    {Qdir, 0, QTDIR},   0,      DMDIR|0555,

    "cons",     {Qcons},    0,      0660,
    "consctl",  {Qconsctl}, 0,      0220,

    /*s: [[consdir]] fields */
        "bintime",  {Qbintime}, 24,     0664,
        "cputime",  {Qcputime}, 6*NUMSIZE,  0444,
        "null",     {Qnull},    0,      0666,
        "pgrpid",   {Qpgrpid},  NUMSIZE,    0444,
        "pid",      {Qpid},     NUMSIZE,    0444,
        "ppid",     {Qppid},    NUMSIZE,    0444,
        "random",   {Qrandom},  0,      0444,
        "swap",     {Qswap},    0,      0664,
        "time",     {Qtime},    NUMSIZE+3*VLNUMSIZE,    0664,
        "user",     {Quser},    0,      0666,
        "zero",     {Qzero},    0,      0444,
    /*x: [[consdir]] fields */
    "kmesg",    {Qkmesg},   0,      0440,
    /*x: [[consdir]] fields */
        "kprint",   {Qkprint, 0, QTEXCL},   0,  DMEXCL|0440,
    /*e: [[consdir]] fields */
};
/*e: global consdir */

/*s: method consinit */
static void
consinit(void)
{
    /*s: [[consinit()]] initializing things */
        /*
         * at 115200 baud, the 1024 char buffer takes 56 ms to process,
         * processing it every 22 ms should be fine
         */
        addclock0link(kbdputcclock, 22);
    /*x: [[consinit()]] initializing things */
        todinit();
        randominit();
    /*e: [[consinit()]] initializing things */
}
/*e: method consinit */

static Chan*
consattach(char *spec)
{
    return devattach('c', spec);
}

static Walkqid*
conswalk(Chan *c, Chan *nc, char **name, int nname)
{
    return devwalk(c, nc, name,nname, consdir, nelem(consdir), devgen);
}

static int
consstat(Chan *c, uchar *dp, int n)
{
    return devstat(c, dp, n, consdir, nelem(consdir), devgen);
}

/*s: method consopen */
static Chan*
consopen(Chan *c, int omode)
{
    c->aux = nil;
    c = devopen(c, omode, consdir, nelem(consdir), devgen);

    switch((ulong)c->qid.path){
    /*s: [[consopen()]] cases */
    case Qconsctl:
        incref(&kbd.ctl);
        break;
    /*x: [[consopen()]] cases */
        case Qkprint:
            if(tas(&kprintinuse) != 0){
                c->flag &= ~COPEN;
                error(Einuse);
            }
            if(kprintoq == nil){
                kprintoq = qopen(8*1024, Qcoalesce, 0, 0);
                if(kprintoq == nil){
                    c->flag &= ~COPEN;
                    error(Enomem);
                }
                qnoblock(kprintoq, true);
            }else
                qreopen(kprintoq);
            c->iounit = qiomaxatomic;
            break;
    /*e: [[consopen()]] cases */
    }
    return c;
}
/*e: method consopen */

/*s: method consclose */
static void
consclose(Chan *c)
{
    switch((ulong)c->qid.path){
    /*s: [[consclose()]] cases */
    /* last close of control file turns off raw */
    case Qconsctl:
        if(c->flag&COPEN){
            if(decref(&kbd.ctl) == 0)
                kbd.raw = false;
        }
        break;
    /*x: [[consclose()]] cases */
        /* close of kprint allows other opens */
        case Qkprint:
            if(c->flag & COPEN){
                kprintinuse = 0;
                qhangup(kprintoq, nil);
            }
            break;
    /*e: [[consclose()]] cases */
    }
}
/*e: method consclose */

/*s: method consread */
static long
consread(Chan *c, void *buf, long n, vlong off)
{
    ulong l;
    char *b, *bp, ch;
    char tmp[256];      /* must be >= 18*NUMSIZE (Qswap) */
    int i, k, id, send;
    vlong offset = off;

    if(n <= 0)
        return n;

    switch((ulong)c->qid.path){
    case Qdir:
        return devdirread(c, buf, n, consdir, nelem(consdir), devgen);

    /*s: [[consread()]] Qcons case */
        case Qcons:
            qlock(&kbd);
            if(waserror()) {
                qunlock(&kbd);
                nexterror();
            }
            while(!qcanread(lineq)){
                if(qread(kbdq, &ch, 1) == 0)
                    continue;
                send = false;
                if(ch == 0){
                    /* flush output on rawoff -> rawon */
                    if(kbd.x > 0)
                        send = !qcanread(kbdq);
            
                /*s: [[consread()]] else if raw mode */
                }else if(kbd.raw){
                    kbd.line[kbd.x++] = ch;
                    send = !qcanread(kbdq);
                /*e: [[consread()]] else if raw mode */
                }else{
                    switch(ch){
                    case '\b':
                        if(kbd.x > 0)
                            kbd.x--;
                        break;
                    case 0x15:  /* ^U */
                        kbd.x = 0;
                        break;
                    case '\n':
                        send = true;
                        kbd.line[kbd.x++] = ch;
                        break;
                    case 0x04:  /* ^D */
                        send = true;
                        break;
                    default:
                        kbd.line[kbd.x++] = ch;
                        break;
                    }
                }
                if(send || kbd.x == sizeof kbd.line){
                    qwrite(lineq, kbd.line, kbd.x);
                    kbd.x = 0;
                }
            }
            n = qread(lineq, buf, n);
            qunlock(&kbd);
            poperror();
            return n;
    /*e: [[consread()]] Qcons case */

    /*s: [[consread()]] cases */
        case Qswap:
            snprint(tmp, sizeof tmp,
                "%lud memory\n"
                "%d pagesize\n"
                "%lud kernel\n"
                "%lud/%lud user\n"
                "%lud/%lud swap\n"
                "%lud/%lud kernel malloc\n"
                "%lud/%lud kernel draw\n",
                conf.npage*BY2PG,
                BY2PG,
                conf.npage-conf.upages,
                palloc.user-palloc.freecount, palloc.user,
                conf.nswap-swapalloc.free, conf.nswap,
                mainmem->cursize, mainmem->maxsize,
                imagmem->cursize, imagmem->maxsize);

            return readstr((ulong)offset, buf, n, tmp);
    /*x: [[consread()]] cases */
    case Qcputime:
        k = offset;
        if(k >= 5*NUMSIZE)
            return 0;
        if(k+n > 5*NUMSIZE)
            n = 5*NUMSIZE - k;
        /* easiest to format in a separate buffer and copy out */

        for(i=0; i<5 && NUMSIZE*i<k+n; i++){
            l = up->time[i];
            if(i == TReal)
                l = CPUS(0)->ticks - l;
            l = TK2MS(l);
            readnum(0, tmp+NUMSIZE*i, NUMSIZE, l, NUMSIZE);
        }
        memmove(buf, tmp+k, n);
        return n;
    /*x: [[consread()]] cases */
        case Qrandom:
            return randomread(buf, n);
    /*x: [[consread()]] cases */
    case Qkmesg:
        /*
         * This is unlocked to avoid tying up a process
         * that's writing to the buffer.  kmesg.n never 
         * gets smaller, so worst case the reader will
         * see a slurred buffer.
         */
        if(off >= kmesg.n)
            n = 0;
        else{
            if(off+n > kmesg.n)
                n = kmesg.n - off;
            memmove(buf, kmesg.buf+off, n);
        }
        return n;
    /*x: [[consread()]] cases */
        case Qkprint:
            return qread(kprintoq, buf, n);
    /*e: [[consread()]] cases */

    case Qtime:
        return readtime((ulong)offset, buf, n);

    case Qbintime:
        return readbintime(buf, n);
        


    case Quser:
        return readstr((ulong)offset, buf, n, up->user);

    case Qpid:
        return readnum((ulong)offset, buf, n, up->pid, NUMSIZE);

    case Qppid:
        return readnum((ulong)offset, buf, n, up->parentpid, NUMSIZE);

    case Qpgrpid:
        return readnum((ulong)offset, buf, n, up->pgrp->pgrpid, NUMSIZE);


    case Qnull:
        return 0;

    case Qzero:
        memset(buf, 0, n);
        return n;

    default:
        print("consread %#llux\n", c->qid.path);
        error(Egreg);
    }
    panic("consread: should not reach this point");
}
/*e: method consread */

/*s: method conswrite */
static long
conswrite(Chan *c, void *va, long n, vlong off)
{
    char *a;
    ulong offset;

    /*s: [[conswrite]] locals */
        char buf[256], ch;
        long l, bp;
        int id, fd;
        Chan *swc;
    /*e: [[conswrite]] locals */

    a = va;
    offset = off;

    switch((ulong)c->qid.path){

    /*s: [[conswrite()]] Qcons case */
        case Qcons:
            /*
             * Can't page fault in putstrn, so copy the data locally.
             */
            l = n;
            while(l > 0){
                bp = l;
                if(bp > sizeof buf)
                    bp = sizeof buf;
                memmove(buf, a, bp);
                putstrn0(buf, bp, 1);
                a += bp;
                l -= bp;
            }
            break;
    /*e: [[conswrite()]] Qcons case */

    /*s: [[conswrite()]] cases */
    case Qswap:
        if(n >= sizeof buf)
            error(Egreg);
        memmove(buf, va, n);    /* so we can NUL-terminate */
        buf[n] = 0;
        /* start a pager if not already started */
        if(strncmp(buf, "start", 5) == 0){
            kickpager();
            break;
        }
        if(!iseve())
            error(Eperm);
        if(buf[0]<'0' || '9'<buf[0])
            error(Ebadarg);
        fd = strtoul(buf, 0, 0);
        swc = fdtochan(fd, -1, true, true);
        setswapchan(swc);
        break;
    /*x: [[conswrite()]] cases */
    case Qconsctl:
        if(n >= sizeof(buf))
            n = sizeof(buf)-1;
        strncpy(buf, a, n);
        buf[n] = 0;
        for(a = buf; a;){
            if(strncmp(a, "rawon", 5) == 0){
                kbd.raw = true;
                /* clumsy hack - wake up reader */
                ch = 0;
                qwrite(kbdq, &ch, 1);           
            } else if(strncmp(a, "rawoff", 6) == 0){
                kbd.raw = false;
            }
            /*s: [[conswrite()]] Qconsctl other ifs */
            else if(strncmp(a, "ctlpon", 6) == 0){
                kbd.ctlpoff = false;
            } else if(strncmp(a, "ctlpoff", 7) == 0){
                kbd.ctlpoff = true;
            }
            /*e: [[conswrite()]] Qconsctl other ifs */
            if(a = strchr(a, ' '))
                a++;
        }
        break;
    /*e: [[conswrite()]] cases */

    case Qtime:
        if(!iseve())
            error(Eperm);
        return writetime(a, n);

    case Qbintime:
        if(!iseve())
            error(Eperm);
        return writebintime(a, n);


    case Quser:
        return userwrite(a, n);


    // > /dev/null :)
    case Qnull:
        break;

    default:
        print("conswrite: %#llux\n", c->qid.path);
        error(Egreg);
    }
    return n;
}
/*e: method conswrite */

/*s: global consdevtab */
Dev consdevtab = {
    .dc       =    'c',
    .name     =    "cons",
               
    .reset    =    devreset,
    .init     =    consinit,
    .shutdown =    devshutdown,
    .attach   =    consattach,
    .walk     =    conswalk,
    .stat     =    consstat,
    .open     =    consopen,
    .create   =    devcreate,
    .close    =    consclose,
    .read     =    consread,
    .bread    =    devbread,
    .write    =    conswrite,
    .bwrite   =    devbwrite,
    .remove   =    devremove,
    .wstat    =    devwstat,
};
/*e: global consdevtab */

/*s: global randn */
static  ulong   randn;
/*e: global randn */

/*s: function seedrand */
static void
seedrand(void)
{
    if(!waserror()){
        randomread((void*)&randn, sizeof(randn));
        poperror();
    }
}
/*e: function seedrand */

/*s: function nrand */
int
nrand(int n)
{
    if(randn == 0)
        seedrand();
    randn = randn*1103515245 + 12345 + CPUS(0)->ticks;
    return (randn>>16) % n;
}
/*e: function nrand */

/*s: global uvorder */
static uvlong uvorder = 0x0001020304050607ULL;
/*e: global uvorder */

/*s: function le2vlong */
static byte*
le2vlong(vlong *to, byte *f)
{
    byte *t, *o;
    int i;

    t = (byte*)to;
    o = (byte*)&uvorder;
    for(i = 0; i < sizeof(vlong); i++)
        t[o[i]] = f[i];
    return f+sizeof(vlong);
}
/*e: function le2vlong */

/*s: function vlong2le */
static byte*
vlong2le(byte *t, vlong from)
{
    byte *f, *o;
    int i;

    f = (byte*)&from;
    o = (byte*)&uvorder;
    for(i = 0; i < sizeof(vlong); i++)
        t[i] = f[o[i]];
    return t+sizeof(vlong);
}
/*e: function vlong2le */

/*s: global order */
static long order = 0x00010203;
/*e: global order */

/*s: function le2long */
static byte*
le2long(long *to, byte *f)
{
    byte *t, *o;
    int i;

    t = (byte*)to;
    o = (byte*)&order;
    for(i = 0; i < sizeof(long); i++)
        t[o[i]] = f[i];
    return f+sizeof(long);
}
/*e: function le2long */

/*s: devcons.c Exxx errors */
char *Ebadtimectl = "bad time control";
/*e: devcons.c Exxx errors */

/*s: function readtime */
/*
 *  like the old #c/time but with added info.  Return
 *
 *  secs    nanosecs    fastticks   fasthz
 */
static int
readtime(ulong off, char *buf, int n)
{
    vlong   nsec, ticks;
    long sec;
    char str[7*NUMSIZE];

    nsec = todget(&ticks);
    if(fasthz == 0LL)
        fastticks((uvlong*)&fasthz);
    sec = nsec/1000000000ULL;
    snprint(str, sizeof(str), "%*lud %*llud %*llud %*llud ",
        NUMSIZE-1, sec,
        VLNUMSIZE-1, nsec,
        VLNUMSIZE-1, ticks,
        VLNUMSIZE-1, fasthz);
    return readstr(off, buf, n, str);
}
/*e: function readtime */

/*s: function writetime */
/*
 *  set the time in seconds
 */
static int
writetime(char *buf, int n)
{
    char b[13];
    long i;
    vlong now;

    if(n >= sizeof(b))
        error(Ebadtimectl);
    strncpy(b, buf, n);
    b[n] = 0;
    i = strtol(b, 0, 0);
    if(i <= 0)
        error(Ebadtimectl);
    now = i*1000000000LL;
    todset(now, 0, 0);
    return n;
}
/*e: function writetime */

/*s: function readbintime */
/*
 *  read binary time info.  all numbers are little endian.
 *  ticks and nsec are syncronized.
 */
static int
readbintime(char *buf, int n)
{
    int i;
    vlong nsec, ticks;
    byte *b = (byte*)buf;

    i = 0;
    if(fasthz == 0LL)
        fastticks((uvlong*)&fasthz);
    nsec = todget(&ticks);
    if(n >= 3*sizeof(uvlong)){
        vlong2le(b+2*sizeof(uvlong), fasthz);
        i += sizeof(uvlong);
    }
    if(n >= 2*sizeof(uvlong)){
        vlong2le(b+sizeof(uvlong), ticks);
        i += sizeof(uvlong);
    }
    if(n >= 8){
        vlong2le(b, nsec);
        i += sizeof(vlong);
    }
    return i;
}
/*e: function readbintime */

/*s: function writebintime */
/*
 *  set any of the following
 *  - time in nsec
 *  - nsec trim applied over some seconds
 *  - clock frequency
 */
static int
writebintime(char *buf, int n)
{
    byte *p;
    vlong delta;
    long period;

    n--;
    p = (byte*)buf + 1;
    switch(*buf){
    case 'n':
        if(n < sizeof(vlong))
            error(Ebadtimectl);
        le2vlong(&delta, p);
        todset(delta, 0, 0);
        break;
    case 'd':
        if(n < sizeof(vlong)+sizeof(long))
            error(Ebadtimectl);
        p = le2vlong(&delta, p);
        le2long(&period, p);
        todset(-1, delta, period);
        break;
    case 'f':
        if(n < sizeof(uvlong))
            error(Ebadtimectl);
        le2vlong(&fasthz, p);
        if(fasthz <= 0)
            error(Ebadtimectl);
        todsetfreq(fasthz);
        break;
    }
    return n;
}
/*e: function writebintime */
/*e: devcons.c */
