/*s: windows/rio/dat.h */
/*s: enum _anon_ (windows/rio/dat.h) */
enum
{
    Qdir,			/* /dev for this window */
    Qcons,
    Qconsctl,
    Qcursor,
    Qwdir,
    Qwinid,
    Qwinname,
    Qkbdin,
    Qlabel,
    Qmouse,
    Qnew,
    Qscreen,
    Qsnarf,
    Qtext,
    Qwctl,
    Qwindow,
    Qwsys,		/* directory of window directories */
    Qwsysdir,		/* window directory, child of wsys */

    QMAX,
};
/*e: enum _anon_ (windows/rio/dat.h) */

/*s: enum _anon_ (windows/rio/dat.h)2 */
enum
{
    Kscrolloneup = KF|0x20,
    Kscrollonedown = KF|0x21,
};
/*e: enum _anon_ (windows/rio/dat.h)2 */

/*s: constant STACK */
#define	STACK	8192
/*e: constant STACK */

typedef	struct	Consreadmesg Consreadmesg;
typedef	struct	Conswritemesg Conswritemesg;
typedef	struct	Stringpair Stringpair;
typedef	struct	Dirtab Dirtab;
typedef	struct	FidRio Fid;
typedef	struct	Filsys Filsys;
typedef	struct	Mouseinfo	Mouseinfo;
typedef	struct	Mousereadmesg Mousereadmesg;
typedef	struct	Mousestate	Mousestate;
typedef	struct	Ref Ref;
typedef	struct	Timer Timer;
typedef	struct	Wctlmesg Wctlmesg;
typedef	struct	Window Window;
typedef	struct	Xfid Xfid;

/*s: enum _anon_ (windows/rio/dat.h)3 */
enum
{
    Selborder		= 4,		/* border of selected window */
    Unselborder	= 1,		/* border of unselected window */
    Scrollwid 		= 12,		/* width of scroll bar */
    Scrollgap 		= 4,		/* gap right of scroll bar */
    BIG			= 3,		/* factor by which window dimension can exceed screen */
};
/*e: enum _anon_ (windows/rio/dat.h)3 */

/*s: enum _anon_ (windows/rio/dat.h)4 */
enum
{
    TRUE		= 1,
    FALSE		= 0,
};
/*e: enum _anon_ (windows/rio/dat.h)4 */

/*s: function QID */
#define	QID(w,q)	((w<<8)|(q))
/*e: function QID */
/*s: function WIN */
#define	WIN(q)	((((ulong)(q).path)>>8) & 0xFFFFFF)
/*e: function WIN */
/*s: function FILE */
#define	FILE(q)	(((ulong)(q).path) & 0xFF)
/*e: function FILE */

/*s: enum _anon_ (windows/rio/dat.h)5 */
enum	/* control messages */
{
    Wakeup,
    Reshaped,
    Moved,
    Refresh,
    Movemouse,
    Rawon,
    Rawoff,
    Holdon,
    Holdoff,
    Deleted,
    Exited,
};
/*e: enum _anon_ (windows/rio/dat.h)5 */

/*s: struct Wctlmesg */
struct Wctlmesg
{
    int		type;
    Rectangle	r;
    Image	*image;
};
/*e: struct Wctlmesg */

/*s: struct Conswritemesg */
struct Conswritemesg
{
    Channel	*cw;		/* chan(Stringpair) */
};
/*e: struct Conswritemesg */

/*s: struct Consreadmesg */
struct Consreadmesg
{
    Channel	*c1;		/* chan(tuple(char*, int) == Stringpair) */
    Channel	*c2;		/* chan(tuple(char*, int) == Stringpair) */
};
/*e: struct Consreadmesg */

/*s: struct Mousereadmesg */
struct Mousereadmesg
{
    Channel	*cm;		/* chan(Mouse) */
};
/*e: struct Mousereadmesg */

/*s: struct Stringpair */
struct Stringpair	/* rune and nrune or byte and nbyte */
{
    void		*s;
    int		ns;
};
/*e: struct Stringpair */

/*s: struct Mousestate */
struct Mousestate
{
    Mouse;
    ulong	counter;	/* serial no. of mouse event */
};
/*e: struct Mousestate */

/*s: struct Mouseinfo */
struct Mouseinfo
{
    Mousestate	queue[16];
    int	ri;	/* read index into queue */
    int	wi;	/* write index */
    ulong	counter;	/* serial no. of last mouse event we received */
    ulong	lastcounter;	/* serial no. of last mouse event sent to client */
    int	lastb;	/* last button state we received */
    uchar	qfull;	/* filled the queue; no more recording until client comes back */	
};	
/*e: struct Mouseinfo */

/*s: struct Window */
struct Window
{
    Ref;
    QLock;
    Frame;
    Image		*i;
    Mousectl		mc;
    Mouseinfo	mouse;
    Channel		*ck;			/* chan(Rune[10]) */
    Channel		*cctl;		/* chan(Wctlmesg)[20] */
    Channel		*conswrite;	/* chan(Conswritemesg) */
    Channel		*consread;	/* chan(Consreadmesg) */
    Channel		*mouseread;	/* chan(Mousereadmesg) */
    Channel		*wctlread;		/* chan(Consreadmesg) */
    uint			nr;			/* number of runes in window */
    uint			maxr;		/* number of runes allocated in r */
    Rune			*r;
    uint			nraw;
    Rune			*raw;
    uint			org;
    uint			q0;
    uint			q1;
    uint			qh;
    int			id;
    char			name[32];
    uint			namecount;
    Rectangle		scrollr;
    /*
     * Rio once used originwindow, so screenr could be different from i->r.
     * Now they're always the same but the code doesn't assume so.
    */
    Rectangle		screenr;	/* screen coordinates of window */
    int			resized;
    int			wctlready;
    Rectangle		lastsr;
    int			topped;
    int			notefd;
    uchar		scrolling;
    Cursor		cursor;
    Cursor		*cursorp;
    uchar		holding;
    uchar		rawing;
    uchar		ctlopen;
    uchar		wctlopen;
    uchar		deleted;
    uchar		mouseopen;
    char			*label;
    int			pid;
    char			*dir;
};
/*e: struct Window */

int		winborder(Window*, Point);
void		winctl(void*);
void		winshell(void*);
Window*	wlookid(int);
Window*	wmk(Image*, Mousectl*, Channel*, Channel*, int);
Window*	wpointto(Point);
Window*	wtop(Point);
void		wtopme(Window*);
void		wbottomme(Window*);
char*	wcontents(Window*, int*);
int		wbswidth(Window*, Rune);
int		wclickmatch(Window*, int, int, int, uint*);
int		wclose(Window*);
int		wctlmesg(Window*, int, Rectangle, Image*);
int		wctlmesg(Window*, int, Rectangle, Image*);
uint		wbacknl(Window*, uint, uint);
uint		winsert(Window*, Rune*, int, uint);
void		waddraw(Window*, Rune*, int);
void		wborder(Window*, int);
void		wclosewin(Window*);
void		wcurrent(Window*);
void		wcut(Window*);
void		wdelete(Window*, uint, uint);
void		wdoubleclick(Window*, uint*, uint*);
void		wfill(Window*);
void		wframescroll(Window*, int);
void		wkeyctl(Window*, Rune);
void		wmousectl(Window*);
void		wmovemouse(Window*, Point);
void		wpaste(Window*);
void		wplumb(Window*);
void		wrefresh(Window*, Rectangle);
void		wrepaint(Window*);
void		wresize(Window*, Image*, int);
void		wscrdraw(Window*);
void		wscroll(Window*, int);
void		wselect(Window*);
void		wsendctlmesg(Window*, int, Rectangle, Image*);
void		wsetcursor(Window*, int);
void		wsetname(Window*);
void		wsetorigin(Window*, uint, int);
void		wsetpid(Window*, int, int);
void		wsetselect(Window*, uint, uint);
void		wshow(Window*, uint);
void		wsnarf(Window*);
void 		wscrsleep(Window*, uint);
void		wsetcols(Window*);

/*s: struct Dirtab */
struct Dirtab
{
    char		*name;
    uchar	type;
    uint		qid;
    uint		perm;
};
/*e: struct Dirtab */

/*s: struct FidRio */
struct FidRio
{
    int		fid;
    int		busy;
    int		open;
    int		mode;
    Qid		qid;
    Window	*w;
    Dirtab	*dir;
    Fid		*next;
    int		nrpart;
    uchar	rpart[UTFmax];
};
/*e: struct FidRio */

/*s: struct Xfid */
struct Xfid
{
        Ref;
        Xfid		*next;
        Xfid		*free;
        Fcall;
        Channel	*c;	/* chan(void(*)(Xfid*)) */
        Fid		*f;
        uchar	*buf;
        Filsys	*fs;
        QLock	active;
        int		flushing;	/* another Xfid is trying to flush us */
        int		flushtag;	/* our tag, so flush can find us */
        Channel	*flushc;	/* channel(int) to notify us we're being flushed */
};
/*e: struct Xfid */

Channel*	xfidinit(void);
void		xfidctl(void*);
void		xfidflush(Xfid*);
void		xfidattach(Xfid*);
void		xfidopen(Xfid*);
void		xfidclose(Xfid*);
void		xfidread(Xfid*);
void		xfidwrite(Xfid*);

/*s: enum _anon_ (windows/rio/dat.h)6 */
enum
{
    Nhash	= 16,
};
/*e: enum _anon_ (windows/rio/dat.h)6 */

/*s: struct Filsys */
struct Filsys
{
        int		cfd;
        int		sfd;
        int		pid;
        char		*user;
        Channel	*cxfidalloc;	/* chan(Xfid*) */
        Fid		*fids[Nhash];
};
/*e: struct Filsys */

Filsys*	filsysinit(Channel*);
int		filsysmount(Filsys*, int);
Xfid*		filsysrespond(Filsys*, Xfid*, Fcall*, char*);
void		filsyscancel(Xfid*);

void		wctlproc(void*);
void		wctlthread(void*);

void		deletetimeoutproc(void*);

/*s: struct Timer */
struct Timer
{
    int		dt;
    int		cancel;
    Channel	*c;	/* chan(int) */
    Timer	*next;
};
/*e: struct Timer */

extern Font		*font;
extern Mousectl	*mousectl;
extern Mouse	*mouse;
extern Keyboardctl	*keyboardctl;
extern Display	*display;
extern Image	*view;
extern Screen	*wscreen;
extern Cursor	boxcursor;
extern Cursor	crosscursor;
extern Cursor	sightcursor;
extern Cursor	whitearrow;
extern Cursor	query;
extern Cursor	*corners[9];
extern Image	*background;
extern Image	*lightgrey;
extern Image	*red;
extern Window	**window;
extern Window	*wkeyboard;	/* window of simulated keyboard */
extern int		nwindow;
extern int		snarffd;
extern Window	*input;
extern QLock	all;			/* BUG */
extern Filsys	*filsys;
extern Window	*hidden[100];
extern int		nhidden;
extern int		nsnarf;
extern Rune*	snarf;
extern int		scrolling;
extern int		maxtab;
extern Channel*	winclosechan;
extern Channel*	deletechan;
extern char		*startdir;
extern int		sweeping;
extern int		wctlfd;
extern char		srvpipe[];
extern char		srvwctl[];
extern int		errorshouldabort;
extern int		menuing;
extern int		snarfversion;	/* updated each time it is written */
extern int		messagesize; 		/* negotiated in 9P version setup */

/*e: windows/rio/dat.h */
