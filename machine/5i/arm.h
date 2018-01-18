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

/*s: enum [[breakpoint_kind]] */
enum breakpoint_kind
{
    Instruction		= 1,

    Read		= 2,
    Write		= 4,
    Access		= Read|Write,

    Equal		= 4|8,
};
/*e: enum [[breakpoint_kind]] */

/*s: struct [[Breakpoint]] */
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
/*e: struct [[Breakpoint]] */

/*s: enum [[ixxx]] */
enum opcode_category
{
    Iarith,
    Imem,
    Ibranch,
    Isyscall,
    Imisc,
};
/*e: enum [[ixxx]] */

// added by pad
/*s: enum [[opcode]] */
enum opcode {
    // -----------------------------------------
    // Arithmetic and logic opcodes
    // -----------------------------------------
    /*s: arith/logic opcodes */
    OAND = 0,
    OEOR = 1, 
    OSUB = 2, 
    ORSB = 3, 
    OADD = 4, 
    OADC = 5, 
    OSBC = 6, 
    ORSC = 7, 
    OTST = 8, 
    OTEQ = 9,
    OCMP = 10,
    OCMN = 11,
    OORR = 12,
    OMOV = 13,
    OBIC = 14,
    OMVN = 15,
    /*x: arith/logic opcodes */
    OMUL    = 64,
    OMULA   = 65,
    /*x: arith/logic opcodes */
    CMUL    = 64,
    /*x: arith/logic opcodes */
    CARITH0 = 0,  // r,r,r
    CARITH1 = 16, // r<>#, r, r
    CARITH2 = 32, // r<>r, r, r
    /*x: arith/logic opcodes */
    CARITH3 = 48, // i,r,r
    /*x: arith/logic opcodes */
    OMULLU  = 66,
    OMULALU = 67,
    OMULL   = 68,
    OMULAL  = 69,
    /*e: arith/logic opcodes */
    // -----------------------------------------
    // Memory MOV opcodes
    // -----------------------------------------
    /*s: memory opcodes */
    OLDW    = 70,
    OLDB    = 71,
    OSTW    = 72,
    OSTB    = 73,
    /*x: memory opcodes */
    CMEM_BASIS = 70,
    CMEM0 = 0, // i(r)
    CMEM1 = 4, // (r),(r)
    CMEM2 = 8, // byte signed or half word
    /*x: memory opcodes */
    OLDH    = 78,
    OLDBU   = 79,
    OSTH    = 80,
    OSTBU   = 81,
    /*x: memory opcodes */
    OSWPW = 82,
    OSWPBU = 83,
    /*x: memory opcodes */
    OLDM = 84,
    OSTM = 85,
    /*x: memory opcodes */
    CBLOC   = 84,
    /*e: memory opcodes */
    // -----------------------------------------
    // Control flow opcodes
    // -----------------------------------------
    /*s: branching opcodes */
    OB = 86,
    OBL = 87,
    /*x: branching opcodes */
    CBRANCH = 86,
    /*e: branching opcodes */
    // -----------------------------------------
    // Syscall opcodes
    // -----------------------------------------
    /*s: syscall opcodes */
    OSWI = 88,
    /*e: syscall opcodes */

    // for opcodes not handled by 5i
    OUNDEF   = 89
};
/*e: enum [[opcode]] */

/*s: constant [[Nmaxtlb]] */
#define Nmaxtlb 64
/*e: constant [[Nmaxtlb]] */

/*s: enum [[regxxx]] */
enum
{
    REGARG	= 0,
    REGRET	= 0,

    REGSP	= 13,
    REGLINK	= 14,
    REGPC	= 15,
};
/*e: enum [[regxxx]] */

/*s: struct [[Tlb]] */
struct Tlb
{
    bool	on;			/* Being updated */
    int		tlbsize;		/* Number of entries */
    // all pointers in array are at page granularity
    uintptr	tlbent[Nmaxtlb];	/* Virtual address tags */

    int	hit;			/* Number of successful tag matches */
    int	miss;			/* Number of failed tag matches */
};		
/*e: struct [[Tlb]] */

/*s: struct [[Icache]] */
struct Icache
{
    bool	on;			/* Turned on */

    int		linesize;		/* Line size in bytes */
    int		stall;			/* Cache stalls */
    int*	lines;			/* Tag array */
    int*	(*hash)(ulong);	/* Hash function */
    char*	hashtext;		/* What the function looks like */
};
/*e: struct [[Icache]] */


/*s: struct [[Inst]] */
struct Inst
{
    void 	(*func)(instruction);
    char*	name;
    // enum<opcode_category>
    int	type;

    /*s: [[Inst]] profiling fields */
    // profiling info
    int	count;
    int	taken;
    int	useddelay;
    /*e: [[Inst]] profiling fields */
};
/*e: struct [[Inst]] */

/*s: struct [[Registers]] */
struct Registers
{
    long	r[16];
    /*s: [[Registers]] other fields */
    uintptr		ar;    // reg.r[REGPC]

    instruction	instr;    // ifetch(reg.ar)
    //enum<opcode>
    int			instr_opcode; // arm_class(reg.instr)
    Inst*		ip;    // &itab[reg.instr_opcode]
    /*x: [[Registers]] other fields */
    int	cbit; // carry bit?
    int	cout;
    /*x: [[Registers]] other fields */
    // actually only 4 bits (16 possibilities)
    int	instr_cond;
    /*x: [[Registers]] other fields */
    // enum<compare_op>
    int	compare_op;
    /*x: [[Registers]] other fields */
    long	cc1;
    long	cc2;
    /*e: [[Registers]] other fields */
};
/*e: struct [[Registers]] */

/*s: enum [[memxxx]] */
// for memio()
enum
{
    MemRead,
    MemReadstring,
    MemWrite,
};
/*e: enum [[memxxx]] */

/*s: enum [[compare_op]] */
enum compare_op
{
    CCcmp, 
    CCtst,
    CCteq,
};
/*e: enum [[compare_op]] */

/*s: enum [[segment_kind]] */
enum segment_kind
{
    Text,
    Data,
    Bss,
    Stack,

    Nseg,
};
/*e: enum [[segment_kind]] */

/*s: struct [[Segment]] */
struct Segment
{
    // enum<segment_kind>
    short	type;

    uintptr	base;
    uintptr	end;

    //array<option<array_4096<byte>>> page table
    byte**	table; // the data

    // for the Text and Data segments the bytes are in the file
    ulong	fileoff;
    ulong	fileend;

    /*s: [[Segment]] profiling fields */
    int	rss;
    int	refs;
    /*e: [[Segment]] profiling fields */
};
/*e: struct [[Segment]] */

/*s: struct [[Memory]] */
struct Memory
{
    //map<enum<segment_kind>, Segment>
    Segment	seg[Nseg];
};
/*e: struct [[Memory]] */

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

extern	Biobuf*		bout;
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

/*s: enum [[_anon_ (machine/5i/arm.h)]]7 */
enum
{
    /* Plan9 Kernel constants */
    BY2PG		= 4096,
    BY2WD		= 4,

    UTZERO		= 0x1000,
    STACKTOP	= 0x80000000,
    STACKSIZE	= 0x10000,

    /*s: constant [[PROFGRAN]] */
    PROFGRAN	= 4,
    /*e: constant [[PROFGRAN]] */
    /*s: constant [[Sbit]] */
    Sbit		= 1<<20,
    /*e: constant [[Sbit]] */
    /*s: constant [[SIGNBIT]] */
    SIGNBIT		= 0x80000000,
    /*e: constant [[SIGNBIT]] */
};
/*e: enum [[_anon_ (machine/5i/arm.h)]]7 */
/*e: machine/5i/arm.h */
