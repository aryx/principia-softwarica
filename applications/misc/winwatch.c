/*s: windows/apps/winwatch.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
//#include <draw_private.h> // for _string, but should not use it
#include <window.h>
#include <event.h>
#include <regexp.h>

extern Point _string(Image *dst, Point pt, Image *src, Point sp, Font *f, char *s, Rune *r, int len, Rectangle clipr, Image *bg, Point bgp, Drawop op);

typedef struct Win Win;
/*s: struct [[Win]] */
struct Win {
    int n;
    int dirty;
    char *label;
    Rectangle r;
};
/*e: struct [[Win]] */



/*s: global [[exclude]] */
static Reprog  *exclude  = nil;
/*e: global [[exclude]] */
/*s: global [[win]] */
static Win *win;
/*e: global [[win]] */
/*s: global [[nwin]] */
static int nwin;
/*e: global [[nwin]] */
/*s: global [[mwin]] */
static int mwin;
/*e: global [[mwin]] */
/*s: global [[onwin]] */
static int onwin;
/*e: global [[onwin]] */
static int rows, cols;
/*s: global [[lightblue]] */
static Image *lightblue;
/*e: global [[lightblue]] */

extern Font *font;

/*s: enum [[_anon_ (windows/apps/winwatch.c)]] */
enum {
    PAD = 3,
    MARGIN = 5
};
/*e: enum [[_anon_ (windows/apps/winwatch.c)]] */

/*s: function erealloc (windows/apps/winwatch.c) */
static void*
erealloc(void *v, ulong n)
{
    v = realloc(v, n);
    if(v == nil)
        sysfatal("out of memory reallocating %lud", n);
    return v;
}
/*e: function erealloc (windows/apps/winwatch.c) */

/*s: function emalloc (windows/apps/winwatch.c) */
static void*
emalloc(ulong n)
{
    void *v;

    v = malloc(n);
    if(v == nil)
        sysfatal("out of memory allocating %lud", n);
    memset(v, 0, n);
    return v;
}
/*e: function emalloc (windows/apps/winwatch.c) */

/*s: function estrdup (windows/apps/winwatch.c) */
static char*
estrdup(char *s)
{
    int l;
    char *t;

    if (s == nil)
        return nil;
    l = strlen(s)+1;
    t = emalloc(l);
    memcpy(t, s, l);

    return t;
}
/*e: function estrdup (windows/apps/winwatch.c) */


/*s: function [[refreshwin]] */
static void
refreshwin(void)
{
    char label[128];
    int i, fd, lfd, n, nr, nw, m;
    Dir *pd;

    if((fd = open("/dev/wsys", OREAD)) < 0)
        return;

    nw = 0;
/* i'd rather read one at a time but rio won't let me */
    while((nr=dirread(fd, &pd)) > 0){
        for(i=0; i<nr; i++){
            n = atoi(pd[i].name);
            sprint(label, "/dev/wsys/%d/label", n);
            if((lfd = open(label, OREAD)) < 0)
                continue;
            m = read(lfd, label, sizeof(label)-1);
            close(lfd);
            if(m < 0)
                continue;
            label[m] = '\0';
            if(exclude != nil && regexec(exclude,label,nil,0))
                continue;

            if(nw < nwin && win[nw].n == n && strcmp(win[nw].label, label)==0){
                nw++;
                continue;
            }
    
            if(nw < nwin){
                free(win[nw].label);
                win[nw].label = nil;
            }
            
            if(nw >= mwin){
                mwin += 8;
                win = erealloc(win, mwin*sizeof(win[0]));
            }
            win[nw].n = n;
            win[nw].label = estrdup(label);
            win[nw].dirty = 1;
            win[nw].r = Rect(0,0,0,0);
            nw++;
        }
        free(pd);
    }
    while(nwin > nw)
        free(win[--nwin].label);
    nwin = nw;
    close(fd);
}
/*e: function [[refreshwin]] */

/*s: function [[drawnowin]] */
static void
drawnowin(int i)
{
    Rectangle r;

    r = Rect(0,0,(Dx(view->r)-2*MARGIN+PAD)/cols-PAD, font->height);
    r = rectaddpt(rectaddpt(r, Pt(MARGIN+(PAD+Dx(r))*(i/rows),
                MARGIN+(PAD+Dy(r))*(i%rows))), view->r.min);
    draw(view, insetrect(r, -1), lightblue, nil, ZP);
}
/*e: function [[drawnowin]] */

/*s: function [[drawwin]] */
static void
drawwin(int i)
{
    draw(view, win[i].r, lightblue, nil, ZP);
    _string(view, addpt(win[i].r.min, Pt(2,0)), display->black, ZP,
        font, win[i].label, nil, strlen(win[i].label), 
        win[i].r, nil, ZP, SoverD);
    border(view, win[i].r, 1, display->black, ZP);	
    win[i].dirty = 0;
}
/*e: function [[drawwin]] */

/*s: function [[geometry]] */
static int
geometry(void)
{
    int i, ncols, z;
    Rectangle r;

    z = 0;
    rows = (Dy(view->r)-2*MARGIN+PAD)/(font->height+PAD);
    if(rows*cols < nwin || rows*cols >= nwin*2){
        ncols = nwin <= 0 ? 1 : (nwin+rows-1)/rows;
        if(ncols != cols){
            cols = ncols;
            z = 1;
        }
    }

    r = Rect(0,0,(Dx(view->r)-2*MARGIN+PAD)/cols-PAD, font->height);
    for(i=0; i<nwin; i++)
        win[i].r = rectaddpt(rectaddpt(r, Pt(MARGIN+(PAD+Dx(r))*(i/rows),
                    MARGIN+(PAD+Dy(r))*(i%rows))), view->r.min);

    return z;
}
/*e: function [[geometry]] */

/*s: function redraw (windows/apps/winwatch.c) */
static void
redraw(Image *view, int all)
{
    int i;

    all |= geometry();
    if(all)
        draw(view, view->r, lightblue, nil, ZP);
    for(i=0; i<nwin; i++)
        if(all || win[i].dirty)
            drawwin(i);
    if(!all)
        for(; i<onwin; i++)
            drawnowin(i);

    onwin = nwin;
}
/*e: function redraw (windows/apps/winwatch.c) */

/*s: function eresized (windows/apps/winwatch.c) */
void
eresized(int new)
{
    if(new && getwindow(display, Refmesg) < 0)
        fprint(2,"can't reattach to window");
    geometry();
    redraw(view, 1);
}
/*e: function eresized (windows/apps/winwatch.c) */

/*s: function [[click]] */
static void
click(Mouse m)
{
    int fd, i, j;	
    char buf[128];

    if(m.buttons == 0 || (m.buttons & ~4))
        return;

    for(i=0; i<nwin; i++)
        if(ptinrect(m.xy, win[i].r))
            break;
    if(i == nwin)
        return;

    do
        m = emouse();
    while(m.buttons == 4);

    if(m.buttons != 0){
        do
            m = emouse();
        while(m.buttons);
        return;
    }

    for(j=0; j<nwin; j++)
        if(ptinrect(m.xy, win[j].r))
            break;
    if(j != i)
        return;

    sprint(buf, "/dev/wsys/%d/wctl", win[i].n);
    if((fd = open(buf, OWRITE)) < 0)
        return;
    write(fd, "unhide\n", 7);
    write(fd, "top\n", 4);
    write(fd, "current\n", 8);
    close(fd);
}
/*e: function [[click]] */

/*s: function usage (windows/apps/winwatch.c) */
static void
usage(void)
{
    fprint(2, "usage: winwatch [-e exclude] [-f font]\n");
    exits("usage");
}
/*e: function usage (windows/apps/winwatch.c) */

/*s: function main (windows/apps/winwatch.c) */
void
main(int argc, char **argv)
{
    char *fontname;
    int Etimer;
    Event e;

    fontname = "/lib/font/bit/lucidasans/unicode.8.font";
    ARGBEGIN{
    case 'f':
        fontname = EARGF(usage());
        break;
    case 'e':
        exclude = regcomp(EARGF(usage()));
        if(exclude == nil)
            sysfatal("Bad regexp");
        break;
    default:
        usage();
    }ARGEND

    if(argc)
        usage();

    initdraw(0, 0, "winwatch");
    lightblue = allocimagemix(display, DPalebluegreen, DWhite);
    if(lightblue == nil)
        sysfatal("allocimagemix: %r");
    if((font = openfont(display, fontname)) == nil)
        sysfatal("font '%s' not found", fontname);

    refreshwin();
    redraw(view, 1);
    einit(Emouse|Ekeyboard);
    Etimer = etimer(0, 2500);

    for(;;){
        switch(eread(Emouse|Ekeyboard|Etimer, &e)){
        case Ekeyboard:
            if(e.kbdc==0x7F || e.kbdc=='q')
                exits(0);
            break;
        case Emouse:
            if(e.mouse.buttons)
                click(e.mouse);
            /* fall through  */
        default:	/* Etimer */
            refreshwin();
            redraw(view, 0);
            break;
        }
    }
}
/*e: function main (windows/apps/winwatch.c) */
/*e: windows/apps/winwatch.c */
