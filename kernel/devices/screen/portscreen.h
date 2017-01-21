/*s: kernel/devices/screen/portscreen.h */
// This file used to contain the stuff now in vga.h, but the prototypes were
// VGA independent, so it is better to have a generic portscreen.h interface 
// and vga.h in a separate file.

// defined in portscreen.c, set in <arch>/screen.c, used by generic 
// portscreen.c stuff
extern Memimage *gscreen;
extern Memdata gscreendata;
extern Rectangle physgscreenr;  /* actual monitor size */

/* defined in <arch>/screen.c, needed by devdraw.c */
extern void 	flushmemscreen(Rectangle);
extern byte* 	attachscreen(Rectangle*, ulong*, int*, int*, int*);
extern void 	blankscreen(bool);

extern void		getcolor(ulong, ulong*, ulong*, ulong*);
extern int		setcolor(ulong, ulong, ulong, ulong);

// this used to be a macro, but then it was preventing this file to be generic
extern bool ishwimage(Memimage*);

/*s: struct Cursorinfo */
struct Cursorinfo {
  Cursor;
  Lock;
};
/*e: struct Cursorinfo */
typedef struct Cursorinfo Cursorinfo;

/*s: global signature cursor */
extern Cursorinfo 	cursor;
/*e: global signature cursor */

/* defined in devmouse.c, needed by ?? */ // just enough Mouse getters/setters
extern Point 	mousexy(void);

/* defined in xxxscreen.c, needed by devmouse.c */
extern Cursor 		arrow;

extern void 	ksetcursor(Cursor*);
extern int  	cursoron(int);
extern void 	cursoroff(int);

/*e: kernel/devices/screen/portscreen.h */
