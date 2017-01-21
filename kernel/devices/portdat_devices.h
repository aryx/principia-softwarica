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

// mouse/devmouse.c (used in portmouse.c, portkbd.c, ...)
extern int mouseshifted;


/*e: portdat_devices.h */
