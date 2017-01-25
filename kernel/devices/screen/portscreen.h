/*s: kernel/devices/screen/portscreen.h */
// The content of this file used to be in pc/screen.h, but many prototypes were
// VGA independent, so it is better to have a generic portscreen.h interface 
// and VGA-specific stuff in pc/screen.h in a separate file.

// Image

// portscreen.c, set in <arch>/screen.c (used by devmouse.c, swcursor.c)
extern Memimage* gscreen;
extern Memdata   gscreendata;
extern Rectangle physgscreenr;  /* actual monitor size */

/* <arch>/screen.c  (used by devdraw.c) */
extern byte* arch_attachscreen(Rectangle*, ulong*, int*, int*, int*);
extern void  arch_flushmemscreen(Rectangle);
extern void  arch_blankscreen(bool);
extern void	 arch_getcolor(ulong, ulong*, ulong*, ulong*);
extern int	 arch_setcolor(ulong, ulong, ulong, ulong);
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

// swcursor.c (called from <arch>/screen.c from arch_cursorxxx)
extern void swcursor_init(void);
extern void swcursor_hide(void);
extern void swcursor_draw(void);
extern int  swcursor_move(Point p);
extern void swcursor_load(Cursor *curs);
extern void swcursor_avoid(Rectangle);
//extern Cursor swcursor_arrow;

/* <arch>/screen.c (needed by devmouse.c) */
extern Cursor 		arch_arrow;

/* devmouse.c (needed by ??) */ // just enough Mouse getters/setters
extern Point 	mousexy(void);

// <arch>/screen.c (called from devmouse.c)
extern void 	arch_ksetcursor(Cursor*);
extern int  	arch_cursoron(int);
extern void 	arch_cursoroff(int);

// Graphical console

// swconsole.c (used from <arch>/screen.c)
extern Memsubfont *swconsole_memdefont;
extern Memimage *swconsole_conscol;
extern Point     swconsole_curpos;
extern Rectangle swconsole_window;

extern void swconsole_init(void);
extern void swconsole_screenputs(char *s, int n);

/*e: kernel/devices/screen/portscreen.h */
