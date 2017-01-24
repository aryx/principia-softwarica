/*s: kernel/devices/screen/portscreen.h */
// This file used to contain the stuff in pc/screen.h, but many prototypes were
// VGA independent, so it is better to have a generic portscreen.h interface 
// and VGA stuff in pc/screen.h in a separate file.

// Image

// defined in portscreen.c, set in <arch>/screen.c, 
// used by devmouse.c, swcursor.c, more?
extern Memimage *gscreen;
extern Memdata gscreendata;
extern Rectangle physgscreenr;  /* actual monitor size */

/* defined in <arch>/screen.c, used by devdraw.c */
extern byte* arch_attachscreen(Rectangle*, ulong*, int*, int*, int*);
extern void  arch_flushmemscreen(Rectangle);
extern void  arch_blankscreen(bool);
extern void	 arch_getcolor(ulong, ulong*, ulong*, ulong*);
extern int	 arch_setcolor(ulong, ulong, ulong, ulong);
// this used to be a macro, but then it was preventing this file to be generic
extern bool  arch_ishwimage(Memimage*);

// Cursor

/*s: struct Cursorinfo */
struct Cursorinfo {
  Cursor;
  Lock;
};
/*e: struct Cursorinfo */
typedef struct Cursorinfo Cursorinfo;

/*s: global signature cursor */
// defined in devmouse.c, set in <arch>/screen.c
extern Cursorinfo 	cursor;
/*e: global signature cursor */

// swcursor.c
extern void swcursorhide(void);
extern void swcursoravoid(Rectangle);
extern void swcursordraw(void);
extern void swload(Cursor *curs);
extern int swmove(Point p);
extern void swcursorinit(void);
extern Cursor swcursor_arrow;
extern bool swenabled;

/* defined in <arch>/screen.c, needed by devmouse.c */
extern Cursor 		arch_arrow;

/* defined in devmouse.c, needed by ?? */ // just enough Mouse getters/setters
extern Point 	mousexy(void);

extern void 	arch_ksetcursor(Cursor*);
extern int  	arch_cursoron(int);
extern void 	arch_cursoroff(int);

/*e: kernel/devices/screen/portscreen.h */
