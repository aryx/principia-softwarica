/*s: portdat_memory.h */

// see also KMap in 386/ (but used in port)

//*****************************************************************************
// Page < Pagetable (can be filled by KImage) < Segment
//*****************************************************************************

// All the ref<Page> here are references to Pages in the array<Page> of 
// Palloc.pages (pool allocator)
// All the ref<Kimage> here are references to KImage in the ?? of 
// Imagealloc.free?

/*s: enum modref */
enum modref 
{
    PG_NOTHING = 0x00, // nothing
    PG_MOD    = 0x01,   /* software modified bit */
    PG_REF    = 0x02,   /* software referenced bit */
};
/*e: enum modref */

/*s: struct Page */
// Page metadata. We will allocate as many Page as to cover all physical memory
// available for the user. xalloc'ed in Palloc.pages
struct Page
{
    phys_addr pa;     /* Physical address in memory */
    virt_addr va;     /* Virtual address for user */

    /*s: [[Page]] other fields */
    ulong daddr;      /* Disc address on swap */
    /*x: [[Page]] other fields */
    // option<ref<Kimage>>
    KImage  *image;     /* Associated text or swap image */
    /*x: [[Page]] other fields */
    ulong gen;      /* Generation counter for swap */
    /*e: [[Page]] other fields */
  
    // Why not Ref? to save space (same reason they use char below)
    // but that means needs to use Lock below to access this non-atomic ref.
    ushort  ref;      /* Reference count */ // Pages are shared!
    // set<enum<modref>>
    char  modref;     /* Simulated modify/reference bits */

    // extra
    Lock;
    /*s: [[Page]] extra fields */
    // list<ref<Page>> Palloc.head, or Proc.mmuused or Proc.mmufree, or in mfreeseg
    Page  *next; /* Lru free list */ 
    // list<ref<Page>> Palloc.tail
    Page  *prev; 
    /*x: [[Page]] extra fields */
    // hash<daddr, list<ref<Page>>> Palloc.hash
    Page  *hash; /* Image hash chains */ 
    /*e: [[Page]] extra fields */
};
/*e: struct Page */

/*s: struct Pagetable */
// ptalloc'ed (malloc'ed)
struct Pagetable
{
    //array<option<ref<Page>> will map 1M of memory
    Page  *pagetab[PAGETABSIZE];
  
    //to avoid iterate over all entries in pagetab
    // ref<ref<Page>> in Pagetable.pages
    Page  **first;    /* First used entry */
    // ref<ref<Page>> in Pagetable.pages
    Page  **last;     /* Last used entry */
};
/*e: struct Pagetable */


/*s: struct KImage */
// a KImage is essentially a channel to a text file (an image of a binary)
// the image in memory for a portion of a given file.
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
    // hash<Qid.path, list<ref<Kimage>>> Imagealloc.hash
    KImage  *hash; /* Qid hash chains */ 

    // option<ref<Segment>>?
    Segment *s;     /* TEXT segment for image if running */
};
/*e: struct KImage */


/*s: enum segtype */
/* Segment types */
enum segtype
{
    SG_TEXT   = 00,
    SG_DATA   = 01,
    SG_BSS    = 02,
    SG_STACK  = 03,
    /*s: enum segtype other cases */
        SG_SHARED = 04,
    /*x: enum segtype other cases */
        SG_PHYSICAL = 05,
    /*e: enum segtype other cases */
    SG_TYPE   = 07,   /* Mask type of segment */
  
    SG_RONLY  = 0040,   /* Segment is read only */
    /*s: enum segtype other flags */
        SG_CEXEC  = 0100,   /* Detach at exec */
    /*e: enum segtype other flags */
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
    // enum<segtype>
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
    ulong fstart;   /* start address in file for demand load */
    ulong flen;   /* length of segment in file */
    /*x: [[Segment]] other fields */
    Sema sema;
    /*x: [[Segment]] other fields */
    ushort  steal;    /* Page stealer lock */
    /*x: [[Segment]] other fields */
    kern_addr2  profile;  /* Tick profile area */ // for TSEG only
    /*x: [[Segment]] other fields */
        ulong mark;   /* portcountrefs */
    /*x: [[Segment]] other fields */
    Physseg *pseg;
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
    // extra
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
        // hash<Page.daddr, list<ref<Page>> (next = Page.hash)>
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
#define ihash(s)  imagealloc.hash[s%IHASHSIZE]
/*e: function ihash */

/*s: struct Imagealloc */
// Image allocator (internal to segment.c, but important so here, singleton)
struct Imagealloc
{
    // array<Kimage>?  xalloc'ed in initimage() (conf.nimage)
    KImage  *free; 
    // hash<Qid.path, list<ref<Kimage>>
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
    // array<byte> xalloc'ed in swapinit()
    // each idx represents a chunk of swapimage, each value a ref count
    uchar*  swmap;      /* Base of swap map in memory */

    int free;     /* currently free swap pages */

    // ref<byte> in swmap
    uchar*  alloc;     /* Round robin allocator */
    // ref<byte> in swmap
    uchar*  top;      /* Top of swap map */

    // ref<byte> in swmap
    uchar*  last;     /* Speed swap allocation */

    ulong highwater;    /* Pager start threshold */ // = 5% conf.upages
    ulong headroom;   /* Space pager frees under highwater */ // = 1.25*hw

    //extra
    Lock;       /* Free map lock */
    Rendez r;      /* Pager kproc idle sleep */ // needpages()
};
/*e: struct Swapalloc */
extern struct Swapalloc swapalloc;
/*e: portdat_memory.h */
