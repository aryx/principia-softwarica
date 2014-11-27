/*s: kernel/devices/screen/screen.h */
// this used to contain the stuff now in vga.h but those prototypes are
// VGA independent, so better to have a generic screen.h interface

// forward decls
typedef struct Cursorinfo Cursorinfo;

/* xxxscreen.c */
extern void 	flushmemscreen(Rectangle);
extern uchar* 	attachscreen(Rectangle*, ulong*, int*, int*, int*);
extern void 	blankscreen(bool);
extern void		getcolor(ulong, ulong*, ulong*, ulong*);
extern int		setcolor(ulong, ulong, ulong, ulong);

// this used to be a macro, but then it was forbidding this file to be generic
//#define ishwimage(i)  (vgascreen[0].gscreendata && (i)->data->bdata == vgascreen[0].gscreendata->bdata)
extern bool ishwimage(Memimage*);



/*s: struct Cursorinfo */
struct Cursorinfo {
  Cursor;
  Lock;
};
/*e: struct Cursorinfo */

/* devmouse.c */ // just enough Mouse getters/setters
extern Point 	mousexy(void);
extern void 	mouseaccelerate(int);
extern void 	mousetrack(int, int, int, int);
extern Cursorinfo 	cursor;
/* xxxmouse.c */
extern void kmousectl(Cmdbuf*);
//now in portfns_devices.h: extern void mouseresize(void);
/* xxxscreen.c */
extern Cursor 		arrow;
extern void 	ksetcursor(Cursor*);
extern int  	cursoron(int);
extern void 	cursoroff(int);

/*e: kernel/devices/screen/screen.h */
