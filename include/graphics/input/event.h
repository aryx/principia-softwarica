/*s: include/graphics/input/event.h */
#pragma src "/sys/src/libdraw"
#pragma lib "libdraw.a"

typedef struct	Cursor Cursor;
typedef struct	Event Event;
typedef struct	Menu Menu;
//typedef struct	Mouse Mouse;

/*s: type [[keys]] */
// bitset<Key>
typedef ulong keys;
/*e: type [[keys]] */

/*s: enum [[Key]] */
enum
{
    Emouse	= 1,
    Ekeyboard	= 2,
};
/*e: enum [[Key]] */

/*s: enum [[_anon_ (include/event.h)2]] */
enum
{
    MAXSLAVE = 32,
    EMAXMSG = 128+8192,	/* size of 9p header+data */
};
/*e: enum [[_anon_ (include/event.h)2]] */

/*s: type [[buttons]] */
// bitset<enum<Click>>
typedef int buttons;
/*e: type [[buttons]] */

/*s: struct Mouse (include/event.h) */
struct	Mouse
{
    int		buttons;	/* bit array: LMR=124 */
    Point	xy;
    ulong	msec;
};
/*e: struct Mouse (include/event.h) */

/*s: enum [[Click]] */
enum Click {
    CLICK_LEFT = 1,
    CLICK_MIDDLE = 2,
    CLICK_RIGHT = 4,
};
/*e: enum [[Click]] */

/*s: struct [[Event]] */
struct	Event
{
    int		kbdc;
    Mouse	mouse;

    int		n;		/* number of characters in message */
    void	*v;		/* data unpacked by general event-handling function */
    byte	data[EMAXMSG];	/* message from an arbitrary file descriptor */
};
/*e: struct [[Event]] */

/*s: struct Menu (include/event.h) */
struct Menu
{
    char	**item;
    char	*(*gen)(int);
    int	lasthit;
};
/*e: struct Menu (include/event.h) */

/*
 * Events
 */
extern void		einit(ulong);
extern ulong	estart(ulong, int, int);
extern ulong	estartfn(ulong, int, int, int (*fn)(int, Event*, uchar*, int));
extern ulong	 event(Event*);
extern ulong	 eread(ulong, Event*);
extern int	 ecanread(ulong);

extern Mouse	emouse(void);
extern int	 	ekbd(void);
extern ulong	etimer(ulong, int);

extern int	 	ecanmouse(void);
extern int	 	ecankbd(void);

//ugly!!!
extern void	 eresized(int);	/* supplied by user */

extern int	 	emenuhit(int, Mouse*, Menu*);
extern void	 	esetcursor(Cursor*);
extern void	 	emoveto(Point);
extern void		edrawgetrect(Rectangle, int);
extern int		ereadmouse(Mouse*);
extern int		eatomouse(Mouse*, char*, int);

extern Rectangle	getrect(int, Mouse*);
extern Rectangle	egetrect(int, Mouse*);

// from plan9front
extern int		eenter(char*, char*, int, Mouse*);

/*e: include/graphics/input/event.h */
