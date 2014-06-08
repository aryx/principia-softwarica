/*s: portfns_memory.h */

// xalloc.c
void*   xalloc(ulong);
void    xfree(void*);
void    xsummary(void);
int   xmerge(void*, void*);
void    xinit(void);
void*   xspanalloc(ulong, int, ulong);

// pool.c
// see include/pool.h: poollock(), ...
void    mallocsummary(void);

// alloc.c
void*   malloc(ulong);
void    free(void*);
void*   smalloc(ulong);
void*   mallocz(ulong, int);
void*   mallocalign(ulong, ulong, long, ulong);
void*   realloc(void *v, ulong size);
ulong   msize(void*);
void    setmalloctag(void*, ulong);
void    kstrcpy(char*, char*, int);
void    kstrdup(char**, char*);
//void    setrealloctag(void*, ulong);
//ulong   getmalloctag(void*);
//ulong   getrealloctag(void*);

// page.c
void    pageinit(void);
void    pagechainhead(Page*);
void    copypage(Page*, Page*); //TODO: why fp no-deps in codegraph?
void cachepage(Page *p, KImage *i);
void    uncachepage(Page*);
void    cachedel(KImage*, ulong);
Pagetable*    ptalloc(void);
Pagetable*    ptcpy(Pagetable*);
void    freept(Segment*, Pagetable*);
ulong pagenumber(Page*);
Page*   lookpage(KImage*, ulong);
void    putpage(Page*);
Page*   auxpage(void);
Page*   newpage(int, Segment **, ulong);
int   duppage(Page*);
void checkpagerefs(void);
//void    portcountpagerefs(ulong*, int);
//int   hasfreepages(void*);

// swap.c
void    swapinit(void);
void    putswap(Page*);
void    dupswap(Page*);
int   swapcount(ulong);
//int   swapfull(void);
void    pagersummary(void);
void    setswapchan(Chan*);
void    kickpager(void);

// fault.c
Segment*  seg(Proc*, ulong, int);
void checkpages(void);
void    validaddr(ulong, ulong, bool);
int   okaddr(ulong, ulong, int);
int   fixfault(Segment*, ulong, int, int);
void*   vmemchr(virt_addr3, int, int);
int   fault(ulong, int);

// segment.c
Segment*  newseg(int, ulong, ulong);
void    relocateseg(Segment*, ulong);
void    segpage(Segment*, Page*);
void    putimage(KImage*);
void    mfreeseg(Segment*, ulong, int);
void    segclock(ulong);
void    putseg(Segment*);
Segment*  dupseg(Segment**, int, int);
long    ibrk(ulong, int);
int   addphysseg(Physseg*);
//int   isphysseg(char*);
ulong   segattach(Proc*, ulong, char *, ulong, ulong);
void    initimage(void);
KImage*   attachimage(int, Chan*, ulong, ulong);
//Segment*  isoverlap(Proc*, ulong, int);
Segment*  data2txt(Segment*);


// sysfile.c
// syssetflush (used in syscalls/ without requiring extern decl)

// in 386/mmu.c (but used in port)
KMap* kmap(Page*);
void  kunmap(KMap*);
void    countpagerefs(ulong*, int);
void    flushmmu(void);
void checkmmu(ulong va, ulong pa);
ulong cankaddr(ulong);
// actually KADDR is used in port, but it's expanding to kaddr
kern_addr3 kaddr(phys_addr);
// actually PADDR is used in port, but it's expanding to paddr
phys_addr paddr(kern_addr3);
void    putmmu(ulong, ulong, Page*);
void    mmurelease(Proc*);
void    mmuswitch(Proc*);
/*e: portfns_memory.h */
