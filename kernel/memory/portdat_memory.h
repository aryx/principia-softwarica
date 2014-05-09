/*s: portdat_memory.h */

// see also KMap in 386/ (but used in port)

/*s: pad memory pointer types */
// physical address
typedef ulong phys_addr;
// virtual address
typedef ulong virt_addr;
/*e: pad memory pointer types */

//*****************************************************************************
// Page < Pte (can be filled by KImage) < Segment
//*****************************************************************************

// All the ref<Page> here are references to Pages in the array<Page> of 
// Palloc.pages (pool allocator)
// All the ref<Kimage> here are references to KImage in the ?? of 
// Imagealloc.free?

/*s: enum modref */
enum modref 
{
    PG_MOD    = 0x01,   /* software modified bit */
    PG_REF    = 0x02,   /* software referenced bit */
};
/*e: enum modref */

/*s: enum cachectl */
enum cachectl
{
    PG_NOFLUSH  = 0,
    PG_TXTFLUSH = 1,    /* flush dcache and invalidate icache */
    //  PG_DATFLUSH = 2,    /* flush both i & d caches (UNUSED) */
    PG_NEWCOL = 3,    /* page has been recolored */
};
/*e: enum cachectl */

/*s: struct Page */
// Page metadata. We will allocate as many Page as to cover all physical memory
// + swap "address space". Either pa or daddr should be valid at one time.
// Should have been xalloc'ed in Palloc.pages
struct Page
{
    phys_addr pa;     /* Physical address in memory */
    virt_addr va;     /* Virtual address for user */
  
    ulong daddr;      /* Disc address on swap */
    ulong gen;      /* Generation counter for swap */
  
    // Why not Ref? to save space?
    ushort  ref;      /* Reference count */
    // set<enum<modref>>
    char  modref;     /* Simulated modify/reference bits */
    // enum<cachectl>??
    char  color;      /* Cache coloring */
    // array<enum<cachectl>>
    char  cachectl[MAXMACH];  /* Cache flushing control for putmmu */
  
    // extra
    Lock;
    Page  *next; /* Lru free list */ // list<ref<Page>> Palloc.head?
    Page  *prev; // ??
    Page  *hash; /* Image hash chains */ // hash<?, list<ref<Page>>> Palloc.hash?
    // option<ref<Kimage>>
    KImage  *image;     /* Associated text or swap image */
};
/*e: struct Page */

/*s: struct Pte */
// ptealloc'ed (malloc'ed)
struct Pte
{
    //array<option<ref<Page>> will map 1M of memory
    Page  *pages[PTEPERTAB];  /* Page map for this chunk of pte */
  
    //to avoid iterate over all pages
    // ref<ref<Page>> in Pte.pages
    Page  **first;    /* First used entry */
    // ref<ref<Page>> in Pte.pages
    Page  **last;     /* Last used entry */
};
/*e: struct Pte */


/*s: struct KImage */
// a KImage is essentially a channel to a text file (an image of a binary)
// the image in memory for a portion of a given file.
// (renamed KImage to avoid name conflict with memdraw Image (picture) and avoid
//  ugly #define Image IMAGE each time one wants to use draw.h from a device driver)
struct KImage
{
    Chan  *c;     /* channel to text file */
    Qid   qid;      /* Qid for page cache coherence */
    Qid mqid;
    Chan  *mchan;
    ushort  type;     /* Device type of owning channel */
  
    bool  notext;     /* no file associated */
  
    // extra
    Ref;
    // list<ref<Kimage>> of Imagealloc.free?
    KImage  *next; /* Free list */ 
    // hash<?, list<ref<Kimage>>> Imagealloc.hash?
    KImage  *hash; /* Qid hash chains */ 
    // option<ref<Segment>>?
    Segment *s;     /* TEXT segment for image if running */
};
/*e: struct KImage */


/*s: enum segtype */
/* Segment types */
enum segtype
{
    SG_TYPE   = 07,   /* Mask type of segment */
    SG_TEXT   = 00,
    SG_DATA   = 01,
    SG_BSS    = 02,
    SG_STACK  = 03,
    SG_SHARED = 04,
    SG_PHYSICAL = 05,
  
    SG_RONLY  = 0040,   /* Segment is read only */
    SG_CEXEC  = 0100,   /* Detach at exec */
};
/*e: enum segtype */

#define PG_ONSWAP 1
#define onswap(s) (((ulong)s)&PG_ONSWAP)
#define pagedout(s) (((ulong)s)==0 || onswap(s))
#define swapaddr(s) (((ulong)s)&~PG_ONSWAP)

#define SEGMAXSIZE  (SEGMAPSIZE*PTEMAPMEM)

/*s: struct Physseg */
struct Physseg
{
    ulong attr;     /* Segment attributes */
    char  *name;      /* Attach name */
    ulong pa;     /* Physical address */
    ulong size;     /* Maximum segment size in pages */
    Page  *(*pgalloc)(Segment*, ulong); /* Allocation if we need it */
    void  (*pgfree)(Page*);
};
/*e: struct Physseg */


// often used as (q->top-q->base)>>LRESPROF
enum
{
    LRESPROF  = 3,
};

/*s: struct Segment */
// smalloc'ed by newseg()
struct Segment
{
    // enum<segtype>
    ushort  type;   /* segment type */
  
    virt_addr base;   /* virtual base */
    virt_addr top;    /* virtual top */
    ulong size;   /* size in pages */ // top - base / BY2PG?
  
    // Kind of a page directory table (and pte = page table)
    // can be SEGMAPSIZE max so 1984 * 1M via PTE =~ 2Go virtual mem per segment!
    // array<option<ref<Pte>>>, smalloc'ed, point to ssegmap if small enough
    Pte **map; 
    // small seg map, used instead of map if segment small enough
    // array<ref<Pte>>
    Pte *ssegmap[SSEGMAPSIZE]; // 16
    int mapsize; // nelem(map)
  
    KImage  *image;   /* text in file attached to this segment */
    ulong fstart;   /* start address in file for demand load */
    ulong flen;   /* length of segment in file */
  
    bool flushme;  /* maintain icache for this segment */
    Physseg *pseg;
    ulong*  profile;  /* Tick profile area */
    ulong mark;   /* portcountrefs */
    ushort  steal;    /* Page stealer lock */
  
    // extra
    Ref;
    QLock lk;
    Sema  sema;
};
/*e: struct Segment */


//*****************************************************************************
// Internal to memory/
//*****************************************************************************

// See also RMap in 386/

// actually internal to xalloc.c, but important so here
/*s: constants holes */
enum
{
    Nhole   = 128,
    Magichole = 0x484F4C45,     /* HOLE */
};
/*e: constants holes */

/*s: struct Hole */
struct Hole
{
    ulong addr; // phys_addr? base?
    ulong top; // phys_addr?
    ulong size; // top - addr?
    
    // extra
    Hole* link; // list<ref<Hole>> of Xalloc.flist or Xalloc.table?
};
/*e: struct Hole */

/*s: struct Xhdr */
// What is the connection with Hole? a used Hole will describe
// a portion of memory, and at this memory there will be a header
// and then just after the actual memory xalloc'ed by someone
struct Xhdr
{
    // bookkeeping area
    ulong size;
    ulong magix;
  
    char  data[]; // memory pointer returned by xalloc
};
/*e: struct Xhdr */

/*s: struct Xalloc */
// Long lived data structure allocator (singleton)
// (can call xalloc() only Nhole time!)
struct Xalloc
{
    // array<Hole>
    Hole  hole[Nhole];
  
    // list<ref<Hole>> (next = Hole.link) free list?
    Hole* flist; 
    // list<ref<Hole>> (next = Hole.link) used list?
    Hole* table; 
  
    // extra
    Lock;
};
/*e: struct Xalloc */
//IMPORTANT: static Xalloc xlists; // private to xalloc.c


// from pool.h
//struct Pool {
//  char* name;
//  ulong maxsize;
//
//  ulong cursize;
//  ulong curfree;
//  ulong curalloc;
//
//  ulong minarena; /* smallest size of new arena */
//  ulong quantum;  /* allocated blocks should be multiple of */
//  ulong minblock; /* smallest newly allocated block */
//
//  void* freeroot; /* actually Free* */
//  void* arenalist;  /* actually Arena* */
//
//  void* (*alloc)(ulong);
//  int (*merge)(void*, void*);
//  void  (*move)(void* from, void* to);
//
//  int flags;
//  int nfree;
//  int lastcompact;
//
//  void  (*lock)(Pool*);
//  void  (*unlock)(Pool*);
//  void  (*print)(Pool*, char*, ...);
//  void  (*panic)(Pool*, char*, ...);
//  void  (*logstack)(Pool*);
//
//  void* private;
//};

// exported by libc include/pool.h, used by malloc, defined in pool.c in this dir
// memory pools for malloc()/free() (using xalloc pools)
//IMPORTANT: extern Pool*  mainmem;
// memory pools for ??
//IMPORTANT: extern Pool*  imagmem;

/*s: struct Pallocmem */
// memory banks, similar to RMap, and Confmem, but page oriented, and portable
struct Pallocmem
{
    phys_addr base;
    ulong npage;
};
/*e: struct Pallocmem */

/*s: function pghash */
enum
{
    PGHLOG  = 9, // 2^9 = 512
    PGHSIZE = 1<<PGHLOG,  /* Page hash for image lookup */
};
#define pghash(daddr) palloc.hash[(daddr>>PGSHIFT)&(PGHSIZE-1)]
/*e: function pghash */

/*s: struct Palloc */
// Page Allocator (singleton)
struct Palloc
{
    Pallocmem mem[4]; // TODO: 4 ?? same as Conf.mem
    // sum of mem.npage (which should be conf.upages)
    ulong user;     /* how many user pages */
  
    // array<Page>, xalloc'ed in pageinit() (huge)
    Page  *pages; /* array of all pages */ 
  
    // list<ref<Page>> (next = Page.next)
    Page  *head;      /* most recently used */
    // list<ref<Page>> (next = prev?)
    Page  *tail;      /* least recently used */
    ulong freecount;    /* how many pages on free list now */
  
    // hash<?pghash(Page.daddr?), list<ref<Page>> (next = Page.hash)>
    Page  *hash[PGHSIZE];
    Lock  hashlock;
  
    // extra
    Lock;
    Rendez  r;      /* Sleep for free mem */
    QLock pwait;      /* Queue of procs waiting for memory */
};
/*e: struct Palloc */
extern  Palloc  palloc;

#define NFREECHAN 64
/*s: function ihash */
#define IHASHSIZE 64
// actually internal to page.c, but important so here
#define ihash(s)  imagealloc.hash[s%IHASHSIZE]
/*e: function ihash */

/*s: struct Imagealloc */
// Image allocator (internal to segment.c, but important so here, singleton)
struct Imagealloc
{
    // array<Kimage>?  xalloc'ed in initseg() (conf.nimage)
    KImage  *free; 
    // hash<?ihash(??), list<ref<Kimage>>
    KImage  *hash[IHASHSIZE];
    QLock ireclaim; /* mutex on reclaiming free images */
  
    Chan  **freechan; /* free image channels */
    int nfreechan;  /* number of free channels */
    int szfreechan; /* size of freechan array */
    QLock fcreclaim;  /* mutex on reclaiming free channels */
  
    // extra
    Lock;

};
/*e: struct Imagealloc */

//IMPORTANT: static struct Imagealloc imagealloc; (segment.c)
// so have conf.nimage + 1 Kimages
extern  KImage  swapimage;

/*s: struct Swapalloc */
// Swap allocator (singleton)
struct Swapalloc
{
    int free;     /* currently free swap pages */
    uchar*  swmap;      /* Base of swap map in memory */
    uchar*  alloc;      /* Round robin allocator */
    uchar*  last;     /* Speed swap allocation */
    uchar*  top;      /* Top of swap map */
    Rendez  r;      /* Pager kproc idle sleep */
    ulong highwater;    /* Pager start threshold */
    ulong headroom;   /* Space pager frees under highwater */
  
    //extra
    Lock;       /* Free map lock */
};
/*e: struct Swapalloc */
extern struct Swapalloc swapalloc;
/*e: portdat_memory.h */
