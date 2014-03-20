
// sys/devroot.c
void		addbootfile(char*, uchar*, ulong);
// sys/devmnt.c
Chan*		mntauth(Chan*, char*);
long		mntversion(Chan*, char*, int, int);
void		muxclose(Mnt*);
// sys/devsrv.c
char*		srvname(Chan*);
// sys/devproc.c
int		procfdprint(Chan*, int, int, char*, int);
//TODO: mv in memory/
Segment*	data2txt(Segment*);
// sys/devrtc.c
//long		rtctime(void);

// screen/devdraw.c
void		drawactive(int);
void		drawcmap(void);
// mouse/devmouse.c
void		mouseresize(void);

// in screen/386/screen.c (but used in port)
void		getcolor(ulong, ulong*, ulong*, ulong*);
int		setcolor(ulong, ulong, ulong, ulong);
// in keyboard/386/kbd.c (but used in port)
int		kbdgetmap(uint, int*, int*, Rune*);
void		kbdputmap(ushort, ushort, Rune);

