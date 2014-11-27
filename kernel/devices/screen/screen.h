/*s: kernel/devices/screen/screen.h */
// this used to contain the stuff now in vga.h but those prototypes are
// VGA independent, so better to have a generic screen.h interface and vga.h
// in a separate file.

// defined in screen.c, set in xxxscreen.c, used by generic screen.c stuff
extern Memimage *gscreen;
extern Memdata gscreendata;
extern Rectangle physgscreenr;  /* actual monitor size */

/* defined in xxxscreen.c, needed by devdraw.c */
extern void 	flushmemscreen(Rectangle);
extern byte* 	attachscreen(Rectangle*, ulong*, int*, int*, int*);
extern void 	blankscreen(bool);
extern void		getcolor(ulong, ulong*, ulong*, ulong*);
extern int		setcolor(ulong, ulong, ulong, ulong);
// this used to be a macro, but then it was forbidding this file to be generic
extern bool ishwimage(Memimage*);



/*s: struct Cursorinfo */
struct Cursorinfo {
  Cursor;
  Lock;
};
/*e: struct Cursorinfo */
typedef struct Cursorinfo Cursorinfo;

/* defined in devmouse.c, needed by ?? */ // just enough Mouse getters/setters
extern Point 	mousexy(void);
extern void 	mouseaccelerate(int);
extern void 	mousetrack(int, int, int, int);
extern Cursorinfo 	cursor;
/* xxxmouse.c */
extern void kmousectl(Cmdbuf*);
/* defined in xxxscreen.c, needed by devmouse.c */
extern Cursor 		arrow;
extern void 	ksetcursor(Cursor*);
extern int  	cursoron(int);
extern void 	cursoroff(int);

//now in portfns_devices.h: extern void mouseresize(void);

/*e: kernel/devices/screen/screen.h */
