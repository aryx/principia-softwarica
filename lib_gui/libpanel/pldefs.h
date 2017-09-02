/*s: windows/libpanel/pldefs.h */
/*
 * Definitions for internal use only
 */
/*
 * Variable-font text routines
 * These could make a separate library.
 */
int pl_rtfmt(Rtext *, int);
void pl_rtdraw(Image *, Rectangle, Rtext *, int);
void pl_rtredraw(Image *, Rectangle, Rtext *, int, int);
Rtext *pl_rthit(Rtext *, int, Point, Point);
/*s: constant HITME */
#define	HITME	0x08000		/* tells ptinpanel not to look at children */
/*e: constant HITME */
/*s: constant LEAF */
#define	LEAF	0x10000		/* newpanel will refuse to attach children */
/*e: constant LEAF */
/*s: constant INVIS */
#define	INVIS	0x20000		/* don't draw this */
/*e: constant INVIS */
/*s: constant REMOUSE */
#define	REMOUSE	0x40000		/* send next mouse event here, even if not inside */
/*e: constant REMOUSE */
/*s: enum _anon_ (windows/libpanel/pldefs.h) */
/*
 * States, also styles
 */
enum{
    UP,
    DOWN1,
    DOWN2,
    DOWN3,
    DOWN,
    PASSIVE,
    FRAME
};
/*e: enum _anon_ (windows/libpanel/pldefs.h) */
/*s: enum _anon_ (windows/libpanel/pldefs.h)2 */
/*
 * Scroll flags
 */
enum{
    SCROLLUP,
    SCROLLDOWN,
    SCROLLABSY,
    SCROLLLEFT,
    SCROLLRIGHT,
    SCROLLABSX,
};
/*e: enum _anon_ (windows/libpanel/pldefs.h)2 */
/*s: enum _anon_ (windows/libpanel/pldefs.h)3 */
/*
 * Scrollbar, slider orientations
 */
enum{
    HORIZ,
    VERT
};
/*e: enum _anon_ (windows/libpanel/pldefs.h)3 */
Panel *pl_newpanel(Panel *, int);	/* make a new Panel, given parent & data size */
void *pl_emalloc(int);			/* allocate some space, exit on error */
void *pl_erealloc(void*,int);		/* reallocate some space, exit on error */
void pl_print(Panel *);			/* print a Panel tree */
Panel *pl_ptinpanel(Point, Panel *);	/* highest-priority subpanel containing point */
/*
 * Drawing primitives
 */
int pl_drawinit(int);
Rectangle pl_box(Image *, Rectangle, int);
Rectangle pl_outline(Image *, Rectangle, int);
Point pl_boxsize(Point, int);
void pl_interior(int, Point *, Point *);
void pl_drawicon(Image *, Rectangle, int, int, Icon *);
Rectangle pl_check(Image *, Rectangle, int);
Rectangle pl_radio(Image *, Rectangle, int);
int pl_ckwid(void);
void pl_sliderupd(Image *, Rectangle, int, int, int);
void pl_invis(Panel *, int);
Point pl_iconsize(int, Icon *);
void pl_highlight(Image *, Rectangle);
void pl_clr(Image *, Rectangle);
void pl_fill(Image *, Rectangle);
void pl_cpy(Image *, Point, Rectangle);

/*
 * Rune mangling functions
 */
int pl_idchar(int);
int pl_rune1st(int);
char *pl_nextrune(char *);
int pl_runewidth(Font *, char *);
/*
 * Fixed-font Text-window routines
 * These could be separated out into a separate library.
 */
typedef struct Textwin Textwin;
/*s: struct Textwin */
struct Textwin{
    Rune *text, *etext, *eslack;	/* text, with some slack off the end */
    int top, bot;			/* range of runes visible on screen */
    int sel0, sel1;			/* selection */
    Point *loc, *eloc;		/* ul corners of visible runes (+1 more at end!) */
    Image *b;			/* bitmap the text is drawn in */
    Rectangle r;			/* rectangle the text is drawn in */
    Font *font;			/* font text is drawn in */
    int hgt;			/* same as font->height */
    int tabstop;			/* tab settings are every tabstop pixels */
    int mintab;			/* the minimum size of a tab */
};
/*e: struct Textwin */
Textwin *twnew(Image *, Font *, Rune *, int);
void twfree(Textwin *);
void twhilite(Textwin *, int, int, int);
void twselect(Textwin *, Mouse *);
void twreplace(Textwin *, int, int, Rune *, int);
void twscroll(Textwin *, int);
int twpt2rune(Textwin *, Point);
void twreshape(Textwin *, Rectangle);
void twmove(Textwin *, Point);
void plemove(Panel *, Point);
/*e: windows/libpanel/pldefs.h */
