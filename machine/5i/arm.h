/*s: machine/5i/arm.h */
/*
 * arm.h
 */

// forward decls
typedef	struct	Registers	Registers;
typedef	struct	Segment		Segment;
typedef	struct	Memory		Memory;
typedef	struct	Inst		Inst;
typedef	struct	Icache		Icache;
typedef	struct	Tlb			Tlb;
typedef	struct	Breakpoint	Breakpoint;

/*s: typedef instruction */
typedef ulong instruction;
/*e: typedef instruction */

/*s: enum breakpoint_kind */
enum breakpoint_kind
{
    Instruction		= 1,

    Read		= 2,
    Write		= 4,
    Access		= Read|Write,

    Equal		= 4|8,
};
/*e: enum breakpoint_kind */

/*s: struct Breakpoint */
struct Breakpoint
{
    //enum<breakpoint_kind>
    int		type;		/* Instruction/Read/Access/Write/Equal */

    uintptr	addr;		/* Place at address */
    int		count;		/* To execute count times or value */
    int		done;		/* How many times passed through */

    // Extra
    /*s: [[Breakpoint]] extra fields */
    Breakpoint*	next;		/* Link to next one */
    /*e: [[Breakpoint]] extra fields */
};
/*e: struct Breakpoint */

/*s: enum ixxx */
enum ixxx
{
    Iarith,
    Imem,
    Ibranch,
    Isyscall,
};
/*e: enum ixxx */

// added by pad
/*s: enum class_kind */
enum class_kind {
    // AND, EOR, ORR, BIC
    // ADD, SUB, RSB, // ADC, SBC, RSC (unused by plan9 5a/5c/5l)
    // TST, TEQ, CMP, CMN
    // MOV, MVN
    CARITH0 = 0,  // r,r,r
    CARITH1 = 16, // r<>r, r, r
    CARITH2 = 32, // r<>#, r, r
    CARITH3 = 48, // i,r,r
    // MUL, MULA, MULLU, MULALU, MULL, MULAL
    CMUL    = 64,

    // SWPW, SWPBU
    CSWAP   = 70,
    // MOV, MOVW, MOVB
    CMEM    = 72,
    // LDM, STM
    CBLOC   = 84,

    // B, BL
    CBRANCH = 86,
    // SWI
    CSYSCALL = 88,

    CUNDEF   = 89
};
/*e: enum class_kind */

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
    int		tlbsize;		/* Number of entries */
    // all pointers in array are at page granularity
    uintptr	tlbent[Nmaxtlb];	/* Virtual address tags */

    int	hit;			/* Number of successful tag matches */
    int	miss;			/* Number of failed tag matches */
};		
/*e: struct Tlb */

/*s: struct Icache */
struct Icache
{
    bool	on;			/* Turned on */

    int		linesize;		/* Line size in bytes */
    int		stall;			/* Cache stalls */
    int*	lines;			/* Tag array */
    int*	(*hash)(ulong);	/* Hash function */
    char*	hashtext;		/* What the function looks like */
};
/*e: struct Icache */


/*s: struct Inst */
struct Inst
{
    void 	(*func)(instruction);
    char*	name;
    // enum<ixxx>
    int	type;

    /*s: [[Inst]] profiling fields */
    // profiling info
    int	count;
    int	taken;
    int	useddelay;
    /*e: [[Inst]] profiling fields */
};
/*e: struct Inst */

/*s: struct Registers */
struct Registers
{
    long	r[16];
    /*s: [[Registers]] other fields */
    uintptr		ar;    // reg.r[REGPC]
    instruction	ir;    // ifetch(reg.ar)
    //enum<class_kind>
    int			class; // arm_class(reg.ir)
    Inst*		ip;    // &itab[reg.class]
    /*x: [[Registers]] other fields */
    // actually only 4 bits (16 possibilities)
    int	cond;
    /*x: [[Registers]] other fields */
    // enum<compare_op>
    int	compare_op;
    /*x: [[Registers]] other fields */
    long	cc1;
    long	cc2;
    /*x: [[Registers]] other fields */
    int	cbit; // carry bit?
    int	cout;
    /*e: [[Registers]] other fields */
};
/*e: struct Registers */

/*s: enum memxxx */
// for memio()
enum
{
    MemRead,
    MemReadstring,
    MemWrite,
};
/*e: enum memxxx */

/*s: enum compare_op */
enum compare_op
{
    CCcmp, 
    CCtst,
    CCteq,
};
/*e: enum compare_op */

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

    uintptr	base;
    uintptr	end;

    ulong	fileoff;
    ulong	fileend;

    //array<option<array_4096<bytes>>> page table
    byte**	table; // the data

    /*s: [[Segment]] profiling fields */
    int	rss;
    int	refs;
    /*e: [[Segment]] profiling fields */
};
/*e: struct Segment */

/*s: struct Memory */
struct Memory
{
    //map<enum<segment_kind>, Segment>
    Segment	seg[Nseg];
};
/*e: struct Memory */

// for cmd.c
void		run(void);
// for cmd.c reset
void		initstk(int, char**);
// for syscalls.c
char*		memio(char*, ulong, int, int);
// for run.c
void		Ssyscall(ulong);
// for run.c
ulong		ifetch(ulong);
// used to be in libmach/, but I copy pasted it in run.c
//int	arm_class(instruction);

void		updateicache(ulong addr);

ulong		getmem_2(ulong);
ulong		getmem_4(ulong);
uchar		getmem_b(ulong);
ushort		getmem_h(ulong);
uvlong		getmem_v(ulong);
ulong		getmem_w(ulong);
void		putmem_b(ulong, uchar);
void		putmem_h(ulong, ushort);
void		putmem_v(ulong, uvlong);
void		putmem_w(ulong, ulong);

void		cmd(void);
ulong		expr(char*);
char*		nextc(char*);


void		breakpoint(char*, char*);
void		delbpt(char*);
void		brkchk(ulong, int);
void		dobplist(void);

void		dumpdreg(void);
void		dumpfreg(void);
void		dumpreg(void);
void		stktrace(int);
void		printparams(Symbol*, ulong);
void		printsource(long);

// profiling
void		iprofile(void);
void		isum(void);
void		segsum(void);
void		tlbsum(void);

void*		emalloc(ulong);
void*		erealloc(void*, ulong, ulong);

void		fatal(int, char*, ...);
void		itrace(char*, ...);

// from libc.h
//long		lnrand(long);

/* Globals */
extern	Registers	reg;
extern	Memory		memory;
extern	Icache		icache;
extern	Tlb			tlb;

extern	Inst		itab[];

extern	instruction	dot;
extern	int		count;

extern	Biobuf*		bioout;
extern	Biobuf*		bin;
extern	int		text;
extern	ulong	textbase;
extern	int		datasize;

extern	Map*	symmap;	

extern	bool	trace;
extern	bool	sysdbg;
extern	bool	calltree;
extern	Breakpoint*	bplist;
extern	int		atbpt;
extern	int		membpt;

extern	jmp_buf	errjmp;

extern	int		cmdcount;
extern	int		nopcount;
extern	ulong*		iprof;

/*s: enum _anon_ (machine/5i/arm.h)7 */
/* Plan9 Kernel constants */
enum
{
    BY2PG		= 4096,
    BY2WD		= 4,

    UTZERO		= 0x1000,
    STACKTOP	= 0x80000000,
    STACKSIZE	= 0x10000,

    /*s: constant PROFGRAN */
    PROFGRAN	= 4,
    /*e: constant PROFGRAN */
    /*s: constant Sbit */
    Sbit		= 1<<20,
    /*e: constant Sbit */
    /*s: constant SIGNBIT */
    SIGNBIT		= 0x80000000,
    /*e: constant SIGNBIT */
};
/*e: enum _anon_ (machine/5i/arm.h)7 */
/*e: machine/5i/arm.h */
