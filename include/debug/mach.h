/*s: include/debug/mach.h */
/*
 *	Architecture-dependent application data
 */
#include "a.out.h"
//TODO: include "elf.h" too? and macho.h?

#pragma	src	"/sys/src/libmach"
#pragma	lib	"libmach.a"

/*
 *	Supported architectures:
 *		i386,
 *		arm,
 *              mips
 */

/*s: enum [[executable_type]] */
/* types of executables */
enum executable_type 
{
    FNONE = 0,		/* unidentified */

    FI386,			/* 8.out */
    FI386B,			/* I386 bootable */
    FARM,			/* 5.out */
    FARMB,			/* ARM bootable */
    FMIPS,			/* v.out */
    FMIPSB,			/* mips bootable */
    FMIPS2BE,		        /* 4.out */
};
/*e: enum [[executable_type]] */
/*s: enum [[machine_type]] */
/* machine types */
enum machine_type 
{
    MI386,
    MARM,
    MMIPS,
};
/*e: enum [[machine_type]] */
/*s: enum [[dissembler_type]] */
/* dissembler types */
enum dissembler_type
{
    ANONE = 0,		

    AI386,
    AI8086,			/* oh god */
    AARM,
    AMIPS,
    AMIPSCO,		/* native mips */
};
/*e: enum [[dissembler_type]] */
/*s: enum [[object_file_type]] */
/* object file types */
enum object_file_type
{
    Obj386 = 0,			/* .8 */
    ObjArm,			/* .5 */
    ObjMips,		/* .v */

    Maxobjtype,
};
/*e: enum [[object_file_type]] */
/*s: enum [[symbol_type]] */
/* symbol table classes */
enum symbol_type
{
    CNONE  = 0,	

    CAUTO,
    CPARAM,
    CSTAB,
    CTEXT,
    CDATA,

    CANY,			/* to look for any class */
};
/*e: enum [[symbol_type]] */

typedef	struct	Map	Map;
typedef struct	Symbol	Symbol;
typedef	struct	Reglist	Reglist;
typedef	struct	Mach	Mach;
typedef	struct	Machdata Machdata;
typedef struct  Fhdr Fhdr;

/*s: struct [[Map]] */
/*
 * 	Structure to map a segment to a position in a file
 */
struct Map {
    int	nsegs;			/* number of segments */
    struct segment {		/* per-segment map */
        char	*name;		/* the segment name */
        int	fd;		/* file descriptor */

        bool	inuse;		/* in use - not in use */
        bool	cache;		/* should cache reads? */

        uvlong	b;		/* base */
        uvlong	e;		/* end */
        vlong	f;		/* offset within file */

    } seg[1];			/* actually n of these */
};
/*e: struct [[Map]] */

/*s: struct [[Symbol]] */
/*
 *	Internal structure describing a symbol table entry
 */
struct Symbol {
    void 	*handle;		/* used internally - owning func */
    struct {
        char	*name;
        vlong	value;		/* address or stack offset */
        char	type;		/* as in a.out.h */
        char	class;		/* as above */
        int	index;		/* in findlocal, globalsym, textsym */
    };
};
/*e: struct [[Symbol]] */

/*s: struct [[Reglist]] */
/*
 *	machine register description
 */
struct Reglist {
    char	*rname;		/* register name */

    short	roffs;		/* offset in u-block */

    // bitset<enum<register_flag>>
    char	rflags;		/* INTEGER/FLOAT, WRITABLE */
    char	rformat;	/* print format: 'x', 'X', 'f', '8', '3', 'Y', 'W' */
};
/*e: struct [[Reglist]] */

/*s: enum [[register_flag]] */
enum {					/* bits in rflags field */
    RINT	= (0<<0),
    RFLT	= (1<<0),

    RRDONLY	= (1<<1),
};
/*e: enum [[register_flag]] */

/*s: struct [[Mach]] */
/*
 *	Machine-dependent data is stored in two structures:
 *		Mach  - miscellaneous general parameters
 *		Machdata - jump vector of service functions used by debuggers
 *
 *	Mach is defined in ?.c and set in executable.c
 *
 *	Machdata is defined in ?db.c
 *		and set in the debugger startup.
 */
struct Mach{
    char	*name;
    // enum<machine_type>
    int	mtype;			/* machine type code */

    Reglist *reglist;		/* register set */
    long	regsize;		/* sizeof registers in bytes */
    long	fpregsize;		/* sizeof fp registers in bytes */

    char	*pc;			/* pc name */
    char	*sp;			/* sp name */
    char	*link;			/* link register name */
    char	*sbreg;			/* static base register name */
    uvlong	sb;			/* static base register value */

    int	pgsize;			/* page size */
    uvlong	kbase;			/* kernel base address */
    uvlong	ktmask;			/* ktzero = kbase & ~ktmask */
    uvlong	utop;			/* user stack top */

    int	pcquant;		/* quantization of pc */

    int	szaddr;			/* sizeof(void*) */
    int	szreg;			/* sizeof(register) */
    int	szfloat;		/* sizeof(float) */
    int	szdouble;		/* sizeof(double) */
};
/*e: struct [[Mach]] */

extern	Mach	*mach;			/* Current machine */

typedef uvlong	(*Rgetter)(Map*, char*);
typedef	void	(*Tracer)(Map*, uvlong, uvlong, Symbol*);

/*s: struct [[Machdata]] */
struct	Machdata {		/* Machine-dependent debugger support */
    uchar	bpinst[4];			/* break point instr. */
    short	bpsize;				/* size of break point instr. */

    ushort	(*swab)(ushort);		/* ushort to local byte order */
    ulong	(*swal)(ulong);			/* ulong to local byte order */
    uvlong	(*swav)(uvlong);		/* uvlong to local byte order */

    int	(*ctrace)(Map*, uvlong, uvlong, uvlong, Tracer); /* C traceback */
    uvlong	(*findframe)(Map*, uvlong, uvlong, uvlong, uvlong);/* frame finder */
    char*	(*excep)(Map*, Rgetter);	/* last exception */
    ulong	(*bpfix)(uvlong);		/* breakpoint fixup */

    int	(*sftos)(char*, int, void*);	/* single precision float */
    int	(*dftos)(char*, int, void*);	/* double precision float */

    int	(*foll)(Map*, uvlong, Rgetter, uvlong*);/* follow set */

    int	(*das)(Map*, uvlong, char, char*, int);	/* symbolic disassembly */
    int	(*hexinst)(Map*, uvlong, char*, int); 	/* hex disassembly */
    int	(*instsize)(Map*, uvlong);	/* instruction size */
};
/*e: struct [[Machdata]] */

/*s: struct [[Fhdr]] */
/*
 *	Common a.out header describing all architectures
 */
struct Fhdr
{
    char	*name;		/* identifier of executable */

    // enum<executable_type>
    byte	type;		/* file type - see codes above */

    byte	hdrsz;		/* header size */
    byte	_magic;		/* _MAGIC() magic */
    byte	spare;

    long	magic;		/* magic number */

    uvlong	txtaddr;	/* text address */
    vlong	txtoff;		/* start of text in file */
    uvlong	dataddr;	/* start of data segment */
    vlong	datoff;		/* offset to data seg in file */
    vlong	symoff;		/* offset of symbol table in file */

    uvlong	entry;		/* entry point */

    vlong	sppcoff;	/* offset of sp-pc table in file */
    vlong	lnpcoff;	/* offset of line number-pc table in file */

    long	txtsz;		/* text size */
    long	datsz;		/* size of data seg */
    long	bsssz;		/* size of bss */

    long	symsz;		/* size of symbol table */
    long	sppcsz;		/* size of sp-pc table */ // unused
    long	lnpcsz;		/* size of line number-pc table */

};
/*e: struct [[Fhdr]] */

extern	int	asstype;	/* dissembler type - machdata.c */
extern	Machdata *machdata;	/* jump vector - machdata.c */

Map*		attachproc(int, int, int, Fhdr*);
int		beieee80ftos(char*, int, void*);
int		beieeesftos(char*, int, void*);
int		beieeedftos(char*, int, void*);
ushort		beswab(ushort);
ulong		beswal(ulong);
uvlong		beswav(uvlong);
uvlong		ciscframe(Map*, uvlong, uvlong, uvlong, uvlong);
int		cisctrace(Map*, uvlong, uvlong, uvlong, Tracer);
int		crackhdr(int fd, Fhdr*);
uvlong		file2pc(char*, long);
int		fileelem(Sym**, uchar *, char*, int);
long		fileline(char*, int, uvlong);
int		filesym(int, char*, int);
int		findlocal(Symbol*, char*, Symbol*);
int		findseg(Map*, char*);
int		findsym(uvlong, int, Symbol *);
int		fnbound(uvlong, uvlong*);
int		fpformat(Map*, Reglist*, char*, int, int);
int		get1(Map*, uvlong, uchar*, int);
int		get2(Map*, uvlong, ushort*);
int		get4(Map*, uvlong, ulong*);
int		get8(Map*, uvlong, uvlong*);
int		geta(Map*, uvlong, uvlong*);
int		getauto(Symbol*, int, int, Symbol*);
Sym*		getsym(int);
int		globalsym(Symbol *, int);
char*		_hexify(char*, ulong, int);
int		ieeesftos(char*, int, ulong);
int		ieeedftos(char*, int, ulong, ulong);
int		isar(Biobuf*);
int		leieee80ftos(char*, int, void*);
int		leieeesftos(char*, int, void*);
int		leieeedftos(char*, int, void*);
ushort		leswab(ushort);
ulong		leswal(ulong);
uvlong		leswav(uvlong);
uvlong		line2addr(long, uvlong, uvlong);
Map*		loadmap(Map*, int, Fhdr*);
int		localaddr(Map*, char*, char*, uvlong*, Rgetter);
int		localsym(Symbol*, int);
int		lookup(char*, char*, Symbol*);
void		machbytype(int);
int		machbyname(char*);
int		nextar(Biobuf*, int, char*);
Map*		newmap(Map*, int);
void		objtraverse(void(*)(Sym*, void*), void*);
int		objtype(Biobuf*, char**);
uvlong		pc2sp(uvlong);
long		pc2line(uvlong);
int		put1(Map*, uvlong, uchar*, int);
int		put2(Map*, uvlong, ushort);
int		put4(Map*, uvlong, ulong);
int		put8(Map*, uvlong, uvlong);
int		puta(Map*, uvlong, uvlong);
int		readar(Biobuf*, int, vlong, int);
int		readobj(Biobuf*, int);
uvlong		riscframe(Map*, uvlong, uvlong, uvlong, uvlong);
int		risctrace(Map*, uvlong, uvlong, uvlong, Tracer);
int		setmap(Map*, int, uvlong, uvlong, vlong, char*);
Sym*		symbase(long*);
int		syminit(int, Fhdr*);
int		symoff(char*, int, uvlong, int);
void		textseg(uvlong, Fhdr*);
int		textsym(Symbol*, int);
void		unusemap(Map*, int);
/*e: include/debug/mach.h */
