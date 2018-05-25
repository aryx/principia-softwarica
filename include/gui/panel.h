/*s: include/gui/panel.h */
#pragma	src	"/sys/src/libpanel"
#pragma	lib	"libpanel.a"

/*s: type [[Icon]] */
typedef void Icon;			/* Always used as Icon * -- Image or char */
/*e: type [[Icon]] */

// forward decls
typedef struct Panel Panel;		/* a Graphical User Interface element */
typedef struct Scroll Scroll;
typedef struct Rtext Rtext;		/* formattable text */
typedef struct Idol Idol;		/* A picture/text combo */

/*s: struct [[Scroll]] */
struct Scroll{
    Point pos, size;
};
/*e: struct [[Scroll]] */
/*s: struct [[Rtext]] */
struct Rtext{
    int flags;		/* responds to hits? text selection? */
    void *user;		/* user data */
    int space;		/* how much space before, if no break */
    int indent;		/* how much space before, after a break */
    Image *b;		/* what to display, if nonzero */
    Panel *p;		/* what to display, if nonzero and b==0 */
    Font *font;		/* font in which to draw text */
    char *text;		/* what to display, if b==0 and p==0 */
    Rtext *next;		/* next piece */
    /* private below */
    Rtext *nextline;	/* links line to line */
    Rtext *last;		/* last, for append */
    Rectangle r;		/* where to draw, if origin were Pt(0,0) */
    int topy;		/* y coord of top of line */
    int wid;		/* not including space */
};
/*e: struct [[Rtext]] */
/*s: struct [[Panel]] */
struct Panel{
    Point ipad, pad;				/* extra space inside and outside */
    Point fixedsize;				/* size of Panel, if FIXED */

    int user;					/* available for user */
    void *userp;					/* available for user */

    Rectangle r;					/* where the Panel goes */




    /* private below */
    Panel *next;					/* It's a list! */
    Panel *child, *echild, *parent;			/* No, it's a tree! */

    Image *b;					/* where we're drawn */
    int flags;					/* position flags, see below */
    char *kind;					/* what kind of panel? */
    int state;					/* for hitting & drawing purposes */
    Point size;					/* space for this Panel */
    Point sizereq;					/* size requested by this Panel */
    Point childreq;					/* total size needed by children */
    Panel *lastmouse;				/* who got the last mouse event? */
    Panel *scrollee;				/* pointer to scrolled window */
    Panel *xscroller, *yscroller;			/* pointers to scroll bars */
    Scroll scr;					/* scroll data */

    void *data;					/* kind-specific data */

    void (*draw)(Panel *);				/* draw panel and children */

    int (*hit)(Panel *, Mouse *);			/* process mouse event */
    void (*type)(Panel *, Rune);			/* process keyboard event */

    Point (*getsize)(Panel *, Point);		/* return size, given child size */
    void (*childspace)(Panel *, Point *, Point *);	/* child ul & size given our size */


    int (*pri)(Panel *, Point);			/* priority for hitting */

    void (*scroll)(Panel *, int, int, int, int);	/* scroll bar to scrollee */
    void (*setscrollbar)(Panel *, int, int, int);	/* scrollee to scroll bar */
    void (*free)(Panel *);				/* free fields of data when done */
    char* (*snarf)(Panel *);			/* snarf text from panel */
    void (*paste)(Panel *, char *);			/* paste text into panel */
};
/*e: struct [[Panel]] */

/*
 * Panel flags
 */
/*s: constant [[PACK]] */
#define	PACK	0x0007		/* which side of the parent is the Panel attached to? */
/*e: constant [[PACK]] */
/*s: constant [[PACKN]] */
#define		PACKN	0x0000
/*e: constant [[PACKN]] */
/*s: constant [[PACKE]] */
#define		PACKE	0x0001
/*e: constant [[PACKE]] */
/*s: constant [[PACKS]] */
#define		PACKS	0x0002
/*e: constant [[PACKS]] */
/*s: constant [[PACKW]] */
#define		PACKW	0x0003
/*e: constant [[PACKW]] */

/*s: constant [[PACKCEN]] */
#define		PACKCEN	0x0004	/* only used by pulldown */
/*e: constant [[PACKCEN]] */

/*s: constant [[FILLX]] */
#define	FILLX	0x0008		/* grow horizontally to fill the available space */
/*e: constant [[FILLX]] */
/*s: constant [[FILLY]] */
#define	FILLY	0x0010		/* grow vertically to fill the available space */
/*e: constant [[FILLY]] */

/*s: constant [[PLACE]] */
#define	PLACE	0x01e0		/* which side of its space should the Panel adhere to? */
/*e: constant [[PLACE]] */
/*s: constant [[PLACECEN]] */
#define		PLACECEN 0x0000
/*e: constant [[PLACECEN]] */

/*s: constant [[PLACES]] */
#define		PLACES	0x0020
/*e: constant [[PLACES]] */
/*s: constant [[PLACEE]] */
#define		PLACEE	0x0040
/*e: constant [[PLACEE]] */
/*s: constant [[PLACEW]] */
#define		PLACEW	0x0060
/*e: constant [[PLACEW]] */
/*s: constant [[PLACEN]] */
#define		PLACEN	0x0080
/*e: constant [[PLACEN]] */
/*s: constant [[PLACENE]] */
#define		PLACENE	0x00a0
/*e: constant [[PLACENE]] */
/*s: constant [[PLACENW]] */
#define		PLACENW	0x00c0
/*e: constant [[PLACENW]] */
/*s: constant [[PLACESE]] */
#define		PLACESE	0x00e0
/*e: constant [[PLACESE]] */
/*s: constant [[PLACESW]] */
#define		PLACESW	0x0100
/*e: constant [[PLACESW]] */

/*s: constant [[EXPAND]] */
#define	EXPAND	0x0200		/* use up all extra space in the parent */
/*e: constant [[EXPAND]] */

/*s: constant [[FIXED]] */
#define	FIXED	0x0c00		/* don't pass children's size requests through to parent */
/*e: constant [[FIXED]] */
/*s: constant [[FIXEDX]] */
#define	FIXEDX	0x0400
/*e: constant [[FIXEDX]] */
/*s: constant [[FIXEDY]] */
#define	FIXEDY	0x0800
/*e: constant [[FIXEDY]] */

/*s: constant [[MAXX]] */
#define	MAXX	0x1000		/* make x size as big as biggest sibling's */
/*e: constant [[MAXX]] */
/*s: constant [[MAXY]] */
#define	MAXY	0x2000		/* make y size as big as biggest sibling's */
/*e: constant [[MAXY]] */

/*s: constant [[BITMAP]] */
#define	BITMAP	0x4000		/* text argument is a bitmap, not a string */
/*e: constant [[BITMAP]] */
/*s: constant [[USERFL]] */
#define USERFL	0x100000	/* start of user flag */
/*e: constant [[USERFL]] */

/*s: constant [[OUT]] */
/*
 * An extra bit in Mouse.buttons
 */
#define	OUT	8			/* Mouse.buttons bit, set when mouse leaves Panel */
/*e: constant [[OUT]] */

/*s: constant [[PRI_NORMAL]] */
/*
 * Priorities
 */
#define	PRI_NORMAL	0		/* ordinary panels */
/*e: constant [[PRI_NORMAL]] */
/*s: constant [[PRI_POPUP]] */
#define	PRI_POPUP	1		/* popup menus */
/*e: constant [[PRI_POPUP]] */
/*s: constant [[PRI_SCROLLBAR]] */
#define	PRI_SCROLLBAR	2		/* scroll bars */
/*e: constant [[PRI_SCROLLBAR]] */

/*s: constant [[PL_HOT]] */
/* Rtext.flags */
#define PL_HOT		1
/*e: constant [[PL_HOT]] */
/*s: constant [[PL_SEL]] */
#define PL_SEL		2
/*e: constant [[PL_SEL]] */

/*s: global [[plkbfocus]] */
Panel *plkbfocus;			/* the panel in keyboard focus */
/*e: global [[plkbfocus]] */

// Initialization
int plinit(int);			/* initialization */

// Memory
void plfree(Panel *);			/* give back space */

// Drawing
void pldraw(Panel *, Image *);		/* display the panel on the bitmap */

// Events
void plkeyboard(Rune);			/* send a keyboard event to the appropriate Panel */
void plmouse(Panel *, Mouse *);		/* send a Mouse event to a Panel tree */

// Packing
void plpack(Panel *, Rectangle);	/* figure out where to put the Panel & children */
void plmove(Panel *, Point);		/* move an already-packed panel to a new location */


// setters
void plplacelabel(Panel *, int);	/* label placement */
void plsetbutton(Panel *, int);		/* set or clear the mark on a button */
void plsetslider(Panel *, int, int);	/* set the value of a slider */
void plesel(Panel *, int, int);		/* set the selection in an edit window */
void plescroll(Panel *, int);		/* scroll an edit window */
void plsetscroll(Panel *, Scroll);	/* set scrolling information */

// getters
char *plentryval(Panel *);		/* entry delivers its value */
Rune *pleget(Panel *);			/* get the text from an edit window */
int plelen(Panel *);			/* get the length of the text from an edit window */
void plegetsel(Panel *, int *, int *);	/* get the selection from an edit window */
Scroll plgetscroll(Panel *);		/* get scrolling information from panel */

//XXX
void plgrabkb(Panel *);			/* this Panel should receive keyboard events */
void plscroll(Panel *, Panel *, Panel *); /* link up scroll bars */
void plepaste(Panel *, Rune *, int);	/* paste in an edit window */

/*
 * Panel creation & reinitialization functions
 */
Panel *pllabel(Panel *pl, int, Icon *);

Panel *plbutton(Panel *pl, int, Icon *, void (*)(Panel *pl, int));
Panel *plcheckbutton(Panel *pl, int, Icon *, void (*)(Panel *pl, int, int));
Panel *plradiobutton(Panel *pl, int, Icon *, void (*)(Panel *pl, int, int));

Panel *plentry(Panel *pl, int, int, char *, void (*)(Panel *pl, char *));
Panel *pledit(Panel *, int, Point, Rune *, int, void (*)(Panel *));

Panel *plslider(Panel *pl, int, Point, void(*)(Panel *pl, int, int, int));

Panel *pllist(Panel *pl, int, char *(*)(Panel *, int), int, void(*)(Panel *pl, int, int));
Panel *plidollist(Panel*, int, Point, Font*, Idol*, void (*)(Panel*, int, void*));
Panel *plgroup(Panel *pl, int);
Panel *plframe(Panel *pl, int);
Panel *plcanvas(Panel *pl, int, void (*)(Panel *), void (*)(Panel *pl, Mouse *));

Panel *plmenu(Panel *pl, int, Icon **, int, void (*)(int, int));
Panel *plmenubar(Panel *pl, int, int, Icon *, Panel *pl, Icon *, ...);
Panel *plpopup(Panel *pl, int, Panel *pl, Panel *pl, Panel *pl);
Panel *plpulldown(Panel *pl, int, Icon *, Panel *pl, int);

Panel *plscrollbar(Panel *plparent, int flags);
Panel *pltextview(Panel *, int, Point, Rtext *, void (*)(Panel *, int, Rtext *));

Panel *plmessage(Panel *pl, int, int, char *);

void plinitlabel(Panel *, int, Icon *);

void plinitbutton(Panel *, int, Icon *, void (*)(Panel *, int));
void plinitcheckbutton(Panel *, int, Icon *, void (*)(Panel *, int, int));
void plinitradiobutton(Panel *, int, Icon *, void (*)(Panel *, int, int));

void plinitcanvas(Panel *, int, void (*)(Panel *), void (*)(Panel *, Mouse *));
void plinitedit(Panel *, int, Point, Rune *, int, void (*)(Panel *));
void plinitentry(Panel *, int, int, char *, void (*)(Panel *, char *));
void plinitframe(Panel *, int);
void plinitgroup(Panel *, int);
void plinitidollist(Panel*, int, Point, Font*, Idol*, void (*)(Panel*, int, void*));
void plinitlist(Panel *, int, char *(*)(Panel *, int), int, void(*)(Panel *, int, int));
void plinitmenu(Panel *, int, Icon **, int, void (*)(int, int));
void plinitmessage(Panel *, int, int, char *);
void plinitpopup(Panel *, int, Panel *, Panel *, Panel *);
void plinitpulldown(Panel *, int, Icon *, Panel *, int);
void plinitscrollbar(Panel *parent, int flags);
void plinitslider(Panel *, int, Point, void(*)(Panel *, int, int, int));
void plinittextview(Panel *, int, Point, Rtext *, void (*)(Panel *, int, Rtext *));

/*
 * Rtext constructors & destructor
 */
Rtext *plrtstr(Rtext **, int, int, Font *, char *, int, void *);
Rtext *plrtbitmap(Rtext **, int, int, Image *, int, void *);
Rtext *plrtpanel(Rtext **, int, int, Panel *, void *);
void plrtfree(Rtext *);
void plrtseltext(Rtext *, Rtext *, Rtext *);
char *plrtsnarftext(Rtext *);

int plgetpostextview(Panel *);
void plsetpostextview(Panel *, int);

/*
 * Idols
 */
Idol *plmkidol(Idol**, Image*, Image*, char*, void*);
void plfreeidol(Idol*);
Point plidolsize(Idol*, Font*, int);
void *plidollistgetsel(Panel*);

/*
 * Snarf
 */
void plputsnarf(char *);
char *plgetsnarf(void);
void plsnarf(Panel *);			/* snarf a panel */
void plpaste(Panel *);			/* paste a panel */
/*e: include/gui/panel.h */
