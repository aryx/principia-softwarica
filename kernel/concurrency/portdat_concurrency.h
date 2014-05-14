/*s: portdat_concurrency.h */

//*****************************************************************************
// Mutual exclusion
//*****************************************************************************

// tas < Lock < QLock|RWLock

// used to be in 386/ but the fields were used from port/ so must be portable!
/*s: struct Lock */
struct Lock
{
    ulong key; // 0 when unset, 0xDEADDEAD when acquired, could be a bool

    // option<ref<Proc>>, None when key = 0
    Proc  *p; // the process locking should be the same unlocking

    // virt_addr?
    uintptr pc; // for debugging

    /*s: [[Lock]] ilock fields */
    bool_ushort isilock; // false when from lock(), true when from ilock()
    ulong sr; // saved priority level when using ilock() to restore in iunlock()
    /*e: [[Lock]] ilock fields */
    /*s: [[Lock]] other fields */
    // option<ref<Mach>>, None when key = 0?
    Mach  *m; // not that used, only in iprintcanlock apparently
    /*e: [[Lock]] other fields */
    /*s: [[Lock]] debugging fields */
    //#ifdef LOCKCYCLES
    long  lockcycles;
    //#endif
    /*e: [[Lock]] debugging fields */
};
/*e: struct Lock */

/*s: struct QLock */
// Kernel basic lock with Queue (renamed to avoid ambiguity with libc.h Qlock)
struct KQLock
{
    bool  locked;   /* flag */
  
    // list<ref<Proc>> (next = Proc.qnext)
    Proc  *head;    /* next process waiting for object */
    // list<ref<Proc>> (direct access to tail, queue)
    Proc  *tail;    /* last process waiting for object */
  
    kern_addr qpc;    /* pc of the holder */ // for debugging?
  
    Lock  use;    /* to access Qlock structure */
};
/*e: struct QLock */

// Read/Write lock
/*s: struct RWlock */
struct RWlock
{
    int readers;  /* number of readers */
    bool writer;   /* number of writers */
  
    // list<ref<Proc>> (next = Proc.qnext)
    Proc  *head;    /* list of waiting processes */
    // list<ref<Proc>> (direct access to tail, queue)
    Proc  *tail;
    // option<ref<Proc>> 
    Proc  *wproc;   /* writing proc */
  
    uintptr wpc;    /* pc of writer */
  
    Lock  use;
};
/*e: struct RWlock */

//*****************************************************************************
// Atomicity
//*****************************************************************************

/*s: struct Ref */
// For reference counting shared things (e.g. a Page)
struct Ref
{
    long  ref;
    Lock;
};
/*e: struct Ref */
/*s: struct Counter */
typedef struct Ref Counter;
/*e: struct Counter */

//*****************************************************************************
// Synchronization
//*****************************************************************************

// defined in this directory but no functions are operating on it in this dir
/*s: struct Rendez */
struct Rendez
{
    // ??
    Proc  *p;
    Lock;
};
/*e: struct Rendez */

/*s: struct Sema */
// user level semaphores, used to implement user-level lock, 
// see libc/port/lock.c
struct Sema
{
    long  *addr; // value stored in user space!
    int waiting;
  
    //list<Sema> of Segment.sema
    Sema  *next;
    Sema  *prev;

    Rendez;
};
/*e: struct Sema */

// see also Waitq in portdat_processes.h?
/*e: portdat_concurrency.h */
