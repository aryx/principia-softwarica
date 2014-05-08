/*s: portdat_concurrency.h */

//*****************************************************************************
// Mutual exclusion
//*****************************************************************************

// tas < Lock < QLock|RWLock|Ref

// used to be in 386/ but the fields were used from port/ so must be portable!
/*s: struct Lock */
struct Lock
{
    ulong key; // 0 when unset, 0xDEADDEAD when acquired, could be a bool

    // option<ref<Proc>>, None when key = 0
    Proc  *p; // the process locking should be the same unlocking

    // virt_addr?
    uintptr pc; // for debugging

    /*s: Lock ilock fields */
    bool_ushort isilock; // false when from lock(), true when from ilock()
    ulong sr; // saved priority level when using ilock() to restore in iunlock()
    /*e: Lock ilock fields */
    /*s: Lock other fields */
    // option<ref<Mach>>, None when key = 0?
    Mach  *m; // not that used, only in iprintcanlock apparently
    /*e: Lock other fields */
    /*s: Lock debugging fields */
    // debugging
    //#ifdef LOCKCYCLES
    long  lockcycles;
    //#endif
    /*e: Lock debugging fields */
};
/*e: struct Lock */

// Kernel basic lock with Queue (renamed to avoid ambiguity with libc.h Qlock)
/*s: struct QLock */
struct KQLock
{
    bool  locked;   /* flag */
  
    // list<ref<Proc>> (next = Proc.qnext)
    Proc  *head;    /* next process waiting for object */
    // list<ref<Proc>> (direct access to tail, queue)
    Proc  *tail;    /* last process waiting for object */
  
    uintptr qpc;    /* pc of the holder */ // for debugging?
  
    Lock  use;    /* to access Qlock structure */
};
/*e: struct QLock */

// Read/Write lock
/*s: struct RWlock */
struct RWlock
{
    int readers;  /* number of readers */
    int writer;   /* number of writers */
  
    // list<ref<Proc>> (next = Proc.qnext)
    Proc  *head;    /* list of waiting processes */
    // list<ref<Proc>>
    Proc  *tail;
    // option<ref<Proc>> (direct access to tail, queue)
    Proc  *wproc;   /* writing proc */
  
    uintptr wpc;    /* pc of writer */
  
    Lock  use;
};
/*e: struct RWlock */

// For reference counting shared things (e.g. a Page)
/*s: struct Ref */
struct Ref
{
    long  ref;
    Lock;
};
/*e: struct Ref */

//*****************************************************************************
// Synchronization
//*****************************************************************************

// defined in this directory but no functions are operating on in this dir
/*s: struct Rendez */
struct Rendez
{
    // ??
    Proc  *p;
  
    Lock;
};
/*e: struct Rendez */

/*s: struct Sema */
struct Sema
{
    long  *addr;
    int waiting;
  
    //list<Sema> of ??
    Sema  *next;
    Sema  *prev;
    Rendez;
};
/*e: struct Sema */

// see also Waitq in portdat_processes.h?
/*e: portdat_concurrency.h */
