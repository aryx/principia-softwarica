/*s: portfns_devices.h */

// sys/devroot.c
extern void    addbootfile(char*, uchar*, ulong);
// sys/devmnt.c
extern Chan*   mntauth(Chan*, char*);
extern long    mntversion(Chan*, char*, int, int);
// sys/devsrv.c
extern char*   srvname(Chan*);
// sys/devproc.c
extern int   procfdprint(Chan*, int, int, char*, int);
// sys/devrtc.c
//long    rtctime(void);
// sys/devenv.c
extern void    envcpy(Egrp*, Egrp*);
extern void    ksetenv(char*, char*, int);
extern char*   getconfenv(void);


// screen/devdraw.c
extern void    drawactive(bool);
extern void    drawcmap(void);

// mouse/devmouse.c (called from portmouse.c or <arch>/mouse.c)
extern void mouseresize(void);
extern void mouseaccelerate(int x);
extern void mousetrack(int dx, int dy, int b, int msec);
// to call to let know devmouse.c about some mouse related keyboard events
extern void (*kbdmouse)(int);

// mouse/portmouse.c (called by devmouse.c for arch-specific mouse settings)
extern void kmousectl(Cmdbuf*);

// keyboard/portkbd.c
extern void kbdputsc(byte, int);
extern int  kbdgetmap(uint, int*, int*, Rune*);
extern void kbdputmap(ushort, ushort, Rune);

// in screen/386/screen.c (but used in port)
extern void  getcolor(ulong, ulong*, ulong*, ulong*);
extern int   setcolor(ulong, ulong, ulong, ulong);

/*e: portfns_devices.h */
