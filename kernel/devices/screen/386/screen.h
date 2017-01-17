/*s: kernel/devices/screen/386/vga.h */

/*s: enum vgaports(x86) */
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
/*e: enum vgaports(x86) */

/*s: enum vgamisc(x86) */
  Pcolours  = 256,    /* Palette */

  Pred    = 0,
  Pgreen  = 1,
  Pblue   = 2,

  Pblack    = 0x00,
  Pwhite    = 0xFF,
};
/*e: enum vgamisc(x86) */

/*s: function VGAMEM(x86) */
#define VGAMEM()  0xA0000
/*e: function VGAMEM(x86) */

/*s: function vgao(x86) */
#define vgao(port, data)  outb(port, data)
/*e: function vgao(x86) */
//#define vgai(port)    inb(port)
extern int vgaxi(long, uchar);
extern int vgaxo(long, uchar, uchar);

// forward decl
typedef struct VGAdev VGAdev;
typedef struct VGAcur VGAcur;
typedef struct VGAscr VGAscr;

/*s: struct VGAdev(x86) */
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
/*e: struct VGAdev(x86) */

/*s: struct VGAcur(x86) */
struct VGAcur {
  char* name;

  void  (*enable)(VGAscr*);
  void  (*disable)(VGAscr*);

  int   (*move)(VGAscr*, Point);

  void  (*load)(VGAscr*, Cursor*);

  // optional
  int doespanning;
};
/*e: struct VGAcur(x86) */

/*s: struct VGAscr(x86) */
struct VGAscr {

  ulong paddr;    /* frame buffer */
  void* vaddr;

  int   apsize;

  Memsubfont* memdefont;

  /*s: [[VGAscr]] cursor fields */
  Cursor;
  // the cursor device methods (software cursor or hardware support)
  VGAcur* cur;
  /*e: [[VGAscr]] cursor fields */

  ulong colormap[Pcolours][3];
  int palettedepth;

  Pcidev* pci;
  ulong io;       /* device specific registers */
  ulong *mmio;

  ulong storage;
  bool useflush;

  ulong id; /* internal identifier for driver use */

  // the vga device methods
  VGAdev* dev;

  // why here? why not in VGAdev?
  /*s: [[VGAscr]] optional methods(x86) */
  void  (*blank)(VGAscr*, int);
  /*x: [[VGAscr]] optional methods(x86) */
  int (*fill)(VGAscr*, Rectangle, ulong);
  int (*scroll)(VGAscr*, Rectangle, Rectangle);
  /*e: [[VGAscr]] optional methods(x86) */

  /*s: [[VGAscr]] other fields(x86) */
  int isblank;
  /*x: [[VGAscr]] other fields(x86) */
  int overlayinit;
  /*e: [[VGAscr]] other fields(x86) */

  // Extra
  Lock  devlock;
};
/*e: struct VGAscr(x86) */

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
