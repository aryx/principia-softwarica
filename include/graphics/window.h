/*s: include/graphics/window.h */
#pragma src "/sys/src/libdraw"
#pragma lib "libdraw.a"
// This file assumes you have included draw.h before.

/*s: struct [[Screen]] */
struct Screen
{
    Display *display;   /* display holding data */
    int     id;         /* id of system-held Screen */

    Image   *image;     /* unused; for reference only */
    Image   *fill;      /* color to paint behind windows */
};
/*e: struct [[Screen]] */

// set by initdraw() automatically.
extern  Screen  *screen; // was called _screen before

// initdraw -> <>
/*s:  signature [[gengetwindow]] */
extern int  gengetwindow(Display*, char*, Image**, Screen**, int);
/*e:  signature [[gengetwindow]] */

//
// Base layer
//
/*s: signatures screen allocation functions */
extern Screen*  allocscreen(Image*, Image*, int);
extern int      freescreen(Screen*);
/*e: signatures screen allocation functions */
extern Screen*  publicscreen(Display*, int, ulong);

/*
 * Windows
 */
/*s: signatures window allocation functions */
extern Image*   allocwindow(Screen*, Rectangle, int, ulong);
/*e: signatures window allocation functions */
extern int  newwindow(char*);

/*s: signatures window stack manipulation functions */
extern void topwindow(Image*);
extern void bottomwindow(Image*);
/*e: signatures window stack manipulation functions */
extern void bottomnwindows(Image**, int);
extern void topnwindows(Image**, int);

/*s: signatures window other functions */
extern int  originwindow(Image*, Point, Point);
/*e: signatures window other functions */
/*e: include/graphics/window.h */
