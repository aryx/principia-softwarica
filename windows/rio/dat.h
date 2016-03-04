/*s: windows/rio/dat.h */
/*s: enum qid */
enum qid
{
    Qdir,			/* /dev for this window */
    /*s: enum qid cases */
    Qcons,
    /*x: enum qid cases */
    Qconsctl,
    /*x: enum qid cases */
    Qmouse,
    /*x: enum qid cases */
    Qcursor,
    /*x: enum qid cases */
    Qscreen,
    /*x: enum qid cases */
    Qwindow,
    /*x: enum qid cases */
    Qwinid,
    /*x: enum qid cases */
    Qwinname,
    /*x: enum qid cases */
    Qlabel,
    /*x: enum qid cases */
    Qtext,
    /*x: enum qid cases */
    Qwdir,
    /*x: enum qid cases */
    Qwsys,		/* directory of window directories */
    /*x: enum qid cases */
    Qwsysdir,		/* window directory, child of wsys */
    /*x: enum qid cases */
    Qwctl,
    /*x: enum qid cases */
    Qsnarf,
    /*x: enum qid cases */
    Qkbdin,
    /*e: enum qid cases */
    QMAX,
};
/*e: enum qid */

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
typedef	struct	Fid Fid;
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
    Selborder		= 4,	/* border of selected window */
    Unselborder		= 1,	/* border of unselected window */
    Scrollwid 		= 12,	/* width of scroll bar */
    Scrollgap 		= 4,	/* gap right of scroll bar */
    BIG			= 3,	/* factor by which window dimension can exceed screen */
};
/*e: enum _anon_ (windows/rio/dat.h)3 */

/*s: function QID */
#define	QID(w,q)	((w<<8)|(q))
/*e: function QID */
/*s: function WIN */
#define	WIN(q)	((((ulong)(q).path)>>8) & 0xFFFFFF)
/*e: function WIN */
/*s: function FILE */
#define	FILE(q)	(((ulong)(q).path) & 0xFF)
/*e: function FILE */

/*s: enum wctlmesgkind */
enum	/* control messages */
{
    Wakeup,

    Reshaped, // Resized, Hide/Unhind
    Moved,

    Refresh,
    Movemouse,

    /*s: enum wctlmesgkind cases */
    Rawon,
    Rawoff,
    /*x: enum wctlmesgkind cases */
    Holdon,
    Holdoff,
    /*e: enum wctlmesgkind cases */

    Deleted,
    Exited,
};
/*e: enum wctlmesgkind */

/*s: struct Wctlmesg */
struct Wctlmesg
{
    // enum<wctlmesgkind>
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
    void	*s;
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
    //--------------------------------------------------------------------
    // ID
    //--------------------------------------------------------------------
    /*s: [[Window]] id fields */
    int	id;       // /dev/winid
    char	name[32]; // /dev/winname
    char	*label;   // /dev/label
    /*x: [[Window]] id fields */
    uint		namecount;
    /*e: [[Window]] id fields */
    
    //--------------------------------------------------------------------
    // Screen
    //--------------------------------------------------------------------
    /*s: [[Window]] screen fields */
    Image	*i;
    /*
     * Rio once used originwindow, so screenr could be different from i->r.
     * Now they're always the same but the code doesn't assume so.
    */
    Rectangle	screenr; /* screen coordinates of window */
    Frame;
    /*e: [[Window]] screen fields */
    
    //--------------------------------------------------------------------
    // Mouse
    //--------------------------------------------------------------------
    /*s: [[Window]] mouse fields */
    Mousectl	mc; // mc.c is the mouse event listening channel
    /*x: [[Window]] mouse fields */
    Cursor		cursor;
    Cursor		*cursorp;
    /*x: [[Window]] mouse fields */
    Mouseinfo	mouse;
    /*e: [[Window]] mouse fields */
    
    //--------------------------------------------------------------------
    // Keyboard
    //--------------------------------------------------------------------
    /*s: [[Window]] keyboard fields */
    Channel	*ck;		/* chan(Rune[10]) */
    /*e: [[Window]] keyboard fields */
    
    //--------------------------------------------------------------------
    // Control
    //--------------------------------------------------------------------
    /*s: [[Window]] control fields */
    Channel	*cctl;		/* chan(Wctlmesg)[20] */
    /*e: [[Window]] control fields */
    
    //--------------------------------------------------------------------
    // Config
    //--------------------------------------------------------------------
    /*s: [[Window]] config fields */
    bool_byte	scrolling;
    /*x: [[Window]] config fields */
    bool_byte	deleted;
    /*x: [[Window]] config fields */
    bool_byte	rawing;
    /*x: [[Window]] config fields */
    bool_byte	holding;
    /*e: [[Window]] config fields */

    //--------------------------------------------------------------------
    // Textual Window
    //--------------------------------------------------------------------
    /*s: [[Window]] textual window fields */
    Rectangle	scrollr;
    /*x: [[Window]] textual window fields */
    // array<Rune>
    Rune		*r;
    uint		nr;	/* number of runes in window */
    uint		maxr;	/* number of runes allocated in r */
    /*x: [[Window]] textual window fields */
    uint		org;
    /*x: [[Window]] textual window fields */
    // selection? start vs end?
    uint		q0;
    uint		q1;
    /*x: [[Window]] textual window fields */
    uint		qh;
    /*e: [[Window]] textual window fields */

    //--------------------------------------------------------------------
    // Graphical Window
    //--------------------------------------------------------------------
    /*s: [[Window]] graphical window fields */
    bool_byte	mouseopen;
    /*x: [[Window]] graphical window fields */
    // array<Rune>
    Rune		*raw;
    uint		nraw;
    /*e: [[Window]] graphical window fields */

    //--------------------------------------------------------------------
    // Misc
    //--------------------------------------------------------------------
    /*s: [[Window]] other fields */
    int	 	topped;
    /*x: [[Window]] other fields */
    Channel		*mouseread;	/* chan(Mousereadmesg) */
    /*x: [[Window]] other fields */
    Channel		*consread;	/* chan(Consreadmesg) */
    /*x: [[Window]] other fields */
    Channel		*conswrite;	/* chan(Conswritemesg) */
    /*x: [[Window]] other fields */
    Channel		*wctlread;	/* chan(Consreadmesg) */
    /*x: [[Window]] other fields */
    int		pid;
    int	 	notefd;
    /*x: [[Window]] other fields */
    bool	 	resized;
    /*x: [[Window]] other fields */
    Rectangle	lastsr;
    /*x: [[Window]] other fields */
    bool_byte	ctlopen;
    /*x: [[Window]] other fields */
    char		*dir;
    /*x: [[Window]] other fields */
    bool_byte	wctlopen;
    int	 	wctlready;
    /*e: [[Window]] other fields */

    //--------------------------------------------------------------------
    // Extra
    //--------------------------------------------------------------------
    /*s: [[Window]] extra fields */
    Ref;
    /*x: [[Window]] extra fields */
    QLock;
    /*e: [[Window]] extra fields */
};
/*e: struct Window */

/*s: struct Dirtab */
struct Dirtab
{
    char	*name;
    uchar	type;
    uint	qid;
    uint	perm;
};
/*e: struct Dirtab */

/*s: struct Fid */
struct Fid
{
    int		fid;

    int		busy;
    int		open;
    int		mode;

    Qid		qid;

    Window	*w;

    Dirtab	*dir;
    int		nrpart;
    uchar	rpart[UTFmax];

    // Extra
    Fid		*next;

};
/*e: struct Fid */

/*s: struct Xfid */
struct Xfid
{
        Ref;
        Xfid	*next;
        Xfid	*free;
        Fcall;
        Channel	*c;	/* chan(void(*)(Xfid*)) */

        Fid	*f;

        uchar	*buf;

        Filsys	*fs;

        QLock	active;
        int	flushing;	/* another Xfid is trying to flush us */
        int	flushtag;	/* our tag, so flush can find us */
        Channel	*flushc;/* channel(int) to notify us we're being flushed */
};
/*e: struct Xfid */


/*s: constant Nhash */
#define Nhash 16
/*e: constant Nhash */

/*s: struct Filsys */
struct Filsys
{
    fdt		cfd;
    fdt		sfd;

    int		pid;

    char	*user;

    Channel	*cxfidalloc;	/* chan(Xfid*) */

    // hash<fid, Fid> (next in bucket = Fid.next?)
    Fid		*fids[Nhash];
};
/*e: struct Filsys */


/*s: struct Timer */
struct Timer
{
    int		dt;
    int		cancel;
    Channel	*c;	/* chan(int) */
    Timer	*next;
};
/*e: struct Timer */

// draw.h
extern Display	*display;
extern Font	*font;


extern Mousectl	*mousectl;
extern Mouse	*mouse;
extern Keyboardctl	*keyboardctl;

//extern Image	*view;
extern Screen	*wscreen;
extern Cursor	boxcursor;
extern Cursor	crosscursor;
extern Cursor	sightcursor;
extern Cursor	whitearrow;
extern Cursor	query;
extern Cursor	*corners[9];

extern Image	*background;
//extern Image	*lightgrey;
extern Image	*red;

extern Window	**windows;

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
extern bool		errorshouldabort;
extern bool		menuing;
extern int		snarfversion;	/* updated each time it is written */
extern int		messagesize;	/* negotiated in 9P version setup */
/*e: windows/rio/dat.h */
