/*s: kernel/devices/screen/screen.h */
// this used to contain the stuff now in vga.h but better to split, cleaner

typedef struct Cursorinfo Cursorinfo;

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
extern void 	blankscreen(int);
extern uchar* 	attachscreen(Rectangle*, ulong*, int*, int*, int*);
extern void 	flushmemscreen(Rectangle);

extern Cursor 		arrow;

extern void 	ksetcursor(Cursor*);
extern int  	cursoron(int);
extern void 	cursoroff(int);

// this used to be a macro, but then it was forbidding this file to be generic
//#define ishwimage(i)  (vgascreen[0].gscreendata && (i)->data->bdata == vgascreen[0].gscreendata->bdata)
extern bool ishwimage(Memimage*);

/*e: kernel/devices/screen/screen.h */
