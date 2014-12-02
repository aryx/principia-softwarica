/*s: kernel/devices/screen/386/vga.h */

/*s: enum vgaports */
/*
 * Generic VGA registers.
 */
enum {
  MiscW   = 0x03C2, /* Miscellaneous Output (W) */
  MiscR   = 0x03CC, /* Miscellaneous Output (R) */
  Status0   = 0x03C2, /* Input status 0 (R) */
  Status1   = 0x03DA, /* Input Status 1 (R) */
  FeatureR  = 0x03CA, /* Feature Control (R) */
  FeatureW  = 0x03DA, /* Feature Control (W) */

  Seqx    = 0x03C4, /* Sequencer Index, Data at Seqx+1 */
  Crtx    = 0x03D4, /* CRT Controller Index, Data at Crtx+1 */
  Grx   = 0x03CE, /* Graphics Controller Index, Data at Grx+1 */
  Attrx   = 0x03C0, /* Attribute Controller Index and Data */

  PaddrW    = 0x03C8, /* Palette Address Register, write */
  Pdata     = 0x03C9, /* Palette Data Register */
  Pixmask   = 0x03C6, /* Pixel Mask Register */
  PaddrR    = 0x03C7, /* Palette Address Register, read */
  Pstatus   = 0x03C7, /* DAC Status (RO) */
/*e: enum vgaports */

/*s: enum vgamisc */
  Pcolours  = 256,    /* Palette */
  Pred    = 0,
  Pgreen  = 1,
  Pblue   = 2,

  Pblack    = 0x00,
  Pwhite    = 0xFF,
};
/*e: enum vgamisc */

/*s: function VGAMEM */
#define VGAMEM()  0xA0000
/*e: function VGAMEM */

/*s: function vgao */
#define vgao(port, data)  outb(port, data)
/*e: function vgao */
//#define vgai(port)    inb(port)
extern int vgaxi(long, uchar);
extern int vgaxo(long, uchar, uchar);

// forward decl
typedef struct VGAdev VGAdev;
typedef struct VGAcur VGAcur;
typedef struct VGAscr VGAscr;

/*s: struct VGAdev */
struct VGAdev {
  char* name;

  void  (*enable)(VGAscr*);
  void  (*disable)(VGAscr*);

  void  (*page)(VGAscr*, int);
  void  (*linear)(VGAscr*, int, int);
 
  // optional
  void  (*drawinit)(VGAscr*);
  int   (*fill)(VGAscr*, Rectangle, ulong);
  void  (*ovlctl)(VGAscr*, Chan*, void*, int);
  int   (*ovlwrite)(VGAscr*, void*, int, vlong);

  void  (*flush)(VGAscr*, Rectangle);
};
/*e: struct VGAdev */

/*s: struct VGAcur */
struct VGAcur {
  char* name;

  void  (*enable)(VGAscr*);
  void  (*disable)(VGAscr*);
  void  (*load)(VGAscr*, Cursor*);
  int   (*move)(VGAscr*, Point);

  // optional
  int doespanning;
};
/*e: struct VGAcur */

/*s: struct VGAscr */
struct VGAscr {

  ulong paddr;    /* frame buffer */
  void* vaddr;
  int   apsize;

  Memsubfont* memdefont;

  Cursor;
  // the cursor device methods (software cursor or hardware support)
  VGAcur* cur;

  ulong colormap[Pcolours][3];
  int palettedepth;

  // the vga device methods
  VGAdev* dev;

  Pcidev* pci;
  ulong io;       /* device specific registers */
  ulong *mmio;

  ulong storage;
  bool useflush;

  ulong id; /* internal identifier for driver use */

  // why here? why not in VGAdev?
  /*s: [[VGAscr]] optional methods */
  void  (*blank)(VGAscr*, int);
  /*x: [[VGAscr]] optional methods */
  int (*fill)(VGAscr*, Rectangle, ulong);
  int (*scroll)(VGAscr*, Rectangle, Rectangle);
  /*e: [[VGAscr]] optional methods */

  /*s: [[VGAscr]] other fields */
  int isblank;
  /*x: [[VGAscr]] other fields */
  int overlayinit;
  /*e: [[VGAscr]] other fields */

  // Extra
  Lock  devlock;
};
/*e: struct VGAscr */

//!!!
extern VGAscr vgascreen;

/*s: enum _anon_ (kernel/devices/screen/386/screen.h)2 */
enum {
  Backgnd   = 0,  /* black */
};
/*e: enum _anon_ (kernel/devices/screen/386/screen.h)2 */


/* screen.c */
extern bool    hwaccel;  /* use hw acceleration; default on */
extern bool    hwblank;  /* use hw blanking; default on */
extern bool    panning;  /* use virtual screen panning; default off */

extern void 	addvgaseg(char*, ulong, ulong);

extern int  screensize(int, int, int, ulong);
extern int  screenaperture(int, int);

/* devdraw.c */
extern void deletescreenimage(void);
extern void resetscreenimage(void);
extern int drawidletime(void);
extern void drawblankscreen(int);
extern ulong  blanktime;
extern QLock  drawlock;
//extern int drawhasclients(void);
//extern void setscreenimageclipr(Rectangle);
//extern void drawflush(void);

/* vga.c */
extern void vgascreenwin(VGAscr*);
extern void vgaimageinit(ulong);
extern void vgalinearpciid(VGAscr*, int, int);
extern void vgalinearaddr(VGAscr*, ulong, int);
extern void vgablank(VGAscr*, int);
extern Lock vgascreenlock;

// software cursors
extern VGAcur swcursor;
extern void swcursorinit(void);
//extern void swcursorhide(void);
//extern void swcursoravoid(Rectangle);
//extern void swcursorunhide(void);
/*e: kernel/devices/screen/386/vga.h */