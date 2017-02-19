/*s: include/mouse.h */
#pragma src "/sys/src/libdraw"

typedef struct	Channel Channel;
typedef struct	Cursor Cursor;
typedef struct	Menu Menu;
typedef struct 	Mousectl Mousectl;

/*s: enum Click */
enum Click {
    CLICK_LEFT = 1,
    CLICK_MIDDLE = 2,
    CLICK_RIGHT = 4,
};
/*e: enum Click */

/*s: struct Mouse (include/mouse.h) */
struct	Mouse
{
    Point	xy;
    // bitset<enum<Click>>
    int	buttons;	/* bit array: LMR=124 */

    ulong	msec;
};
/*e: struct Mouse (include/mouse.h) */

/*s: struct Mousectl */
struct Mousectl
{
    Mouse;

    // /dev/mouse
    fdt		mfd;		/* to mouse file */ 
    // ref_own<filename>
    char	*file; // "/dev/mouse" usually

    // ref<Image>
    Image*	image;	/* of associated window/display */

    /*s: [[Mousectl]] channel fields */
    // chan<Mouse> (listener = user program, sender = _ioproc(mouse.c))
    Channel	*c;			/* chan(Mouse) */
    /*x: [[Mousectl]] channel fields */
    Channel	*resizec;	/* chan(int)[2] */
    /* buffered in case client is waiting for a mouse action before handling resize */
    /*e: [[Mousectl]] channel fields */
    /*s: [[Mousectl]] IO process field */
    int		pid;	/* of slave proc */
    /*e: [[Mousectl]] IO process field */
    /*s: [[Mousectl]] cursor field */
    // /dev/cursor
    fdt		cfd;		/* to cursor file */
    /*e: [[Mousectl]] cursor field */
};
/*e: struct Mousectl */

//TODO: mv in menu.h?
/*s: struct Menu (include/mouse.h) */
struct Menu
{
    char	**item;
    char	*(*gen)(int);
    int	lasthit;
};
/*e: struct Menu (include/mouse.h) */

/*
 * Mouse
 */
extern Mousectl*	initmouse(char*, Image*);
extern void			closemouse(Mousectl*);

extern int			readmouse(Mousectl*);
extern void			moveto(Mousectl*, Point);
extern void			setcursor(Mousectl*, Cursor*);

//TODO? mv in menu.h?
extern void			drawgetrect(Rectangle, int);
extern Rectangle	getrect(int, Mousectl*);

extern int	 		menuhit(int, Mousectl*, Menu*, Screen*);
/*e: include/mouse.h */
