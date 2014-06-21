/*s: portfns_devices.h */

// sys/devroot.c
void    addbootfile(char*, uchar*, ulong);
// sys/devmnt.c
Chan*   mntauth(Chan*, char*);
long    mntversion(Chan*, char*, int, int);
// sys/devsrv.c
char*   srvname(Chan*);
// sys/devproc.c
int   procfdprint(Chan*, int, int, char*, int);
// sys/devrtc.c
//long    rtctime(void);
// sys/devenv.c
void    envcpy(Egrp*, Egrp*);
void    ksetenv(char*, char*, int);
char*   getconfenv(void);


// screen/devdraw.c
void    drawactive(bool);
void    drawcmap(void);
// mouse/devmouse.c
void    mouseresize(void);

// in screen/386/screen.c (but used in port)
void    getcolor(ulong, ulong*, ulong*, ulong*);
int   setcolor(ulong, ulong, ulong, ulong);
// in keyboard/386/kbd.c (but used in port)
int   kbdgetmap(uint, int*, int*, Rune*);
void    kbdputmap(ushort, ushort, Rune);

/*e: portfns_devices.h */
