
// xalloc.c
void*		xalloc(ulong);
void		xfree(void*);
void		xsummary(void);
int		xmerge(void*, void*);
void		xinit(void);
void*		xspanalloc(ulong, int, ulong);

// pool.c
// see include/pool.h: poollock(), ...
void		mallocsummary(void);

// alloc.c
void*		malloc(ulong);
void		free(void*);
void*		smalloc(ulong);
void*		mallocz(ulong, int);
void*		mallocalign(ulong, ulong, long, ulong);
void*		realloc(void *v, ulong size);
ulong		msize(void*);
void		setmalloctag(void*, ulong);
void		kstrcpy(char*, char*, int);
void		kstrdup(char**, char*);
void		setrealloctag(void*, ulong);//internal to alloc.c
//ulong		getmalloctag(void*);
//ulong		getrealloctag(void*);

// page.c
void		pageinit(void);
void		pagechainhead(Page*);
void		copypage(Page*, Page*); //TODO: why fp no-deps in codegraph?
void cachepage(Page *p, KImage *i);
void		uncachepage(Page*);
void		cachedel(KImage*, ulong);
Pte*		ptealloc(void);
Pte*		ptecpy(Pte*);
void		freepte(Segment*, Pte*);
ulong	pagenumber(Page*);
Page*		lookpage(KImage*, ulong);
void		putpage(Page*);
Page*		auxpage(void);
Page*		newpage(int, Segment **, ulong);
int		duppage(Page*);
void checkpagerefs(void);
void		portcountpagerefs(ulong*, int);
//int		ispages(void*);

// swap.c
void		swapinit(void);
void		putswap(Page*);
void		dupswap(Page*);
int		swapcount(ulong);
int		swapfull(void);
void		pagersummary(void);
void		setswapchan(Chan*);
void		kickpager(void);

// fault.c
Segment*	seg(Proc*, ulong, int);
void checkpages(void);
void		validaddr(ulong, ulong, int);
int		okaddr(ulong, ulong, int);
int		fixfault(Segment*, ulong, int, int);
void*		vmemchr(void*, int, int);
int		fault(ulong, int);

// segment.c
Segment*	newseg(int, ulong, ulong);
void		relocateseg(Segment*, ulong);
void		segpage(Segment*, Page*);
void		putimage(KImage*);
void		mfreeseg(Segment*, ulong, int);
void		segclock(ulong);
void		putseg(Segment*);
Segment*	dupseg(Segment**, int, int);
long		ibrk(ulong, int);
int		addphysseg(Physseg*);
//int		isphysseg(char*);
ulong		segattach(Proc*, ulong, char *, ulong, ulong);
void		initseg(void);
KImage*		attachimage(int, Chan*, ulong, ulong);
//Segment*	isoverlap(Proc*, ulong, int);


// sysfile.c
// syssetflush

// in 386/mmu.c (but used in port)
KMap*	kmap(Page*);
void	kunmap(KMap*);
void		countpagerefs(ulong*, int);
void		flushmmu(void);
void checkmmu(ulong va, ulong pa);
ulong	cankaddr(ulong);
// actually KADDR is used in port, but it's expanding to kaddr
void*	kaddr(ulong);
// actually PADDR is used in port, but it's expanding to paddr
ulong	paddr(void*);
void		putmmu(ulong, ulong, Page*);
void		mmurelease(Proc*);
void		mmuswitch(Proc*);
