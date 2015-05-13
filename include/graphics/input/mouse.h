/*s: include/mouse.h */
#pragma src "/sys/src/libdraw"

typedef struct	Channel Channel;
typedef struct	Cursor Cursor;
typedef struct	Menu Menu;
typedef struct 	Mousectl Mousectl;

enum click {
    CLICK_LEFT = 1,
    CLICK_MIDDLE = 2,
    CLICK_RIGHT = 4,
};

/*s: struct Mouse (include/mouse.h) */
struct	Mouse
{
    // bitset<enum<click>>
    int	buttons;	/* bit array: LMR=124 */
    Point	xy;
    ulong	msec;
};
/*e: struct Mouse (include/mouse.h) */

/*s: struct Mousectl */
struct Mousectl
{
    Mouse;

    Channel	*c;			/* chan(Mouse) */
    Channel	*resizec;	/* chan(int)[2] */
    /* buffered in case client is waiting for a mouse action before handling resize */

    char	*file;

    fdt		mfd;		/* to mouse file */
    fdt		cfd;		/* to cursor file */

    int		pid;	/* of slave proc */
    Image*	image;	/* of associated window/display */
};
/*e: struct Mousectl */

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

extern void			drawgetrect(Rectangle, int);
extern Rectangle	getrect(int, Mousectl*);

extern int	 		menuhit(int, Mousectl*, Menu*, Screen*);
/*e: include/mouse.h */
