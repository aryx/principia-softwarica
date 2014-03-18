
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

