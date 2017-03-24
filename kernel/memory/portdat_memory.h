/*s: portdat_memory.h */

// see also Arch_KMap in <arch>/ (but used in port)

//*****************************************************************************
// Page < Pagetable (can be filled by KImage) < Segment
//*****************************************************************************

// All the ref<Page> here are references to Pages in the array<Page> of 
// Palloc.pages (pool allocator)
// All the ref<Kimage> here are references to KImage in the ?? of 
// Imagealloc.free?

/*s: enum modref */
enum Modref 
{
    PG_NOTHING = 0x00, // nothing

    PG_MOD    = 0x01,   /* software modified bit */
    PG_REF    = 0x02,   /* software referenced bit */
};
/*e: enum modref */

/*s: enum Cachectl */
enum Cachectl {
    PG_NOFLUSH	= 0,
    PG_TXTFLUSH	= 1,		/* flush dcache and invalidate icache */

    PG_NEWCOL	= 3,		/* page has been recolored */
};
/*e: enum Cachectl */

/*s: struct Page */
// Page metadata. We will allocate as many Page as to cover all physical memory
// available for the user. xalloc'ed in Palloc.pages
struct Page
{
    phys_addr pa;     /* Physical address in memory */
    virt_addr va;     /* Virtual address for user */

    // Why not Ref? to save space (same reason they use char below)
    // but that means needs to use Lock below to access this non-atomic ref.
    ushort  ref;      /* Reference count */ // Pages are shared!

    // set<enum<modref>>
    char  modref;     /* Simulated modify/reference bits */

    /*s: [[Page]] other fields */
    // option<ref<Kimage>>
    KImage  *image;     /* Associated binary image or swap image */
    ulong daddr;      /* Disc address on image */
    /*x: [[Page]] other fields */
    ulong gen;      /* Generation counter for swap */
    /*x: [[Page]] other fields */
    // int [0..NCOLOR[
    char color;			/* Cache coloring */
    /*x: [[Page]] other fields */
    // array<enum<Cachectl>>
    char cachectl[MAXCPUS];	/* Cache flushing control for putmmu */
    /*e: [[Page]] other fields */

    // extra
    Lock;
    /*s: [[Page]] extra fields */
    // list<ref<Page>> Palloc.head, or Proc.mmuused or Proc.mmufree, or in mfreeseg
    Page  *next; /* Lru free list */ 
    // list<ref<Page>> Palloc.tail
    Page  *prev; 
    /*x: [[Page]] extra fields */
    // hash<daddr, ref<Page>> Palloc.hash
    Page  *hash; /* Image hash chains */ 
    /*e: [[Page]] extra fields */
};
/*e: struct Page */

/*s: type PageOrSwap */
typedef Page PageOrSwap;
/*e: type PageOrSwap */

/*s: struct Pagetable */
// ptalloc'ed (malloc'ed)
struct Pagetable
{
    //array<option<ref<PageOrSwap>> will map 1M of memory
    PageOrSwap  *pagetab[PAGETABSIZE];
  
    //to avoid iterate over all entries in pagetab
    // ref<ref<Page>> in Pagetable.pages
    Page  **first;    /* First used entry */
    // ref<ref<Page>> in Pagetable.pages
    Page  **last;     /* Last used entry */
};
/*e: struct Pagetable */

// used to be arch-specific, but many constants were used in port/ code
/*s: type PTExxx */
#define PTEVALID    (1<<0)
#define PTERONLY    0
#define PTEWRITE    (1<<1)
/*s: [[PTExxx]] other cases */
#define PTEUNCACHED (1<<4)
/*x: [[PTExxx]] other cases */
// x86 specific constants
#define PTEUSER   (1<<2)
#define PTEWT     (1<<3) // Write Through, e.g. VGA memory, mean??
#define PTESIZE   (1<<7) // Big pages (x86 extension)
#define PTEGLOBAL (1<<8) // do not clear from TLB kernel pages (x86 extension)
/*e: [[PTExxx]] other cases */
/*e: type PTExxx */

/*s: struct KImage */
// a KImage is essentially a channel to an executable or swapfile
struct KImage
{
    Chan  *c;     /* channel to text file */
    bool  notext;     /* no file associated */ // for swapfile
    /*s: [[Kimage]] other fields */
    Qid   qid;      /* Qid for page cache coherence */
    Chan  *mchan;
    Qid   mqid;
    ushort  type;     /* Device type of owning channel */
    /*e: [[Kimage]] other fields */
  
    // extra
    Ref;
    /*s: [[Kimage]] extra fields */
    // list<ref<Kimage>> of Imagealloc.free
    KImage  *next; /* Free list */ 
    // hash<qid.path, ref<Kimage>> Imagealloc.hash
    KImage  *hash; /* Qid hash chains */ 
    /*x: [[Kimage]] extra fields */
    // option<ref<Segment>>?
    Segment *s;     /* TEXT segment for image if running */
    /*e: [[Kimage]] extra fields */
};
/*e: struct KImage */

/*s: enum segtype */
/* Segment types */
enum Segtype
{
    SG_TEXT   = 00,
    SG_DATA   = 01,
    SG_BSS    = 02,
    SG_STACK  = 03,
    /*s: [[Segtype]] other cases */
    SG_SHARED = 04,
    /*x: [[Segtype]] other cases */
    SG_PHYSICAL = 05,
    /*e: [[Segtype]] other cases */
    SG_TYPE   = 07,   /* Mask type of segment */
  
    SG_RONLY  = 0040,   /* Segment is read only */
    /*s: [[Segtype]] other flags */
    SG_CEXEC  = 0100,   /* Detach at exec */
    /*e: [[Segtype]] other flags */
};
/*e: enum segtype */

/*s: constant PG_ONSWAP */
#define PG_ONSWAP 1
/*e: constant PG_ONSWAP */

/*s: function onswap */
#define onswap(s) (((kern_addr)s)&PG_ONSWAP)
/*e: function onswap */
/*s: function pagedout */
#define pagedout(s) (((ulong)s)==0 || onswap(s))
/*e: function pagedout */
/*s: function swapaddr */
#define swapaddr(s) (((ulong)s)&~PG_ONSWAP)
/*e: function swapaddr */

#define SEGMAXSIZE  (PAGEDIRSIZE*PAGETABMAPMEM)

/*s: struct Physseg */
struct Physseg
{
    ulong attr;     /* Segment attributes */
    char  *name;      /* Attach name */
    phys_addr pa;     /* Physical address */
    ulong size;     /* Maximum segment size in pages */
};
/*e: struct Physseg */

enum
{
    /*s: constant LRESPROF */
    LRESPROF  = 3,
    /*e: constant LRESPROF */
};

/*s: struct Segment */
// smalloc'ed by newseg()
struct Segment
{
    // enum<Segtype>
    ushort  type;   /* segment type */
  
    virt_addr base;   /* virtual base */
    virt_addr top;    /* virtual top */
    ulong size;   /* size in pages */ // top - base / BY2PG?
  
    // Kind of a page directory table. Points to smallpagedir if small enough.
    // array<option<ref_own<Pagetable>>>, smalloc'ed (or smallpagedir alias)
    // can map up to 2G of memory
    Pagetable **pagedir; // array of PAGEDIRSIZE max
    // array<option<ref_own<Pagetable>>
    Pagetable *smallpagedir[SMALLPAGEDIRSIZE];
    int pagedirsize; // nelem(pagedir)

  
    /*s: [[Segment]] other fields */
    KImage  *image;   /* text in file attached to this segment */
    /*x: [[Segment]] other fields */
    ulong fstart;   /* start address in file for demand load */
    ulong flen;   /* length of segment in file */
    /*x: [[Segment]] other fields */
    Sema sema;
    /*x: [[Segment]] other fields */
    ushort  steal;    /* Page stealer lock */
    /*x: [[Segment]] other fields */
    kern_addr2  profile;  /* Tick profile area */ // for TSEG only
    /*x: [[Segment]] other fields */
    bool	flushme;	/* maintain icache for this segment */
    /*x: [[Segment]] other fields */
    Physseg *pseg;
    /*x: [[Segment]] other fields */
    ulong mark;   /* portcountrefs */
    /*e: [[Segment]] other fields */
  
    // extra
    Ref; // LOCK ORDERING: always do lock(img); lock(s) ??
    QLock lk;
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
    // between addr and top the memory is free, this is the "hole"
    phys_addr addr; 
    phys_addr top; 
    ulong size; // top - addr

    // Extra
    /*s: [[Hole]] extra fields */
    Hole* next; // list<ref<Hole>> of Xalloc.sorted_holes or Xalloc.unused_slots
    /*e: [[Hole]] extra fields */
};
/*e: struct Hole */

/*s: struct Xhdr */
// What is the connection with Hole? A Hole that get used will gets
// its top and size decremented, and this newly allocated part will describe
// a portion of used memory, and at this memory there will be a header
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
struct Xalloc
{
    // array<Hole> where each Hole is linked to another hole
    Hole  hole[Nhole];
  
    // list<ref<Hole>> (next = Hole.next) list of free hole entries (addr=top=size=0)
    Hole* unused_slots; 

    // list<ref<Hole>> (next = Hole.next) memory holes, sorted by their top addr
    Hole* sorted_holes; 
  
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
// memory banks for user memory, similar to Confmem (and RMap)
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
    Pallocmem mem[4]; // = Conf.mem minus memory allocated for the kernel
    // sum of mem.npage (which should be conf.upages)
    ulong user;     /* how many user pages */
  
    // array<Page>, xalloc'ed in pageinit(), huge, cover physical user space
    Page  *pages; /* array of all pages */ 
  
    // list<ref<Page>> (next = Page.next), list of free pages
    Page  *head;      /* most recently used */
    // list<ref<Page>> (prev = Page.prev), list of free pages (backward)
    Page  *tail;      /* least recently used */

    ulong freecount;    /* how many pages on free list now */

    /*s: [[Palloc]] other fields */
    // hash<Page.daddr, ref<Page>> (next = Page.hash)>
    Page  *hash[PGHSIZE];
    Lock  hashlock;
    /*e: [[Palloc]] other fields */
  
    // extra
    Lock; // LOCK ORDERING: always do lock(&palloc); lock(&page)!!
    /*s: [[Palloc]] extra fields */
    Rendez  freememr; /* Sleep for free mem */ // hasfreepages()
    QLock pwait; /* Queue of procs waiting for memory */
    /*e: [[Palloc]] extra fields */
};
/*e: struct Palloc */
extern  Palloc  palloc;

/*s: constant NFREECHAN */
#define NFREECHAN 64
/*e: constant NFREECHAN */

/*s: function ihash */
#define IHASHSIZE 64
// actually internal to page.c, but important so here
#define ihash(qidpath)  imagealloc.hash[qidpath%IHASHSIZE]
/*e: function ihash */

/*s: struct Imagealloc */
// Image allocator (internal to segment.c, but important so here, singleton)
struct Imagealloc
{
    // hash<qid.path, ref<Kimage>> (next = Kimage.hash)
    KImage  *hash[IHASHSIZE];

    // list<ref<Kimage> (next = Kimage.next)
    KImage  *free; // originally  xalloc'ed in imageinit() (conf.nimage)
    QLock ireclaim; /* mutex on reclaiming free images */

    /*s: [[Imagealloc]] other fields */
    Chan  **freechan; /* free image channels */
    int nfreechan;  /* number of free channels */
    int szfreechan; /* size of freechan array */
    QLock fcreclaim;  /* mutex on reclaiming free channels */
    /*e: [[Imagealloc]] other fields */
 
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
    // array<byte> xalloc'ed in swapinit()
    // each idx represents a chunk of swapimage, each value a ref count
    byte*  swmap;      /* Base of swap map in memory */

    int free;     /* currently free swap pages */

    // ref<byte> in swmap
    byte*  alloc;     /* Round robin allocator */
    // ref<byte> in swmap
    byte*  top;      /* Top of swap map */

    // ref<byte> in swmap
    byte*  last;     /* Speed swap allocation */

    ulong highwater;    /* Pager start threshold */ // = 5% conf.upages
    ulong headroom;   /* Space pager frees under highwater */ // = 1.25*hw

    //extra
    Lock;       /* Free map lock */
    Rendez r;      /* Pager kproc idle sleep */ // needpages()
};
/*e: struct Swapalloc */
extern struct Swapalloc swapalloc;
/*e: portdat_memory.h */
