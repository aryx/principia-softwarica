struct Rendez
{
	Lock;
	Proc	*p;
};

struct Sema
{
	Rendez;
	long	*addr;
	int	waiting;
	Sema	*next;
	Sema	*prev;
};



enum
{
	PG_NOFLUSH	= 0,
	PG_TXTFLUSH	= 1,		/* flush dcache and invalidate icache */
	PG_DATFLUSH	= 2,		/* flush both i & d caches (UNUSED) */
	PG_NEWCOL	= 3,		/* page has been recolored */

	PG_MOD		= 0x01,		/* software modified bit */
	PG_REF		= 0x02,		/* software referenced bit */
};

struct Page
{
	Lock;
	ulong	pa;			/* Physical address in memory */
	ulong	va;			/* Virtual address for user */
	ulong	daddr;			/* Disc address on swap */
	ulong	gen;			/* Generation counter for swap */
	ushort	ref;			/* Reference count */
	char	modref;			/* Simulated modify/reference bits */
	char	color;			/* Cache coloring */
	char	cachectl[MAXMACH];	/* Cache flushing control for putmmu */
	KImage	*image;			/* Associated text or swap image */
	Page	*next;			/* Lru free list */
	Page	*prev;
	Page	*hash;			/* Image hash chains */
};


struct KImage
{
	Ref;
	Chan	*c;			/* channel to text file */
	Qid 	qid;			/* Qid for page cache coherence */
	Qid	mqid;
	Chan	*mchan;
	ushort	type;			/* Device type of owning channel */
	Segment *s;			/* TEXT segment for image if running */
	KImage	*hash;			/* Qid hash chains */
	KImage	*next;			/* Free list */
	int	notext;			/* no file associated */
};


struct Pte
{
	Page	*pages[PTEPERTAB];	/* Page map for this chunk of pte */
	Page	**first;		/* First used entry */
	Page	**last;			/* Last used entry */
};



/* Segment types */
enum
{
	SG_TYPE		= 07,		/* Mask type of segment */
	SG_TEXT		= 00,
	SG_DATA		= 01,
	SG_BSS		= 02,
	SG_STACK	= 03,
	SG_SHARED	= 04,
	SG_PHYSICAL	= 05,

	SG_RONLY	= 0040,		/* Segment is read only */
	SG_CEXEC	= 0100,		/* Detach at exec */
};

#define PG_ONSWAP	1
#define onswap(s)	(((ulong)s)&PG_ONSWAP)
#define pagedout(s)	(((ulong)s)==0 || onswap(s))
#define swapaddr(s)	(((ulong)s)&~PG_ONSWAP)

#define SEGMAXSIZE	(SEGMAPSIZE*PTEMAPMEM)

struct Physseg
{
	ulong	attr;			/* Segment attributes */
	char	*name;			/* Attach name */
	ulong	pa;			/* Physical address */
	ulong	size;			/* Maximum segment size in pages */
	Page	*(*pgalloc)(Segment*, ulong);	/* Allocation if we need it */
	void	(*pgfree)(Page*);
};


/*
 *  process memory segments - NSEG always last !
 */
enum
{
	SSEG, TSEG, DSEG, BSEG, ESEG, LSEG, SEG1, SEG2, SEG3, SEG4, NSEG
};

struct Segment
{
	Ref;
	QLock	lk;
	ushort	steal;		/* Page stealer lock */
	ushort	type;		/* segment type */
	ulong	base;		/* virtual base */
	ulong	top;		/* virtual top */
	ulong	size;		/* size in pages */
	ulong	fstart;		/* start address in file for demand load */
	ulong	flen;		/* length of segment in file */
	int	flushme;	/* maintain icache for this segment */
	KImage	*image;		/* text in file attached to this segment */
	Physseg *pseg;
	ulong*	profile;	/* Tick profile area */
	Pte	**map;
	int	mapsize;
	Pte	*ssegmap[SSEGMAPSIZE];
	Lock	semalock;
	Sema	sema;
	ulong	mark;		/* portcountrefs */
};



// internals


struct Swapalloc
{
	Lock;				/* Free map lock */
	int	free;			/* currently free swap pages */
	uchar*	swmap;			/* Base of swap map in memory */
	uchar*	alloc;			/* Round robin allocator */
	uchar*	last;			/* Speed swap allocation */
	uchar*	top;			/* Top of swap map */
	Rendez	r;			/* Pager kproc idle sleep */
	ulong	highwater;		/* Pager start threshold */
	ulong	headroom;		/* Space pager frees under highwater */
};

struct Pallocmem
{
	ulong base;
	ulong npage;
};

struct Palloc
{
	Lock;
	Pallocmem	mem[4];
	Page	*head;			/* most recently used */
	Page	*tail;			/* least recently used */
	ulong	freecount;		/* how many pages on free list now */
	Page	*pages;			/* array of all pages */
	ulong	user;			/* how many user pages */
	Page	*hash[PGHSIZE];
	Lock	hashlock;
	Rendez	r;			/* Sleep for free mem */
	QLock	pwait;			/* Queue of procs waiting for memory */
};

extern struct Swapalloc swapalloc;
extern	Palloc	palloc;
extern	KImage	swapimage;

// exported by include/pool.h
//extern Pool*	mainmem;
//extern Pool*	imagmem;

