/*s: include/frame.h */
#pragma	src	"/sys/src/libframe"
#pragma	lib	"libframe.a"

typedef struct Frbox Frbox;
typedef struct Frame Frame;

/*s: enum _anon_ (include/frame.h) */
enum FrameColors {
    BACK, // Background
    HIGH, // Background highlighted text
    BORD, // Border
    TEXT, // Text
    HTEXT, // Highlited text

    NCOL
};
/*e: enum _anon_ (include/frame.h) */

/*s: constant FRTICKW */
#define	FRTICKW	3
/*e: constant FRTICKW */

/*s: struct Frbox */
struct Frbox
{
    long		wid;		/* in pixels */

    long		nrune;		/* <0 ==> negate and treat as break char */
    union{
        // array<byte> UTF8?
        uchar	*ptr;
        struct{
            short	bc;	/* break char */
            short	minwid;
        };
    };
};
/*e: struct Frbox */

/*s: struct Frame */
struct Frame
{
    Image	*b;		    /* on which frame appears */
    Rectangle	r;		/* in which text appears */

    Font	*font;		/* of chars in the frame */
    Display	*display;	/* on which frame appears */

    /*s: [[Frame]] colors */
    Image	*cols[NCOL];	/* text and background colors */
    /*e: [[Frame]] colors */
    /*s: [[Frame]] text fields */
    ushort	nchars;		/* # runes in frame */
    /*x: [[Frame]] text fields */
    ushort	nlines;		/* # lines with text */
    /*x: [[Frame]] text fields */
    ushort	maxlines;	/* total # lines in frame */
    /*x: [[Frame]] text fields */
    ulong	p0, p1;		/* selection */
    /*x: [[Frame]] text fields */
    bool	modified;	/* changed since frselect() */
    /*x: [[Frame]] text fields */
    ushort	lastlinefull;	/* last line fills frame */
    /*x: [[Frame]] text fields */
    ushort	maxtab;		/* max size of tab, in pixels */
    /*e: [[Frame]] text fields */
    /*s: [[Frame]] tick fields */
    Image	*tick;	/* typing tick */
    Image	*tickback;	/* saved image under tick */
    /*x: [[Frame]] tick fields */
    bool	ticked;	/* flag: is tick onscreen? */
    /*e: [[Frame]] tick fields */
    /*s: [[Frame]] box fields */
    // growing_array<Frbox> (size = nalloc, unused after nbox)
    Frbox	*box;
    ushort	nbox;
    ushort nalloc;
    /*e: [[Frame]] box fields */
    /*s: [[Frame]] scroll */
    void (*scroll)(Frame*, int); /* scroll function provided by application */
    /*e: [[Frame]] scroll */

    /*s: [[Frame]] other fields */
    Rectangle	entire;		/* of full frame */
    /*e: [[Frame]] other fields */
};
/*e: struct Frame */

ulong	frcharofpt(Frame*, Point);
Point	frptofchar(Frame*, ulong);

int	frdelete(Frame*, ulong, ulong);
void	frinsert(Frame*, Rune*, Rune*, ulong);

void	frselect(Frame*, Mousectl*);
void	frselectpaint(Frame*, Point, Point, Image*);

void	frdrawsel(Frame*, Point, ulong, ulong, int);
Point 	frdrawsel0(Frame*, Point, ulong, ulong, Image*, Image*);

void	frinit(Frame*, Rectangle, Font*, Image*, Image**);
void	frsetrects(Frame*, Rectangle, Image*);
void	frclear(Frame*, int);

// private??? frame_private.h?
uchar	*_frallocstr(Frame*, unsigned);
void	_frinsure(Frame*, int, unsigned);
Point	_frdraw(Frame*, Point);
void	_frgrowbox(Frame*, int);
void	_frfreebox(Frame*, int, int);
void	_frmergebox(Frame*, int);
void	_frdelbox(Frame*, int, int);
void	_frsplitbox(Frame*, int, int);
int	_frfindbox(Frame*, int, ulong, ulong);
void	_frclosebox(Frame*, int, int);
int	_frcanfit(Frame*, Point, Frbox*);
void	_frcklinewrap(Frame*, Point*, Frbox*);
void	_frcklinewrap0(Frame*, Point*, Frbox*);
void	_fradvance(Frame*, Point*, Frbox*);
int	_frnewwid(Frame*, Point, Frbox*);
int	_frnewwid0(Frame*, Point, Frbox*);
void	_frclean(Frame*, Point, int, int);
void	_frdrawtext(Frame*, Point, Image*, Image*);
void	_fraddbox(Frame*, int, int);
Point	_frptofcharptb(Frame*, ulong, Point, int);
Point	_frptofcharnb(Frame*, ulong, int);
int	_frstrlen(Frame*, int);

void	frtick(Frame*, Point, int);
void	frinittick(Frame*);

void	frredraw(Frame*);

/*s: function NRUNE */
#define	NRUNE(b)	((b)->nrune < 0 ? 1 : (b)->nrune)
/*e: function NRUNE */
/*s: function NBYTE */
#define	NBYTE(b)	strlen((char*)(b)->ptr)
/*e: function NBYTE */
/*e: include/frame.h */
