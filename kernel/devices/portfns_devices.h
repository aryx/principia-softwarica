/*s: portfns_devices.h */

// sys/devroot.c
extern void  addbootfile(char*, uchar*, ulong);

// sys/devmnt.c
extern Chan* mntauth(Chan*, char*);
extern long  mntversion(Chan*, char*, int, int);

// sys/devsrv.c
extern char* srvname(Chan*);

// sys/devproc.c
extern int   procfdprint(Chan*, int, int, char*, int);

// sys/devenv.c
extern void  envcpy(Egrp*, Egrp*);
extern void  ksetenv(char*, char*, int);
extern char* getconfenv(void);



// screen/devdraw.c
extern void  drawactive(bool);
extern void  drawcmap(void);

// mouse/devmouse.c (called from portmouse.c or <arch>/mouse.c)
extern void  mouseresize(void);
extern void  mouseaccelerate(int x);
extern void  mousetrack(int dx, int dy, int b, int msec);
// to call to let know devmouse.c about some mouse related keyboard events
extern void  (*kbdmouse)(int);

// sys/devrtc.c
//long    rtctime(void);


// mouse/portmouse.c (called by devmouse.c for arch-specific mouse settings)
extern void  kmousectl(Cmdbuf*);

// keyboard/latin1.c (used by portkbd.c)
//@Scheck: def currently skipped, see skip_list.txt, because of unicode
extern long  latin1(Rune*, int);

// keyboard/portkbd.c
extern void  kbdputsc(byte, int);
extern int   kbdgetmap(uint, int*, int*, Rune*);
extern void  kbdputmap(ushort, ushort, Rune);


// mouse/<arch>/mouse.c (called from portmouse.c)
extern void  arch_ps2mouse(void);
extern void  arch_setaccelerated(void);
extern void  arch_setlinear(void);
extern void  arch_setres(int n);
extern void  arch_setintellimouse(void);
extern void  arch_resetmouse(void);

// keyboard/<arch>/kbd.c (called from portkbd.c)
extern void  arch_setleds(Kbscan *kbscan);

// screen/<arch>/screen.c (called from main)
extern void  arch__screeninit(void);

// sys/<arch>/devarch.c (called from arch specific)
extern Dirtab* addarchfile(char*, int, long(*)(Chan*,void*,long,vlong), long(*)(Chan*,void*,long,vlong));

/*e: portfns_devices.h */
