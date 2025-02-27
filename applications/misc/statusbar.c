/*s: windows/apps/statusbar.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <window.h>
#include <bio.h>
#include <event.h>

/*s: enum [[_anon_ (windows/apps/statusbar.c)]] */
enum {PNCTL=3};
/*e: enum [[_anon_ (windows/apps/statusbar.c)]] */

static char* rdenv(char*);
int newwin(char*);
Rectangle screenrect(void);

/*s: global [[nokill]] */
static int nokill;
/*e: global [[nokill]] */
/*s: global [[textmode]] */
static int textmode;
/*e: global [[textmode]] */
/*s: global [[title]] */
static char *title;
/*e: global [[title]] */

/*s: global [[light]] */
static Image *light;
/*e: global [[light]] */
/*s: global [[dark]] */
static Image *dark;
/*e: global [[dark]] */
/*s: global text (windows/apps/statusbar.c) */
static Image *text;
/*e: global text (windows/apps/statusbar.c) */

/*s: function [[initcolor]] */
static void
initcolor(void)
{
    text = display->black;
    light = allocimagemix(display, DPalegreen, DWhite);
    dark = allocimage(display, Rect(0,0,1,1), CMAP8, 1, DDarkgreen);
}
/*e: function [[initcolor]] */

/*s: global [[rbar]] */
static Rectangle rbar;
/*e: global [[rbar]] */
/*s: global [[ptext]] */
static Point ptext;
/*e: global [[ptext]] */
static vlong n, d;
/*s: global [[last]] */
static int last;
/*e: global [[last]] */
/*s: global lastp (windows/apps/statusbar.c) */
static int lastp = -1;
/*e: global lastp (windows/apps/statusbar.c) */

/*s: global [[backup]] */
static char backup[80];
/*e: global [[backup]] */

/*s: function [[drawbar]] */
void
drawbar(void)
{
    int i, j;
    int p;
    char buf[400], bar[200];
    static char lastbar[200];

    if(n > d || n < 0 || d <= 0)
        return;

    i = (Dx(rbar)*n)/d;
    p = (n*100LL)/d;

    if(textmode){
        if(Dx(rbar) > 150){
            rbar.min.x = 0;
            rbar.max.x = 150;
            return;
        }
        bar[0] = '|';
        for(j=0; j<i; j++)
            bar[j+1] = '#';
        for(; j<Dx(rbar); j++)
            bar[j+1] = '-';
        bar[j++] = '|';
        bar[j++] = ' ';
        sprint(bar+j, "%3d%% ", p);
        for(i=0; bar[i]==lastbar[i] && bar[i]; i++)
            ;
        memset(buf, '\b', strlen(lastbar)-i);
        strcpy(buf+strlen(lastbar)-i, bar+i);
        if(buf[0])
            write(1, buf, strlen(buf));
        strcpy(lastbar, bar);
        return;
    }

    if(lastp == p && last == i)
        return;

    if(lastp != p){
        sprint(buf, "%d%%", p);
        
        stringbg(view, addpt(view->r.min, Pt(Dx(rbar)-30, 4)), text, ZP, display->defaultfont, buf, light, ZP);
        lastp = p;
    }

    if(last != i){
        if(i > last)
            draw(view, Rect(rbar.min.x+last, rbar.min.y, rbar.min.x+i, rbar.max.y),
                dark, nil, ZP);
        else
            draw(view, Rect(rbar.min.x+i, rbar.min.y, rbar.min.x+last, rbar.max.y),
                light, nil, ZP);
        last = i;
    }
    flushimage(display, 1);
}
/*e: function [[drawbar]] */

/*s: function eresized (windows/apps/statusbar.c) */
void
eresized(int new)
{
    Point p, q;
    Rectangle r;

    if(new && getwindow(display, Refnone) < 0)
        fprint(2,"can't reattach to window");

    r = view->r;
    draw(view, r, light, nil, ZP);
    p = string(view, addpt(r.min, Pt(4,4)), text, ZP,
        display->defaultfont, title);

    p.x = r.min.x+4;
    p.y += display->defaultfont->height+4;

    q = subpt(r.max, Pt(4,4));
    rbar = Rpt(p, q);

    ptext = Pt(r.max.x-4-stringwidth(display->defaultfont, "100%"), r.min.x+4);
    border(view, rbar, -2, dark, ZP);
    last = 0;
    lastp = -1;

    drawbar();
}
/*e: function eresized (windows/apps/statusbar.c) */

/*s: function [[bar]] */
void
bar(Biobuf *b)
{
    char *p, *f[2];
    Event e;
    int k, die, parent, child;

    parent = getpid();

    die = 0;
    if(textmode)
        child = -1;
    else
    switch(child = rfork(RFMEM|RFPROC)) {
    case 0:
        sleep(1000);
        while(!die && (k = eread(Ekeyboard|Emouse, &e))) {
            if(nokill==0 && k == Ekeyboard && (e.kbdc == 0x7F || e.kbdc == 0x03)) { /* del, ctl-c */
                die = 1;
                postnote(PNPROC, parent, "interrupt");
                _exits("interrupt");
            }
        }
        _exits(0);
    }

    while(!die && (p = Brdline(b, '\n'))) {
        p[Blinelen(b)-1] = '\0';
        if(tokenize(p, f, 2) != 2)
            continue;
        n = strtoll(f[0], 0, 0);
        d = strtoll(f[1], 0, 0);
        drawbar();
    }
    postnote(PNCTL, child, "kill");
}
/*e: function [[bar]] */


/*s: function usage (windows/apps/statusbar.c) */
static void
usage(void)
{
    fprint(2, "usage: aux/statusbar [-kt] [-w minx,miny,maxx,maxy] 'title'\n");
    exits("usage");
}
/*e: function usage (windows/apps/statusbar.c) */

/*s: function main (windows/apps/statusbar.c) */
void
main(int argc, char **argv)
{
    Biobuf b;
    char *p, *q;
    int lfd;

    p = "0,0,200,60";
    
    ARGBEGIN{
    case 'w':
        p = ARGF();
        break;
    case 't':
        textmode = 1;
        break;
    case 'k':
        nokill = 1;
        break;
    default:
        usage();
    }ARGEND;

    if(argc != 1)
        usage();

    title = argv[0];

    lfd = dup(0, -1);

    while(q = strchr(p, ','))
        *q = ' ';
    Binit(&b, lfd, OREAD);
    if(textmode || newwin(p) < 0){
        textmode = 1;
        rbar = Rect(0, 0, 60, 1);
    }else{
        if(initdraw(0, 0, "bar") < 0)
            exits("initdraw");
        initcolor();
        einit(Emouse|Ekeyboard);
        eresized(0);
    }
    bar(&b);

    exits(0);
}
/*e: function main (windows/apps/statusbar.c) */


/*s: function [[rdenv]] */
/* all code below this line should be in the library, but is stolen from colors instead */
static char*
rdenv(char *name)
{
    char *v;
    int fd, size;

    fd = open(name, OREAD);
    if(fd < 0)
        return 0;
    size = seek(fd, 0, 2);
    v = malloc(size+1);
    if(v == 0){
        fprint(2, "%s: can't malloc: %r\n", argv0);
        exits("no mem");
    }
    seek(fd, 0, 0);
    read(fd, v, size);
    v[size] = 0;
    close(fd);
    return v;
}
/*e: function [[rdenv]] */

/*s: function [[newwin]] */
int
newwin(char *win)
{
    char *srv, *mntsrv;
    char spec[100];
    int srvfd, cons, pid;

    switch(rfork(RFFDG|RFPROC|RFNAMEG|RFENVG|RFNOTEG|RFNOWAIT)){
    case -1:
        fprint(2, "statusbar: can't fork: %r\n");
        return -1;
    case 0:
        break;
    default:
        exits(0);
    }

    srv = rdenv("/env/wsys");
    if(srv == 0){
        mntsrv = rdenv("/mnt/term/env/wsys");
        if(mntsrv == 0){
            fprint(2, "statusbar: can't find $wsys\n"); //$
            return -1;
        }
        srv = malloc(strlen(mntsrv)+10);
        sprint(srv, "/mnt/term%s", mntsrv);
        free(mntsrv);
        pid  = 0;			/* can't send notes to remote processes! */
    }else
        pid = getpid();
    USED(pid);
    srvfd = open(srv, ORDWR);
    free(srv);
    if(srvfd == -1){
        fprint(2, "statusbar: can't open %s: %r\n", srv);
        return -1;
    }
    sprint(spec, "new -r %s", win);
    if(mount(srvfd, -1, "/mnt/wsys", 0, spec) == -1){
        fprint(2, "statusbar: can't mount /mnt/wsys: %r (spec=%s)\n", spec);
        return -1;
    }
    close(srvfd);
    unmount("/mnt/acme", "/dev");
    bind("/mnt/wsys", "/dev", MBEFORE);
    cons = open("/dev/cons", OREAD);
    if(cons==-1){
    NoCons:
        fprint(2, "statusbar: can't open /dev/cons: %r");
        return -1;
    }
    dup(cons, 0);
    close(cons);
    cons = open("/dev/cons", OWRITE);
    if(cons==-1)
        goto NoCons;
    dup(cons, 1);
    dup(cons, 2);
    close(cons);
//	wctlfd = open("/dev/wctl", OWRITE);
    return 0;
}
/*e: function [[newwin]] */

/*s: function [[screenrect]] */
Rectangle
screenrect(void)
{
    int fd;
    char buf[12*5];

    fd = open("/dev/screen", OREAD);
    if(fd == -1)
        fd=open("/mnt/term/dev/screen", OREAD);
    if(fd == -1){
        fprint(2, "%s: can't open /dev/screen: %r\n", argv0);
        exits("window read");
    }
    if(read(fd, buf, sizeof buf) != sizeof buf){
        fprint(2, "%s: can't read /dev/screen: %r\n", argv0);
        exits("screen read");
    }
    close(fd);
    return Rect(atoi(buf+12), atoi(buf+24), atoi(buf+36), atoi(buf+48));
}
/*e: function [[screenrect]] */

/*s: function [[postnote]] */
int
postnote(int group, int pid, char *note)
{
    char file[128];
    int f, r;

    switch(group) {
    case PNPROC:
        sprint(file, "/proc/%d/note", pid);
        break;
    case PNGROUP:
        sprint(file, "/proc/%d/notepg", pid);
        break;
    case PNCTL:
        sprint(file, "/proc/%d/ctl", pid);
        break;
    default:
        return -1;
    }

    f = open(file, OWRITE);
    if(f < 0)
        return -1;

    r = strlen(note);
    if(write(f, note, r) != r) {
        close(f);
        return -1;
    }
    close(f);
    return 0;
}
/*e: function [[postnote]] */
/*e: windows/apps/statusbar.c */
