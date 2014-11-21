
typedef struct Cursorinfo Cursorinfo;
struct Cursorinfo {
	Cursor;
	Lock;
};

/* devmouse.c */
extern void mousetrack(int, int, int, int);
extern Point mousexy(void);
extern void mouseaccelerate(int);

// specific? maybe should not be in this interface file
extern int m3mouseputc(Queue*, int);
extern int m5mouseputc(Queue*, int);
extern int mouseputc(Queue*, int);

extern Cursorinfo cursor;
extern Cursor arrow;

/* mouse.c */
//kmousectl now
extern void mousectl(Cmdbuf*);
//I think I've moved it
extern void mouseresize(void);

/* screen.c */
extern void	blankscreen(int);
extern void	flushmemscreen(Rectangle);
extern uchar*	attachscreen(Rectangle*, ulong*, int*, int*, int*);

//ksetcursor now
extern void	setcursor(Cursor*);
extern int	cursoron(int);
extern void	cursoroff(int);

/* devdraw.c */
extern QLock	drawlock;

#define ishwimage(i)	1		/* for ../port/devdraw.c */
