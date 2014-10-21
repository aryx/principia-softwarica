/*s: machine/5i/arm.h */
/*
 * arm.h
 */

typedef	struct	Registers	Registers;
typedef	struct	Segment		Segment;
typedef	struct	Memory		Memory;
typedef	struct	Mul		Mul;
typedef	struct	Mulu		Mulu;
typedef	struct	Inst		Inst;
typedef	struct	Icache		Icache;
typedef	struct	Tlb		Tlb;
typedef	struct	Breakpoint	Breakpoint;

/*s: enum _anon_ */
enum
{
    Instruction		= 1,
    Read		= 2,
    Write		= 4,
    Access		= Read|Write,
    Equal		= 4|8,
};
/*e: enum _anon_ */

/*s: struct Breakpoint */
struct Breakpoint
{
    //enum<breakpoint_kind>
    int		type;		/* Instruction/Read/Access/Write/Equal */

    ulong	addr;		/* Place at address */
    int		count;		/* To execute count times or value */
    int		done;		/* How many times passed through */
    Breakpoint*	next;		/* Link to next one */
};
/*e: struct Breakpoint */

/*s: enum ixxx */
enum ixxx
{
    Imem,
    Iarith,
    Ibranch,
    Isyscall,
};
/*e: enum ixxx */

/*s: constant Nmaxtlb */
#define Nmaxtlb 64
/*e: constant Nmaxtlb */
/*s: enum regxxx */
enum
{
    REGARG	= 0,
    REGRET	= 0,
    REGPC	= 15,
    REGLINK	= 14,
    REGSP	= 13,
};
/*e: enum regxxx */

/*s: struct Tlb */
struct Tlb
{
    bool	on;			/* Being updated */
    int	tlbsize;		/* Number of entries */
    ulong	tlbent[Nmaxtlb];	/* Virtual address tags */
    int	hit;			/* Number of successful tag matches */
    int	miss;			/* Number of failed tag matches */
};		
/*e: struct Tlb */

/*s: struct Icache */
struct Icache
{
    bool	on;			/* Turned on */

    int	linesize;		/* Line size in bytes */
    int	stall;			/* Cache stalls */
    int*	lines;			/* Tag array */
    int*	(*hash)(ulong);		/* Hash function */
    char*	hashtext;		/* What the function looks like */
};
/*e: struct Icache */

/*s: struct Inst */
struct Inst
{
    void 	(*func)(ulong);
    char*	name;
    // enum<ixxx>
    int	type;

    // profiling info
    int	count;
    int	taken;
    int	useddelay;
};
/*e: struct Inst */

/*s: struct Registers */
struct Registers
{
    long	r[16];

    ulong	ar;    // reg.r[REGPC]
    ulong	ir;    // ifetch(reg.ar)
    int		class; // armclass(reg.ir)
    Inst*	ip;    // &itab[reg.class]

    long	cc1;
    long	cc2;

    int	cond;
    int	compare_op;
    int	cbit;
    int	cout;
};
/*e: struct Registers */

/*s: enum _anon_ (machine/5i/arm.h)4 */
enum
{
    MemRead,
    MemReadstring,
    MemWrite,
};
/*e: enum _anon_ (machine/5i/arm.h)4 */

/*s: enum _anon_ (machine/5i/arm.h)5 */
enum
{
    CCcmp, 
    CCtst,
    CCteq,
};
/*e: enum _anon_ (machine/5i/arm.h)5 */

/*s: enum segment_kind */
enum segment_kind
{
    Stack,
    Text,
    Data,
    Bss,

    Nseg,
};
/*e: enum segment_kind */

/*s: struct Segment */
struct Segment
{
    // enum<segment_kind>
    short	type;

    ulong	base;
    ulong	end;

    ulong	fileoff;
    ulong	fileend;

    // ??
    int	rss;
    int	refs;

    byte**	table;
};
/*e: struct Segment */

/*s: struct Memory */
struct Memory
{
    //map<enum<segment_kind>, Segment>
    Segment	seg[Nseg];
};
/*e: struct Memory */

//@Scheck: in libmach.a
int		armclass(long);

void		Ssyscall(ulong);
void		breakpoint(char*, char*);
void		brkchk(ulong, int);
void		cmd(void);
void		delbpt(char*);
void		dobplist(void);
void		dumpdreg(void);
void		dumpfreg(void);
void		dumpreg(void);
void*		emalloc(ulong);
void*		erealloc(void*, ulong, ulong);
ulong		expr(char*);
void		fatal(int, char*, ...);
ulong		getmem_2(ulong);
ulong		getmem_4(ulong);
uchar		getmem_b(ulong);
ushort		getmem_h(ulong);
uvlong		getmem_v(ulong);
ulong		getmem_w(ulong);
ulong		ifetch(ulong);
void		initstk(int, char**);
void		iprofile(void);
void		isum(void);
void		itrace(char*, ...);
long		lnrand(long);
char*		memio(char*, ulong, int, int);
char*		nextc(char*);
void		printparams(Symbol*, ulong);
void		printsource(long);
void		putmem_b(ulong, uchar);
void		putmem_h(ulong, ushort);
void		putmem_v(ulong, uvlong);
void		putmem_w(ulong, ulong);
void		run(void);
void		segsum(void);
void		stktrace(int);
void		tlbsum(void);
void		updateicache(ulong addr);

/* Globals */
extern	Registers	reg;
extern	Memory		memory;
extern	int		text;
extern	int		trace;
extern	int		sysdbg;
extern	int		calltree;
extern	Inst		itab[];
extern	Icache		icache;
extern	Tlb		tlb;
extern	int		count;
extern	jmp_buf		errjmp;
extern	Breakpoint*	bplist;
extern	int		atbpt;
extern	int		membpt;
extern	int		cmdcount;
extern	int		nopcount;
extern	ulong		dot;

extern	Biobuf*		bioout;
extern	Biobuf*		bin;
extern	ulong*		iprof;
extern	int		datasize;
extern	Map*		symmap;	
extern ulong	textbase;

/*s: enum _anon_ (machine/5i/arm.h)7 */
/* Plan9 Kernel constants */
enum
{
    BY2PG		= 4096,
    BY2WD		= 4,
    UTZERO		= 0x1000,
    STACKTOP	= 0x80000000,
    STACKSIZE	= 0x10000,

    PROFGRAN	= 4,
    Sbit		= 1<<20,
    SIGNBIT		= 0x80000000,

    FP_U		= 3,
    FP_L		= 1,
    FP_G		= 2,
    FP_E		= 0,
    FP_CBIT		= 1<<23,
};
/*e: enum _anon_ (machine/5i/arm.h)7 */
/*e: machine/5i/arm.h */
