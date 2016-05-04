/*s: include/window.h */
#pragma src "/sys/src/libdraw"
#pragma lib "libdraw.a"
// This file assumes you have included draw.h before.

/*s: struct Screen */
struct Screen
{
    Display	*display;	/* display holding data */
    int		id;			/* id of system-held Screen */

    Image	*image;		/* unused; for reference only */
    Image	*fill;		/* color to paint behind windows */
};
/*e: struct Screen */

//
// Desktop
//
extern Screen*	allocscreen(Image*, Image*, int);
extern int		freescreen(Screen*);
extern Screen*	publicscreen(Display*, int, ulong);

// set by initdraw() automatically.
extern	Screen	*screen; // was called _screen before

/*
 * Windows
 */
extern Image*	allocwindow(Screen*, Rectangle, int, ulong);

extern int	originwindow(Image*, Point, Point);
extern void	bottomnwindows(Image**, int);
extern void	bottomwindow(Image*);
extern void	topnwindows(Image**, int);
extern void	topwindow(Image*);

extern int	newwindow(char*);
extern int	getwindow(Display*, int);
extern int	gengetwindow(Display*, char*, Image**, Screen**, int);

/*e: include/window.h */
