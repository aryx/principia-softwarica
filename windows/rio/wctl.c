/*s: windows/rio/wctl.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <window.h>
#include <thread.h>
#include <cursor.h>
#include <mouse.h>
#include <keyboard.h>
#include <frame.h>
#include <fcall.h>
#include <plumb.h>

#include "dat.h"
#include "fns.h"

#include <ctype.h>

/*s: global [[Ebadwr]] */
char	Ebadwr[]		= "bad rectangle in wctl request";
/*e: global [[Ebadwr]] */
/*s: global [[Ewalloc]] */
char	Ewalloc[]		= "window allocation failed in wctl request";
/*e: global [[Ewalloc]] */

/*s: enum [[_anon_ (windows/rio/wctl.c)]] */
/* >= Top are disallowed if mouse button is pressed */
enum
{
    New,
    Resize,
    Move,
    Scroll,
    Noscroll,
    Set,
    Top,
    Bottom,
    Current,
    Hide,
    Unhide,
    Delete,
};
/*e: enum [[_anon_ (windows/rio/wctl.c)]] */

/*s: global [[cmds]] */
static char *cmds[] = {
    [New]	= "new",
    [Resize]	= "resize",
    [Move]	= "move",
    [Scroll]	= "scroll",
    [Noscroll]	= "noscroll",
    [Set]	= "set",
    [Top]	= "top",
    [Bottom]	= "bottom",
    [Current]	= "current",
    [Hide]	= "hide",
    [Unhide]	= "unhide",
    [Delete]	= "delete",
    nil
};
/*e: global [[cmds]] */

/*s: enum [[_anon_ (windows/rio/wctl.c)]]2 */
enum
{
    Cd,
    Deltax,
    Deltay,
    Hidden,
    Id,

    Maxx,
    Maxy,
    Minx,
    Miny,

    PID,
    R,

    Scrolling,
    Noscrolling,
};
/*e: enum [[_anon_ (windows/rio/wctl.c)]]2 */

/*s: global [[params]] */
static char *params[] = {
    [Cd] 			= "-cd",
    [Deltax]			= "-dx",
    [Deltay]			= "-dy",
    [Hidden]			= "-hide",
    [Id]			= "-id",
    [Maxx]			= "-maxx",
    [Maxy]			= "-maxy",
    [Minx]			= "-minx",
    [Miny]			= "-miny",
    [PID]			= "-pid",
    [R]				= "-r",
    [Scrolling]			= "-scroll",
    [Noscrolling]		= "-noscroll",
    nil
};
/*e: global [[params]] */


/*s: function [[word]] */
static
int
word(char **sp, char *tab[])
{
    char *s, *t;
    int i;

    s = *sp;
    while(isspace(*s))
        s++;
    t = s;
    while(*s!='\0' && !isspace(*s))
        s++;
    for(i=0; tab[i]!=nil; i++)
        if(strncmp(tab[i], t, strlen(tab[i])) == 0){
            *sp = s;
            return i;
    }
    return -1;
}
/*e: function [[word]] */

/*s: function [[set]] */
int
set(int sign, int neg, int abs, int pos)
{
    if(sign < 0)
        return neg;
    if(sign > 0)
        return pos;
    return abs;
}
/*e: function [[set]] */

/*s: function [[newrect]] */
Rectangle
newrect(void)
{
    static int i = 0;
    int minx, miny, dx, dy;

    dx = min(600, Dx(view->r) - 2*Borderwidth);
    dy = min(400, Dy(view->r) - 2*Borderwidth);
    minx = 32 + 16*i;
    miny = 32 + 16*i;
    i++;
    i %= 10;

    return Rect(minx, miny, minx+dx, miny+dy);
}
/*e: function [[newrect]] */

/*s: function [[shift]] */
void
shift(int *minp, int *maxp, int min, int max)
{
    if(*minp < min){
        *maxp += min-*minp;
        *minp = min;
    }
    if(*maxp > max){
        *minp += max-*maxp;
        *maxp = max;
    }
}
/*e: function [[shift]] */

/*s: function [[rectonscreen]] */
Rectangle
rectonscreen(Rectangle r)
{
    shift(&r.min.x, &r.max.x, view->r.min.x, view->r.max.x);
    shift(&r.min.y, &r.max.y, view->r.min.y, view->r.max.y);
    return r;
}
/*e: function [[rectonscreen]] */

/*s: function [[riostrtol]] */
/* permit square brackets, in the manner of %R */
int
riostrtol(char *s, char **t)
{
    int n;

    while(*s!='\0' && (*s==' ' || *s=='\t' || *s=='['))
        s++;
    if(*s == '[')
        s++;
    n = strtol(s, t, 10);
    if(*t != s)
        while((*t)[0] == ']')
            (*t)++;
    return n;
}
/*e: function [[riostrtol]] */


/*s: function [[parsewctl]] */
int
parsewctl(char **argp, Rectangle r, Rectangle *rp, int *pidp, int *idp, int *hiddenp, int *scrollingp, char **cdp, char *s, char *err)
{
    int cmd, param, xy, sign;
    char *t;

    *pidp = 0;
    *hiddenp = 0;
    *scrollingp = scrolling;
    *cdp = nil;
    cmd = word(&s, cmds);
    if(cmd < 0){
        strcpy(err, "unrecognized wctl command");
        return -1;
    }
    if(cmd == New)
        r = newrect();

    strcpy(err, "missing or bad wctl parameter");

    while((param = word(&s, params)) >= 0){
        switch(param){	/* special cases */
        case Hidden:
            *hiddenp = 1;
            continue;
        case Scrolling:
            *scrollingp = 1;
            continue;
        case Noscrolling:
            *scrollingp = 0;
            continue;
        case R:
            r.min.x = riostrtol(s, &t);
            if(t == s)
                return -1;
            s = t;
            r.min.y = riostrtol(s, &t);
            if(t == s)
                return -1;
            s = t;
            r.max.x = riostrtol(s, &t);
            if(t == s)
                return -1;
            s = t;
            r.max.y = riostrtol(s, &t);
            if(t == s)
                return -1;
            s = t;
            continue;
        }
        while(isspace(*s))
            s++;
        if(param == Cd){
            *cdp = s;
            while(*s && !isspace(*s))
                s++;
            if(*s != '\0')
                *s++ = '\0';
            continue;
        }
        sign = 0;
        if(*s == '-'){
            sign = -1;
            s++;
        }else if(*s == '+'){
            sign = +1;
            s++;
        }
        if(!isdigit(*s))
            return -1;
        xy = riostrtol(s, &s);

        switch(param){
        case Minx:
            r.min.x = set(sign, r.min.x-xy, xy, r.min.x+xy);
            break;
        case Miny:
            r.min.y = set(sign, r.min.y-xy, xy, r.min.y+xy);
            break;
        case Maxx:
            r.max.x = set(sign, r.max.x-xy, xy, r.max.x+xy);
            break;
        case Maxy:
            r.max.y = set(sign, r.max.y-xy, xy, r.max.y+xy);
            break;
        case Deltax:
            r.max.x = set(sign, r.max.x-xy, r.min.x+xy, r.max.x+xy);
            break;
        case Deltay:
            r.max.y = set(sign, r.max.y-xy, r.min.y+xy, r.max.y+xy);
            break;
        case Id:
            if(idp != nil)
                *idp = xy;
            break;
        case PID:
            if(pidp != nil)
                *pidp = xy;
            break;
        case -1:
            strcpy(err, "unrecognized wctl parameter");
            return -1;
        }
    }

    *rp = rectonscreen(rectaddpt(r, view->r.min));

    while(isspace(*s))
        s++;
    if(cmd!=New && *s!='\0'){
        strcpy(err, "extraneous text in wctl message");
        return -1;
    }

    if(argp)
        *argp = s;

    return cmd;
}
/*e: function [[parsewctl]] */

/*s: function [[wctlnew]] */
int
wctlnew(Rectangle rect, char *arg, int pid, int hideit, int scrollit, char *dir, char *err)
{
    char **argv;
    Image *i;

    if(!goodrect(rect)){
        strcpy(err, Ebadwr);
        return -1;
    }
    argv = emalloc(4*sizeof(char*));
    argv[0] = "rc";
    argv[1] = "-c";
    while(isspace(*arg))
        arg++;
    if(*arg == '\0'){
        argv[1] = "-i";
        argv[2] = nil;
    }else{
        argv[2] = arg;
        argv[3] = nil;
    }
    if(hideit)
        i = allocimage(display, rect, view->chan, false, DWhite);
    else
        i = allocwindow(desktop, rect, Refbackup, DWhite);
    if(i == nil){
        strcpy(err, Ewalloc);
        return -1;
    }
    border(i, rect, Selborder, red, ZP);

    new(i, hideit, scrollit, pid, dir, "/bin/rc", argv);

    free(argv);	/* when new() returns, argv and args have been copied */
    return 1;
}
/*e: function [[wctlnew]] */

/*s: function [[writewctl]] */
int
writewctl(Xfid *x, char *err)
{
    int cnt, cmd, j, id, hideit, scrollit, pid;
    Image *i;
    char *arg, *dir;
    Rectangle rect;
    Window *w;

    w = x->f->w;
    cnt = x->req.count;
    x->req.data[cnt] = '\0';
    id = 0;

    rect = rectsubpt(w->screenr, view->r.min);
    cmd = parsewctl(&arg, rect, &rect, &pid, &id, &hideit, &scrollit, &dir, x->req.data, err);
    if(cmd < 0)
        return -1;

    if(mouse->buttons!=0 && cmd>=Top){
        strcpy(err, "action disallowed when mouse active");
        return -1;
    }

    if(id != 0){
        for(j=0; j<nwindow; j++)
            if(windows[j]->id == id)
                break;
        if(j == nwindow){
            strcpy(err, "no such window id");
            return -1;
        }
        w = windows[j];
        if(w->deleted || w->i==nil){
            strcpy(err, "window deleted");
            return -1;
        }
    }

    switch(cmd){
    case New:
        return wctlnew(rect, arg, pid, hideit, scrollit, dir, err);
    case Set:
        if(pid > 0)
            wsetpid(w, pid, 0);
        return 1;
    case Move:
        rect = Rect(rect.min.x, rect.min.y, rect.min.x+Dx(w->screenr), rect.min.y+Dy(w->screenr));
        rect = rectonscreen(rect);
        /* fall through */
    case Resize:
        if(!goodrect(rect)){
            strcpy(err, Ebadwr);
            return -1;
        }
        if(eqrect(rect, w->screenr))
            return 1;
        i = allocwindow(desktop, rect, Refbackup, DWhite);
        if(i == nil){
            strcpy(err, Ewalloc);
            return -1;
        }
        border(i, rect, Selborder, red, ZP);
        wsendctlmesg(w, Reshaped, i->r, i);
        return 1;
    case Scroll:
        w->scrolling = 1;
        wshow(w, w->nr);
        wsendctlmesg(w, Wakeup, ZR, nil);
        return 1;
    case Noscroll:
        w->scrolling = 0;
        wsendctlmesg(w, Wakeup, ZR, nil);
        return 1;
    case Top:
        wtopme(w);
        return 1;
    case Bottom:
        wbottomme(w);
        return 1;
    case Current:
        wcurrent(w);
        return 1;
    case Hide:
        switch(whide(w)){
        case -1:
            strcpy(err, "window already hidden");
            return -1;
        case 0:
            strcpy(err, "hide failed");
            return -1;
        default:
            break;
        }
        return 1;
    case Unhide:
        for(j=0; j<nhidden; j++)
            if(hidden[j] == w)
                break;
        if(j == nhidden){
            strcpy(err, "window not hidden");
            return -1;
        }
        if(wunhide(j) == 0){
            strcpy(err, "hide failed");
            return -1;
        }
        return 1;
    case Delete:
        wsendctlmesg(w, Deleted, ZR, nil);
        return 1;
    }
    strcpy(err, "invalid wctl message");
    return -1;
}
/*e: function [[writewctl]] */

/*s: function [[wctlthread]] */
void
wctlthread(void *v)
{
    Channel *c = v;
    char *buf, *arg, *dir;
    int cmd, id, pid, hideit, scrollit;
    Rectangle rect;
    char err[ERRMAX];

    threadsetname("WCTLTHREAD");

    for(;;){
        buf = recvp(c);

        cmd = parsewctl(&arg, ZR, &rect, &pid, &id, &hideit, &scrollit, &dir, buf, err);

        switch(cmd){
        case New:
            wctlnew(rect, arg, pid, hideit, scrollit, dir, err);
        }
        free(buf);
    }
}
/*e: function [[wctlthread]] */

/*s: function [[wctlproc]] */
void
wctlproc(void *v)
{
    Channel *c = v;
    char *buf;
    int n, eofs;

    threadsetname("WCTLPROC");

    eofs = 0;
    for(;;){
        buf = emalloc(messagesize);
        // blocking call
        n = read(wctlfd, buf, messagesize-1);	/* room for \0 */
        if(n < 0)
            break;
        if(n == 0){
            if(++eofs > 20)
                break;
            continue;
        }
        eofs = 0;

        buf[n] = '\0';
        sendp(c, buf);
    }
}
/*e: function [[wctlproc]] */
/*e: windows/rio/wctl.c */
