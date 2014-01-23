/*
 * Generic VGA registers.
 */
enum {
	MiscW		= 0x03C2,	/* Miscellaneous Output (W) */
	MiscR		= 0x03CC,	/* Miscellaneous Output (R) */
	Status0		= 0x03C2,	/* Input status 0 (R) */
	Status1		= 0x03DA,	/* Input Status 1 (R) */
	FeatureR	= 0x03CA,	/* Feature Control (R) */
	FeatureW	= 0x03DA,	/* Feature Control (W) */

	Seqx		= 0x03C4,	/* Sequencer Index, Data at Seqx+1 */
	Crtx		= 0x03D4,	/* CRT Controller Index, Data at Crtx+1 */
	Grx		= 0x03CE,	/* Graphics Controller Index, Data at Grx+1 */
	Attrx		= 0x03C0,	/* Attribute Controller Index and Data */

	PaddrW		= 0x03C8,	/* Palette Address Register, write */
	Pdata		= 0x03C9,	/* Palette Data Register */
	Pixmask		= 0x03C6,	/* Pixel Mask Register */
	PaddrR		= 0x03C7,	/* Palette Address Register, read */
	Pstatus		= 0x03C7,	/* DAC Status (RO) */

	Pcolours	= 256,		/* Palette */
	Red		= 0,
	Green		= 1,
	Blue		= 2,

	Pblack		= 0x00,
	Pwhite		= 0xFF,
};

enum {
	RefFreq		= 14318180,	/* External Reference Clock frequency */
	VgaFreq0	= 25175000,
	VgaFreq1	= 28322000,
};

enum {
	Namelen		= 32,
};

typedef struct Ctlr Ctlr;
typedef struct Vga Vga;

typedef struct Ctlr {
	char	name[Namelen+1];
	void	(*snarf)(Vga*, Ctlr*);
	void	(*options)(Vga*, Ctlr*);
	void	(*init)(Vga*, Ctlr*);
	void	(*load)(Vga*, Ctlr*);
	void	(*dump)(Vga*, Ctlr*);
	char*	type;

	ulong	flag;

	Ctlr*	link;
} Ctlr;

enum {					/* flag */
	Fsnarf		= 0x00000001,	/* snarf done */
	Foptions	= 0x00000002,	/* options done */
	Finit		= 0x00000004,	/* init done */
	Fload		= 0x00000008,	/* load done */
	Fdump		= 0x00000010,	/* dump done */
	Ferror		= 0x00000020, /* error during snarf */

	Hpclk2x8	= 0x00000100,	/* have double 8-bit mode */
	Upclk2x8	= 0x00000200,	/* use double 8-bit mode */
	Henhanced	= 0x00000400,	/* have enhanced mode */
	Uenhanced	= 0x00000800,	/* use enhanced mode */
	Hpvram		= 0x00001000,	/* have parallel VRAM */
	Upvram		= 0x00002000,	/* use parallel VRAM */
	Hextsid		= 0x00004000,	/* have external SID mode */
	Uextsid		= 0x00008000,	/* use external SID mode */
	Hclk2		= 0x00010000,	/* have clock-doubler */
	Uclk2		= 0x00020000,	/* use clock-doubler */
	Hlinear		= 0x00040000,	/* have linear-address mode */
	Ulinear		= 0x00080000,	/* use linear-address mode */
	Hclkdiv		= 0x00100000,	/* have a clock-divisor */
	Uclkdiv		= 0x00200000,	/* use clock-divisor */
	Hsid32		= 0x00400000,	/* have a 32-bit (as opposed to 64-bit) SID */
};

typedef struct Attr Attr;
typedef struct Attr {
	char*	attr;
	char*	val;

	Attr*	next;
} Attr;

typedef struct Mode {
	char	type[Namelen+1];	/* monitor type e.g. "vs1782" */
	char	size[Namelen+1];	/* size e.g. "1376x1024x8" */
	char	chan[Namelen+1];	/* channel descriptor, e.g. "m8" or "r8g8b8a8" */
	char name[Namelen+1];	/* optional */

	int	frequency;		/* Dot Clock (MHz) */
	int	deffrequency;		/* Default dot clock if calculation can't be done */
	int	x;			/* Horizontal Display End (Crt01), from .size[] */
	int	y;			/* Vertical Display End (Crt18), from .size[] */
	int	z;			/* depth, from .size[] */

	int	ht;			/* Horizontal Total (Crt00) */
	int	shb;			/* Start Horizontal Blank (Crt02) */
	int	ehb;			/* End Horizontal Blank (Crt03) */

	int	shs;			/* optional Start Horizontal Sync (Crt04) */
	int	ehs;			/* optional End Horizontal Sync (Crt05) */

	int	vt;			/* Vertical Total (Crt06) */
	int	vrs;			/* Vertical Retrace Start (Crt10) */
	int	vre;			/* Vertical Retrace End (Crt11) */

	int		vbs;		/* optional Vertical Blank Start */
	int		vbe;		/* optional Vertical Blank End */
	
	ulong	videobw;

	char	hsync;
	char	vsync;
	char	interlace;

	Attr*	attr;
} Mode;

/*
 * The sizes of the register sets are large as many SVGA and GUI chips have extras.
 * The Crt registers are ushorts in order to keep overflow bits handy.
 * The clock elements are used for communication between the VGA, RAMDAC and clock chips;
 * they can use them however they like, it's assumed they will be used compatibly.
 *
 * The mode->x, mode->y coordinates are the physical size of the screen. 
 * Virtx and virty are the coordinates of the underlying memory image.
 * This can be used to implement panning around a larger screen or to cope
 * with chipsets that need the in-memory pixel line width to be a round number.
 * For example, virge.c uses this because the Savage chipset needs the pixel 
 * width to be a multiple of 16.  Also, mga2164w.c needs the pixel width
 * to be a multiple of 128.
 *
 * Vga->panning differentiates between these two uses of virtx, virty.
 *
 * (14 October 2001, rsc) Most drivers don't know the difference between
 * mode->x and virtx, a bug that should be corrected.  Vga.c, virge.c, and 
 * mga2164w.c know.  For the others, the computation that sets crt[0x13]
 * should use virtx instead of mode->x (and maybe other places change too,
 * dependent on the driver).
 */
typedef struct Vga {
	uchar	misc;
	uchar	feature;
	uchar	sequencer[256];
	ushort	crt[256];
	uchar	graphics[256];
	uchar	attribute[256];
	uchar	pixmask;
	uchar	pstatus;
	uchar	palette[Pcolours][3];

	ulong	f[2];			/* clock */
	ulong	d[2];
	ulong	i[2];
	ulong	m[2];
	ulong	n[2];
	ulong	p[2];
	ulong	q[2];
	ulong	r[2];

	ulong	vma;			/* video memory linear-address alignment */
	ulong	vmb;			/* video memory linear-address base */
	ulong	apz;			/* aperture size */
	ulong	vmz;			/* video memory size */

	ulong	membw;			/* memory bandwidth, MB/s */

	long	offset;			/* BIOS string offset */
	char*	bios;			/* matching BIOS string */
	Pcidev*	pci;			/* matching PCI device if any */

	Mode*	mode;

	ulong	virtx;			/* resolution of virtual screen */
	ulong	virty;

	int	panning;		/* pan the virtual screen */

	Ctlr*	ctlr;
	Ctlr*	ramdac;
	Ctlr*	clock;
	Ctlr*	hwgc;
	Ctlr* vesa;
	Ctlr*	link;
	int	linear;
	Attr*	attr;

	void*	private;
} Vga;

/* clgd542x.c */
extern void clgd54xxclock(Vga*, Ctlr*);
extern Ctlr clgd542x;
extern Ctlr clgd542xhwgc;

/* data.c */
extern int cflag;
extern int dflag;
extern Ctlr *ctlrs[];
extern ushort dacxreg[4];

/* db.c */
extern char* dbattr(Attr*, char*);
extern int dbctlr(char*, Vga*);
extern Mode* dbmode(char*, char*, char*);
extern void dbdumpmode(Mode*);

/* error.c */
extern void error(char*, ...);
extern void trace(char*, ...);
extern int vflag, Vflag;


/* io.c */
extern uchar inportb(long);
extern void outportb(long, uchar);
extern ushort inportw(long);
extern void outportw(long, ushort);
extern ulong inportl(long);
extern void outportl(long, ulong);
extern char* vgactlr(char*, char*);
extern void vgactlw(char*, char*);
extern char* readbios(long, long);
extern void dumpbios(long);
extern void error(char*, ...);
extern void* alloc(ulong);
extern void printitem(char*, char*);
extern void printreg(ulong);
extern void printflag(ulong);
extern void setpalette(int, int, int, int);
extern int curprintindex;

/* main.c */
extern char* chanstr[];
extern void resyncinit(Vga*, Ctlr*, ulong, ulong);
extern void sequencer(Vga*, int);
extern void main(int, char*[]);
Biobuf stdout;

/* palette.c */
extern Ctlr palette;

/* pci.c */
typedef struct Pcidev Pcidev;

extern int pcicfgr8(Pcidev*, int);
extern int pcicfgr16(Pcidev*, int);
extern int pcicfgr32(Pcidev*, int);
extern void pcicfgw8(Pcidev*, int, int);
extern void pcicfgw16(Pcidev*, int, int);
extern void pcicfgw32(Pcidev*, int, int);
extern void pcihinv(Pcidev*);
extern Pcidev* pcimatch(Pcidev*, int, int);


/* vga.c */
extern uchar vgai(long);
extern uchar vgaxi(long, uchar);
extern void vgao(long, uchar);
extern void vgaxo(long, uchar, uchar);
extern Ctlr generic;

/* vesa.c */
extern Ctlr vesa;
extern Ctlr softhwgc;	/* has to go somewhere */
extern int dbvesa(Vga*);
extern Mode *dbvesamode(char*);
extern void vesatextmode(void);

#pragma	varargck	argpos	error	1
#pragma	varargck	argpos	trace	1
