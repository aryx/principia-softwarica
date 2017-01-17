/*s: lib_graphics/libdraw/event.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>
#include <cursor.h>
#include <event.h>

typedef struct	Slave Slave;
typedef struct	Ebuf Ebuf;

/*s: struct Slave */
struct Slave
{
    int	pid;

    Ebuf	*head;		/* queue of messages for this descriptor */
    Ebuf	*tail;

    int	(*fn)(int, Event*, uchar*, int);
};
/*e: struct Slave */

/*s: struct Ebuf */
struct Ebuf
{
    int	n;		/* number of bytes in buf */
    byte	buf[EMAXMSG];

    // Extra
    Ebuf	*next;
};
/*e: struct Ebuf */

/*s: global eslave */
static	Slave	eslave[MAXSLAVE];
/*e: global eslave */
/*s: global Skeyboard */
static	int	Skeyboard = -1;
/*e: global Skeyboard */
/*s: global Smouse */
static	int	Smouse = -1;
/*e: global Smouse */
/*s: global Stimer */
static	int	Stimer = -1;
/*e: global Stimer */
/*s: global logfid */
static	fdt	logfid;
/*e: global logfid */

/*s: global nslave */
static	int	nslave;
/*e: global nslave */
/*s: global parentpid */
static	int	parentpid;
/*e: global parentpid */
/*s: global epipe */
static	fdt	epipe[2];
/*e: global epipe */

static	int	eforkslave(ulong);
static	void	extract(void);
static	void	ekill(void);
static	int	enote(void *, char *);

/*s: global mousefd */
static	fdt	mousefd;
/*e: global mousefd */
/*s: global cursorfd */
static	fdt	cursorfd;
/*e: global cursorfd */

/*s: function ebread */
static
Ebuf*
ebread(Slave *s)
{
    Ebuf *eb;
    Dir *d;
    ulong l;

    for(;;){
        d = dirfstat(epipe[0]);
        if(d == nil)
            drawerror(display, "events: eread stat error");
        l = d->length;
        free(d);
        if(s->head && l==0)
            break;
        // else
        extract();
    }
    // eb = pop_queue(s->head, s->tail);
    eb = s->head;
    s->head = s->head->next;
    if(s->head == nil)
        s->tail = nil;

    return eb;
}
/*e: function ebread */

/*s: function event */
keys
event(Event *e)
{
    return eread(~0UL, e);
}
/*e: function event */

/*s: function eread */
keys
eread(keys keys, Event *e)
{
    Ebuf *eb;
    int i, id;

    if(keys == 0)
        return 0;
    for(;;){
        // check if pending message
        for(i=0; i<nslave; i++) {
            if((keys & (1<<i)) && eslave[i].head){
                id = 1<<i;
                if(i == Smouse)
                    e->mouse = emouse();
                else if(i == Skeyboard)
                    e->kbdc = ekbd();
                else if(i == Stimer)
                    eslave[i].head = nil;
                else{
                    eb = ebread(&eslave[i]);
                    e->n = eb->n;
                    if(eslave[i].fn)
                        id = (*eslave[i].fn)(id, e, eb->buf, eb->n);
                    else
                        memmove(e->data, eb->buf, eb->n);
                    free(eb);
                }
                return id;
            }
        }
        // read from pipe new messages from the slaves
        extract();
    }
    return 0; // unreachable
}
/*e: function eread */

/*s: function ecanmouse */
int
ecanmouse(void)
{
    if(Smouse < 0)
        drawerror(display, "events: mouse not initialized");
    return ecanread(Emouse);
}
/*e: function ecanmouse */

/*s: function ecankbd */
int
ecankbd(void)
{
    if(Skeyboard < 0)
        drawerror(display, "events: keyboard not initialzed");
    return ecanread(Ekeyboard);
}
/*e: function ecankbd */

/*s: function ecanread */
int
ecanread(ulong keys)
{
    Dir *d;
    int i;
    ulong l;

    for(;;){
        for(i=0; i<nslave; i++)
            if((keys & (1<<i)) && eslave[i].head)
                return 1;
        d = dirfstat(epipe[0]);
        if(d == nil)
            drawerror(display, "events: ecanread stat error");
        l = d->length;
        free(d);
        if(l == 0)
            return 0;
        extract();
    }
    return 0; // unreachable
}
/*e: function ecanread */

/*s: function estartfn */
ulong
estartfn(ulong key, fdt fd, int n, int (*fn)(int, Event*, uchar*, int))
{
    char buf[EMAXMSG+1];
    int i, r;

    if(fd < 0)
        drawerror(display, "events: bad file descriptor");
    if(n <= 0 || n > EMAXMSG)
        n = EMAXMSG;

    i = eforkslave(key);
    // master
    if(i < MAXSLAVE){
        eslave[i].fn = fn;
        return 1<<i;
    }
    // slave
    buf[0] = i - MAXSLAVE; // slave identifier
    while((r = read(fd, buf+1, n))>0)
        if(write(epipe[1], buf, r+1)!=r+1)
            break;

    buf[0] = MAXSLAVE;
    write(epipe[1], buf, 1);
    _exits(nil);
    return 0;
}
/*e: function estartfn */

/*s: function estart */
ulong
estart(ulong key, fdt fd, int n)
{
    return estartfn(key, fd, n, nil);
}
/*e: function estart */

/*s: function etimer */
ulong
etimer(ulong key, int n)
{
    char t[2];
    int cnt;

    if(Stimer != -1)
        drawerror(display, "events: timer started twice");
    Stimer = eforkslave(key);
    // master
    if(Stimer < MAXSLAVE)
        return 1<<Stimer;
    // slave
    if(n <= 0)
        n = 1000;
    t[0] = t[1] = Stimer - MAXSLAVE;
    do {
        sleep(n);
        cnt = write(epipe[1], t, 2);
    } while(cnt == 2);

    t[0] = MAXSLAVE;
    write(epipe[1], t, 1);
    _exits(nil);
    return 0;
}
/*e: function etimer */

/*s: function ekeyslave */
static void
ekeyslave(fdt fd)
{
    Rune r;
    char t[3], k[10];
    int kr, kn, w;

    if(eforkslave(Ekeyboard) < MAXSLAVE)
        // parent (master)
        return;

    // child (slave) code
    kn = 0;
    t[0] = Skeyboard;
    for(;;){
        while(!fullrune(k, kn)){
            // blocking call
            kr = read(fd, k+kn, sizeof k - kn);
            if(kr <= 0)
                goto breakout;
            kn += kr;
        }
        w = chartorune(&r, k);
        kn -= w;
        memmove(k, &k[w], kn);
        t[1] = r;
        t[2] = r>>8; // TODO what about other parts of the Rune????
        // send to master
        if(write(epipe[1], t, 3) != 3)
            break;
    }
breakout:;
    t[0] = MAXSLAVE;
    write(epipe[1], t, 1);
    _exits(0);
}
/*e: function ekeyslave */

/*s: function einit */
void
einit(keys keys)
{
    fdt ctl, fd;
    char buf[256];

    parentpid = getpid();
    if(pipe(epipe) < 0)
        drawerror(display, "events: einit pipe");

    atexit(ekill);
    atnotify(enote, 1);

    snprint(buf, sizeof buf, "%s/mouse", display->devdir);
    mousefd = open(buf, ORDWR|OCEXEC);
    if(mousefd < 0)
        drawerror(display, "einit: can't open mouse\n");

    snprint(buf, sizeof buf, "%s/cursor", display->devdir);
    cursorfd = open(buf, ORDWR|OCEXEC);
    if(cursorfd < 0)
        drawerror(display, "einit: can't open cursor\n");

    if(keys&Ekeyboard){
        snprint(buf, sizeof buf, "%s/cons", display->devdir);
        fd = open(buf, OREAD);
        if(fd < 0)
            drawerror(display, "events: can't open console");

        snprint(buf, sizeof buf, "%s/consctl", display->devdir);
        ctl = open("/dev/consctl", OWRITE|OCEXEC);
        if(ctl < 0)
            drawerror(display, "events: can't open consctl");
        write(ctl, "rawon", 5);

        for(Skeyboard=0; Ekeyboard & ~(1<<Skeyboard); Skeyboard++)
            ;
        ekeyslave(fd);
    }
    if(keys&Emouse){
        estart(Emouse, mousefd, 1+4*12);
        for(Smouse=0; Emouse & ~(1<<Smouse); Smouse++)
            ;
    }
}
/*e: function einit */

/*s: function extract */
static void
extract(void)
{
    Slave *s;
    Ebuf *eb;
    int i, n;
    byte ebuf[EMAXMSG+1];

    /* avoid generating a message if there's nothing to show. */
    /* this test isn't perfect, though; could do flushimage(display, 0) then call extract */
    /* also: make sure we don't interfere if we're multiprocessing the display */
    if(display->locking){
        /* if locking is being done by program, this means it can't depend on automatic flush in emouse() etc. */
        if(canqlock(&display->qlock)){
            if(display->bufp > display->buf)
                flushimage(display, true);
            unlockdisplay(display);
        }
    }else
        if(display->bufp > display->buf)
            flushimage(display, true);

loop:
    if((n=read(epipe[0], ebuf, EMAXMSG+1)) < 0
    || ebuf[0] >= MAXSLAVE)
        drawerror(display, "eof on event pipe");
    if(n == 0)
        goto loop;

    i = ebuf[0];
    if(i >= nslave || n <= 1)
        drawerror(display, "events: protocol error: short read");
    s = &eslave[i];

    if(i == Stimer){
        s->head = (Ebuf *)1;
        return;
    }
    if(i == Skeyboard && n != 3)
        drawerror(display, "events: protocol error: keyboard");
    if(i == Smouse){
        if(n < 1+1+2*12)
            drawerror(display, "events: protocol error: mouse");
        if(ebuf[1] == 'r')
            eresized(1);

        /* squash extraneous mouse events */
        if((eb=s->tail) && memcmp(eb->buf+1+2*12, ebuf+1+1+2*12, 12)==0){
            memmove(eb->buf, &ebuf[1], n - 1);
            return;
        }
    }

    /* try to save space by only allocating as much buffer as we need */
    eb = malloc(sizeof(*eb) - sizeof(eb->buf) + n - 1);
    if(eb == nil)
        drawerror(display, "events: protocol error 4");
    eb->n = n - 1;
    memmove(eb->buf, &ebuf[1], n - 1);

    // add_queue(eb, s->head, s->tail)
    eb->next = nil;
    if(s->head)
        s->tail = s->tail->next = eb;
    else
        s->head = s->tail = eb;
}
/*e: function extract */

/*s: function eforkslave */
static int
eforkslave(ulong key)
{
    int i, pid;

    for(i=0; i<MAXSLAVE; i++)
        if((key & ~(1<<i)) == 0 && eslave[i].pid == 0){
            if(nslave <= i)
                nslave = i + 1;
            /*
             * share the file descriptors so the last child
             * out closes all connections to the window server.
             */
            switch(pid = rfork(RFPROC)){
            // child
            case 0:
                return MAXSLAVE+i;
            case -1:
                fprint(2, "events: fork error\n");
                exits("fork");
            }
            // parent
            eslave[i].pid = pid;
            eslave[i].head = eslave[i].tail = nil;
            return i;
        }
    drawerror(display, "events: bad slave assignment");
    return 0;
}
/*e: function eforkslave */

/*s: function enote */
static int
enote(void *v, char *s)
{
    char t[1];
    int i, pid;

    USED(v, s);
    pid = getpid();
    if(pid != parentpid){
        for(i=0; i<nslave; i++){
            if(pid == eslave[i].pid){
                t[0] = MAXSLAVE;
                write(epipe[1], t, 1);
                break;
            }
        }
        return 0;
    }
    close(epipe[0]);
    epipe[0] = -1;
    close(epipe[1]);
    epipe[1] = -1;
    for(i=0; i<nslave; i++){
        if(pid == eslave[i].pid)
            continue;	/* don't kill myself */
        postnote(PNPROC, eslave[i].pid, "die");
    }
    return 0;
}
/*e: function enote */

/*s: function ekill */
static void
ekill(void)
{
    enote(0, 0);
}
/*e: function ekill */

/*s: function emouse */
Mouse
emouse(void)
{
    Mouse m;
    Ebuf *eb;
    static int but[2];
    int b;

    if(Smouse < 0)
        drawerror(display, "events: mouse not initialized");
    eb = ebread(&eslave[Smouse]);
    m.xy.x = atoi((char*)eb->buf+1+0*12);
    m.xy.y = atoi((char*)eb->buf+1+1*12);
    b = atoi((char*)eb->buf+1+2*12);
    m.buttons = b;
    m.msec = atoi((char*)eb->buf+1+3*12);
    if (logfid)
        fprint(logfid, "b: %d xy: %P\n", m.buttons, m.xy);
    free(eb);
    return m;
}
/*e: function emouse */

/*s: function ekbd */
int
ekbd(void)
{
    Ebuf *eb;
    int c;

    if(Skeyboard < 0)
        drawerror(display, "events: keyboard not initialzed");
    eb = ebread(&eslave[Skeyboard]);
    c = eb->buf[0] + (eb->buf[1]<<8);
    free(eb);
    return c;
}
/*e: function ekbd */

/*s: function emoveto */
void
emoveto(Point pt)
{
    char buf[2*12+2];
    int n;

    n = sprint(buf, "m%d %d", pt.x, pt.y);
    write(mousefd, buf, n);
}
/*e: function emoveto */

/*s: function esetcursor */
void
esetcursor(Cursor *c)
{
    uchar curs[2*4+2*2*16];

    if(c == 0)
        write(cursorfd, curs, 0);
    else{
        BPLONG(curs+0*4, c->offset.x);
        BPLONG(curs+1*4, c->offset.y);
        memmove(curs+2*4, c->clr, 2*2*16);
        write(cursorfd, curs, sizeof curs);
    }
}
/*e: function esetcursor */

/*s: function ereadmouse */
int
ereadmouse(Mouse *m)
{
    int n;
    char buf[128];

    do{
        n = read(mousefd, buf, sizeof(buf));
        if(n < 0)	/* probably interrupted */
            return -1;
        n = eatomouse(m, buf, n);
    }while(n == 0);
    return n;
}
/*e: function ereadmouse */

/*s: function eatomouse */
int
eatomouse(Mouse *m, char *buf, int n)
{
    if(n != 1+4*12){
        werrstr("atomouse: bad count");
        return -1;
    }

    if(buf[0] == 'r')
        eresized(1);
    m->xy.x = atoi(buf+1+0*12);
    m->xy.y = atoi(buf+1+1*12);
    m->buttons = atoi(buf+1+2*12);
    m->msec = atoi(buf+1+3*12);
    return n;
}
/*e: function eatomouse */
/*e: lib_graphics/libdraw/event.c */
