/*s: portdat_devices.h */
/*
 *  hardware info about a device
 */
/*s: struct Devport */
struct Devport {
    ulong port; 
    int size;
};
/*e: struct Devport */

/*s: struct DevConf */
struct DevConf
{
    ulong intnum;     /* interrupt number */
    char  *type;      /* card type, malloced */
    int nports;     /* Number of ports */
    Devport *ports;     /* The ports themselves */
};
/*e: struct DevConf */

// keyboard

/*s: enum kbscan */
/* kbscans indices */
enum KbscanKind {
    KbInt=    0,          
    KbExt,

    KbNscans,
};
/*e: enum kbscan */

/*s: struct Kbscan */
struct Kbscan {
    bool ctl;
    bool shift;
    bool caps;
    bool alt;
    bool altgr;
    bool num;
    /*s: [[Kbscan]] other fields */
    bool esc1;
    int esc2;
    /*x: [[Kbscan]] other fields */
    int buttons;
    /*x: [[Kbscan]] other fields */
    bool collecting;
    int nk;
    Rune    kc[5];
    /*e: [[Kbscan]] other fields */
};
/*e: struct Kbscan */

// keyboard/portkbd.c
extern Kbscan kbscans[KbNscans];

// mouse

/*
 *  mouse types
 */
enum MouseType
{
    Mouseother= 0,
    Mouseserial=    1,
    MousePS2=   2,
};

// mouse/devmouse.c (used in portmouse.c, portkbd.c, ...)
extern int mouseshifted;

// mouse/portmouse.c (used in <arch>/mouse.c)
extern int mousetype;
extern int packetsize;
extern bool mousehwaccel;
extern bool intellimouse;

/* devdraw.c */ // used in some <arch>/screen.c
extern QLock	drawlock;
// for gscreen, gscreendata, see portscreen.h

/*e: portdat_devices.h */
