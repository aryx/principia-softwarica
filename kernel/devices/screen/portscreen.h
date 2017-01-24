/*s: kernel/devices/screen/portscreen.h */
// The content of this file used to be in pc/screen.h, but many prototypes were
// VGA independent, so it is better to have a generic portscreen.h interface 
// and VGA-specific stuff in pc/screen.h in a separate file.

// Image

// portscreen.c, set in <arch>/screen.c (used devmouse.c/swcursor.c)
extern Memimage *gscreen;
extern Memdata gscreendata;
extern Rectangle physgscreenr;  /* actual monitor size */

/* <arch>/screen.c  (used by devdraw.c) */
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
// devmouse.c (set in <arch>/screen.c)
extern Cursorinfo 	cursor;
/*e: global signature cursor */

// swcursor.c
extern void swcursor_hide(void);
extern void swcursor_avoid(Rectangle);
extern void swcursor_draw(void);
extern void swcursor_load(Cursor *curs);
extern int  swcursor_move(Point p);
extern void swcursor_init(void);
//extern Cursor swcursor_arrow;


/* <arch>/screen.c (needed by devmouse.c) */
extern Cursor 		arch_arrow;

/* devmouse.c (needed by ??) */ // just enough Mouse getters/setters
extern Point 	mousexy(void);

extern void 	arch_ksetcursor(Cursor*);
extern int  	arch_cursoron(int);
extern void 	arch_cursoroff(int);

/*e: kernel/devices/screen/portscreen.h */
