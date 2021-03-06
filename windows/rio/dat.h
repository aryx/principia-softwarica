/*s: windows/rio/dat.h */

// forward decls
typedef	struct	Window Window;
typedef	struct	Wctlmesg Wctlmesg;
typedef	struct	Filsys Filsys;
typedef	struct	Fid Fid;
typedef	struct	Xfid Xfid;
typedef	struct	Consreadmesg Consreadmesg;
typedef	struct	Conswritemesg Conswritemesg;
typedef	struct	Stringpair Stringpair;
typedef	struct	Dirtab Dirtab;
typedef	struct	Mouseinfo	Mouseinfo;
typedef	struct	Mousereadmesg Mousereadmesg;
typedef	struct	Mousestate	Mousestate;
typedef	struct	Ref Ref;
typedef	struct	Timer Timer;

//----------------------------------------------------------------------------
// Data structures and constants
//----------------------------------------------------------------------------

/*s: enum [[qid]] */
enum Qxxx
{
    Qdir,			/* /dev for this window */
    /*s: [[qid]] cases */
    Qmouse,
    /*x: [[qid]] cases */
    Qcons,
    /*x: [[qid]] cases */
    Qconsctl,
    /*x: [[qid]] cases */
    Qcursor,
    /*x: [[qid]] cases */
    Qwinname,
    /*x: [[qid]] cases */
    Qwindow,
    /*x: [[qid]] cases */
    Qtext,
    /*x: [[qid]] cases */
    Qwinid,
    /*x: [[qid]] cases */
    Qlabel,
    /*x: [[qid]] cases */
    Qscreen,
    /*x: [[qid]] cases */
    Qwsys,		/* directory of window directories */
    /*x: [[qid]] cases */
    Qwsysdir,		/* window directory, child of wsys */
    /*x: [[qid]] cases */
    Qwctl,
    /*x: [[qid]] cases */
    Qsnarf,
    /*x: [[qid]] cases */
    Qwdir,
    /*x: [[qid]] cases */
    Qkbdin,
    /*e: [[qid]] cases */

    QMAX,
};
/*e: enum [[qid]] */

/*s: function [[QID]] */
#define	QID(winid,qxxx)	((winid<<8)|(qxxx))
/*e: function [[QID]] */
/*s: function [[WIN]] */
#define	WIN(q)	((((ulong)(q).path)>>8) & 0xFFFFFF)
/*e: function [[WIN]] */
/*s: function [[FILE]] */
#define	FILE(q)	(((ulong)(q).path) & 0xFF)
/*e: function [[FILE]] */


/*s: enum [[_anon_ (windows/rio/dat.h)]]2 */
enum
{
    Kscrolloneup = KF|0x20,
    Kscrollonedown = KF|0x21,
};
/*e: enum [[_anon_ (windows/rio/dat.h)]]2 */

/*s: constant [[STACK]] */
#define	STACK	8192
/*e: constant [[STACK]] */

/*s: enum [[_anon_ (windows/rio/dat.h)]]3 */
enum
{
    /*s: constant [[Selborder]] */
    Selborder		= 4,	/* border of selected window */
    /*e: constant [[Selborder]] */
    /*s: constant [[Unselborder]] */
    Unselborder		= 1,	/* border of unselected window */
    /*e: constant [[Unselborder]] */
    /*s: constants Scrollxxx */
    Scrollwid 		= 12,	/* width of scroll bar */
    /*x: constants Scrollxxx */
    Scrollgap 		= 4,	/* gap right of scroll bar */
    /*e: constants Scrollxxx */
    /*s: constant [[BIG]] */
    BIG			= 3,	/* factor by which window dimension can exceed screen */
    /*e: constant [[BIG]] */
};
/*e: enum [[_anon_ (windows/rio/dat.h)]]3 */

/*s: constant [[DEBUG]] */
#define DEBUG false
/*e: constant [[DEBUG]] */

/*s: enum [[wctlmesgkind]] */
enum ControlMessage	/* control messages */
{
    Reshaped, // Resized, Hide/Expose
    Moved,
    /*s: [[Wctlmesgkind]] cases */
    Deleted,
    /*x: [[Wctlmesgkind]] cases */
    Exited,
    /*x: [[Wctlmesgkind]] cases */
    Movemouse,
    /*x: [[Wctlmesgkind]] cases */
    Rawon,
    Rawoff,
    /*x: [[Wctlmesgkind]] cases */
    Holdon,
    Holdoff,
    /*x: [[Wctlmesgkind]] cases */
    Wakeup,
    /*x: [[Wctlmesgkind]] cases */
    Refresh,
    /*e: [[Wctlmesgkind]] cases */
};
/*e: enum [[wctlmesgkind]] */

/*s: struct [[Wctlmesg]] */
struct Wctlmesg
{
    // enum<Wctlmesgkind>
    int		type;

    Rectangle	r;
    Image	*image;
};
/*e: struct [[Wctlmesg]] */

/*s: struct [[Conswritemesg]] */
struct Conswritemesg
{
    // chan<ref<array<Rune>> (listener = winctl, sender = xfidwrite(Qcons))
    Channel	*cw;		/* chan(Stringpair) */
};
/*e: struct [[Conswritemesg]] */

/*s: struct [[Consreadmesg]] */
struct Consreadmesg
{
    // chan<ref<array<Rune>> (listener = winctl, sender = xfidread(Qcons))
    Channel	*c1;		/* chan(tuple(char*, int) == Stringpair) */
    // chan<ref<array<Rune>> (listener = xdidread(Qcons), sender = winctl)
    Channel	*c2;		/* chan(tuple(char*, int) == Stringpair) */
};
/*e: struct [[Consreadmesg]] */

/*s: struct [[Mousereadmesg]] */
struct Mousereadmesg
{
    // chan<Mouse> (listener = xfidread(Qmouse), sender = winctl)
    Channel	*cm;		/* chan(Mouse) */
};
/*e: struct [[Mousereadmesg]] */

/*s: struct [[Stringpair]] */
struct Stringpair	/* rune and nrune or byte and nbyte */
{
    void	*s;
    int		ns;
};
/*e: struct [[Stringpair]] */

/*s: struct [[Mousestate]] */
struct Mousestate
{
    Mouse;
    ulong	counter;	/* serial no. of mouse event */
};
/*e: struct [[Mousestate]] */

/*s: struct [[Mouseinfo]] */
struct Mouseinfo
{
    // queue of mouse clicks and releases
    Mousestate	queue[16];

    // consumer
    int	ri;	/* read index into queue */
    // producer
    int	wi;	/* write index */

    bool	qfull;/* filled the queue; no more recording until client comes back */	
    /*s: [[Mouseinfo]] other fields */
    ulong	counter;	/* serial no. of last mouse event we received */
    ulong	lastcounter;/* serial no. of last mouse event sent to client */
    /*x: [[Mouseinfo]] other fields */
    int	lastb;	/* last button state we received */
    /*e: [[Mouseinfo]] other fields */
};	
/*e: struct [[Mouseinfo]] */

/*s: struct [[Window]] */
struct Window
{
    //--------------------------------------------------------------------
    // ID
    //--------------------------------------------------------------------
    /*s: [[Window]] id fields */
    int	    id;       // visible through /mnt/wsys/winid
    char    name[32]; // visible through /mnt/wsys/winname
    /*x: [[Window]] id fields */
    char    *label;   // writable through /mnt/wsys/label
    /*x: [[Window]] id fields */
    uint		namecount;
    /*e: [[Window]] id fields */
    
    //--------------------------------------------------------------------
    // Graphics
    //--------------------------------------------------------------------
    /*s: [[Window]] graphics fields */
    // ref_own<Image>, public image for the window (name in /dev/winname)
    Image	*i;
    /*x: [[Window]] graphics fields */
    /*
     * Rio once used originwindow, so screenr could be different from i->r.
     * Now they're always the same but the code doesn't assume so.
    */
    Rectangle	screenr; /* screen coordinates of window */
    /*e: [[Window]] graphics fields */
    
    //--------------------------------------------------------------------
    // Mouse
    //--------------------------------------------------------------------
    /*s: [[Window]] mouse fields */
    // mc.c = chan<Mouse> (listener = winctl, sender = mousethread)
    Mousectl	mc;
    /*x: [[Window]] mouse fields */
    Cursor		cursor;
    // option<ref<Cursor>> (to Window.cursor when not None)
    Cursor		*cursorp;
    /*x: [[Window]] mouse fields */
    Mouseinfo	mouse;
    /*e: [[Window]] mouse fields */
   
    //--------------------------------------------------------------------
    // Keyboard
    //--------------------------------------------------------------------
    /*s: [[Window]] keyboard fields */
    // chan<Rune, 20> (listener = winctl, sender = keyboardthread)
    Channel	*ck;		/* chan(Rune[10]) */
    /*e: [[Window]] keyboard fields */
    
    //--------------------------------------------------------------------
    // Control
    //--------------------------------------------------------------------
    /*s: [[Window]] control fields */
    // chan<Wctlmesg, 20> (listener = winctl, sender = mousethread | ...)
    Channel	*cctl;		/* chan(Wctlmesg)[20] */
    /*e: [[Window]] control fields */

    //--------------------------------------------------------------------
    // Process
    //--------------------------------------------------------------------
    /*s: [[Window]] process fields */
    int		pid;
    /*x: [[Window]] process fields */
    // /proc/<pid>/notepg
    fdt	 	notefd;
    /*e: [[Window]] process fields */
    
    //--------------------------------------------------------------------
    // Config
    //--------------------------------------------------------------------
    /*s: [[Window]] config fields */
    bool	scrolling;
    /*x: [[Window]] config fields */
    bool	deleted;
    /*x: [[Window]] config fields */
    bool	rawing;
    /*x: [[Window]] config fields */
    bool	holding;
    /*e: [[Window]] config fields */

    //--------------------------------------------------------------------
    // Textual Window
    //--------------------------------------------------------------------
    /*s: [[Window]] textual window fields */
    /*s: [[Window]] textual window fields, text data */
    // growing_array<Rune> (size = Window.maxr)
    Rune		*r;
    uint		nr;	/* number of runes in window */
    uint		maxr;	/* number of runes allocated in r */
    /*e: [[Window]] textual window fields, text data */
    /*s: [[Window]] textual window fields, text cursors */
    // index in Window.r
    uint		q0; // cursor, where entered text go (and selection start)
    // index in Window.r
    uint		q1; // selection end or same value than q0 when no selection
    /*x: [[Window]] textual window fields, text cursors */
    // index in Window.r
    uint		qh; // output point
    /*e: [[Window]] textual window fields, text cursors */
    /*s: [[Window]] textual window fields, visible text */
    uint		org;
    /*e: [[Window]] textual window fields, visible text */
    /*s: [[Window]] textual window fields, graphics */
    Frame frm;
    /*x: [[Window]] textual window fields, graphics */
    Rectangle	scrollr;
    /*e: [[Window]] textual window fields, graphics */
    /*e: [[Window]] textual window fields */

    //--------------------------------------------------------------------
    // Graphical Window
    //--------------------------------------------------------------------
    /*s: [[Window]] graphical window fields */
    bool	mouseopen;
    /*x: [[Window]] graphical window fields */
    // growing_array<Rune> (size = Window.nraw)
    Rune		*raw;
    uint		nraw;
    /*e: [[Window]] graphical window fields */

    //--------------------------------------------------------------------
    // Misc
    //--------------------------------------------------------------------
    /*s: [[Window]] other fields */
    int	 	topped;
    /*x: [[Window]] other fields */
    char		*dir; // /dev/wdir
    /*x: [[Window]] other fields */
    bool	 	resized;
    /*x: [[Window]] other fields */
    // chan<chan<Mouse> > (listener = xfidread(Qmouse), sender = winctl)
    Channel		*mouseread;	/* chan(Mousereadmesg) */
    /*x: [[Window]] other fields */
    // chan<Consreadmesg> (listener = xfidread(Qcons), sender = winctl)
    Channel		*consread;	/* chan(Consreadmesg) */
    /*x: [[Window]] other fields */
    // chan<Conswritemesg> (listener = xfidwrite(Qcons), sender = winctl)
    Channel		*conswrite;	/* chan(Conswritemesg) */
    /*x: [[Window]] other fields */
    bool	ctlopen;
    /*x: [[Window]] other fields */
    Rectangle	lastsr;
    /*x: [[Window]] other fields */
    bool	wctlopen;
    bool 	wctlready;
    /*x: [[Window]] other fields */
    // chan<Consreadmesg> (listener = , sender = )
    Channel		*wctlread;	/* chan(Consreadmesg) */
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
/*e: struct [[Window]] */

/*s: struct [[Dirtab]] */
struct Dirtab
{
    char	*name;
    // bitset<enum<QTxxx>>
    byte	type;
    // enum<Qxxx>
    uint	qid;
    uint	perm;
};
/*e: struct [[Dirtab]] */

/*s: struct [[Fid]] */
struct Fid
{
    // the key
    int		fid;

    // the state
    bool	open;
    int		mode;

    /*s: [[Fid]] other fields */
    Qid		qid;
    /*x: [[Fid]] other fields */
    Window	*w;
    /*x: [[Fid]] other fields */
    // ref<Dirtab>
    Dirtab	*dir;
    /*x: [[Fid]] other fields */
    uchar	rpart[UTFmax];
    int		nrpart;
    /*e: [[Fid]] other fields */

    // Extra
    /*s: [[Fid]] extra fields */
    // list<Fid> (head = Filsys.fids[i])
    Fid		*next;
    /*x: [[Fid]] extra fields */
    bool	busy;
    /*e: [[Fid]] extra fields */
};
/*e: struct [[Fid]] */

/*s: struct [[Xfid]] */
struct Xfid
{
        // incoming parsed request
        Fcall req;
        // answer buffer
        byte	*buf;
 
        // handler to worker thread
        // chan<void(*)(Xfid*)> (listener = xfidctl, senders = filsysxxx)
        Channel	*c;	/* chan(void(*)(Xfid*)) */

        Fid	*f;

        Filsys	*fs;

        /*s: [[Xfid]] flushing fields */
        int	flushtag;	/* our tag, so flush can find us */
        // chan<int> (listener = ?, sender = ?)
        Channel	*flushc;/* channel(int) to notify us we're being flushed */
        bool	flushing;	/* another Xfid is trying to flush us */
        /*e: [[Xfid]] flushing fields */
        /*s: [[Xfid]] other fields */
        QLock	active;
        /*e: [[Xfid]] other fields */

        // Extra
        Ref;
        /*s: [[Xfid]] extra fields */
        Xfid	*next;
        Xfid	*free;
        /*e: [[Xfid]] extra fields */
};
/*e: struct [[Xfid]] */

/*s: constant [[Nhash]] */
#define Nhash 16
/*e: constant [[Nhash]] */

/*s: struct [[Filsys]] */
struct Filsys
{
    // client
    fdt		cfd;
    // server
    fdt		sfd;

    // ref_own<string>
    char	*user;

    // map<fid, Fid> (next in bucket = Fid.next)
    Fid		*fids[Nhash];

    /*s: [[Filsys]] other fields */
    // chan<ref<Xfid>> (listener = filsysproc, sender = xfidallocthread)
    Channel	*cxfidalloc;	/* chan(Xfid*) */
    /*e: [[Filsys]] other fields */
};
/*e: struct [[Filsys]] */

/*s: struct [[Timer]] */
struct Timer
{
    int		dt;
    int		cancel;
    Channel	*c;	/* chan(int) */
    Timer	*next;
};
/*e: struct [[Timer]] */

//----------------------------------------------------------------------------
// Globals
//----------------------------------------------------------------------------

// see also draw.h globals: display, font, view

// globals.c

extern Screen	*desktop;
extern Image	*background;
extern Image	*red;

extern Window	**windows;
extern int		nwindow;
extern Window	*input;

extern Window	*hidden[100];
extern int		nhidden;

extern Filsys	*filsys;

extern Keyboardctl	*keyboardctl;
extern Mousectl	*mousectl;
extern Mouse	*mouse;

extern Channel	*exitchan; // was static in rio.c
extern Channel*	winclosechan;
extern Channel*	deletechan;

extern int		snarfversion;	/* updated each time it is written */

extern int		sweeping;
extern bool		menuing;
extern int		scrolling;

extern char		*startdir;

// misc
extern Window	*wkeyboard;	/* window of simulated keyboard */
extern QLock	all;			/* BUG */
extern fdt		wctlfd;
extern int		maxtab;


// 9p.c
extern int		messagesize;	/* negotiated in 9P version setup */


// thread_mouse.c
extern Rectangle	viewr; // was static in rio.c

// data.c
extern Cursor	boxcursor;
extern Cursor	crosscursor;
extern Cursor	sightcursor;
extern Cursor	whitearrow;
extern Cursor	query;
extern Cursor	*corners[9];

// cursor.c
extern Cursor	*lastcursor; // was static in wind.c

// snarf.c
extern fdt		snarffd;
extern Rune*	snarf;
extern int		nsnarf;

// error.c
extern bool		errorshouldabort;
extern char Eperm[];
/*e: windows/rio/dat.h */
