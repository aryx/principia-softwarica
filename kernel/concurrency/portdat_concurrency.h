/*s: portdat_concurrency.h */

//*****************************************************************************
// Mutual exclusion
//*****************************************************************************

// arch_tas() < Lock|ILock < QLock|RWLock

// This used to be in <arch>/, but its fields are used from port/, so the 
// code must be portable!
/*s: struct Lock */
struct Lock
{
    ulong key; // 0 when unset, 1 (or 0xDEADDEAD) when acquired, could be a bool

    /*s: [[Lock]] ilock fields */
    bool_ushort isilock; // false when from lock(), true when from ilock()
    ulong sr; // saved priority level when using ilock() to restore in iunlock()
    /*e: [[Lock]] ilock fields */
    /*s: [[Lock]] debugging fields */
    // for debugging, the caller who did the lock()
    kern_addr pc; 
    /*x: [[Lock]] debugging fields */
    // option<ref<Proc>>, None when key == 0
    Proc  *p; // the process who did the locking should be the same unlocking
    /*x: [[Lock]] debugging fields */
    //#ifdef LOCKCYCLES
    long  lockcycles;
    //#endif
    /*e: [[Lock]] debugging fields */
    /*s: [[Lock]] other fields */
    // option<ref<Cpu>>, None when key = 0?
    Cpu  *cpu; // not that used, only in iprintcanlock apparently
    /*e: [[Lock]] other fields */
};
/*e: struct Lock */

/*s: struct ILock */
typedef Lock ILock;
/*e: struct ILock */

/*s: struct QLock */
// Kernel basic lock with Queue (renamed to avoid ambiguity with libc.h QLock)
struct KQLock
{
    bool  locked;   /* flag */
  
    // list<ref<Proc>> (next = Proc.qnext)
    Proc  *head;    /* next process waiting for object */
    // option<ref<Proc>> (direct access to tail, queue)
    Proc  *tail;    /* last process waiting for object */

    /*s: [[KQLock]] debugging fields */
    kern_addr qpc;    /* pc of the holder */ // for debugging?
    /*e: [[KQLock]] debugging fields */
  
    Lock  use;    /* to access Qlock structure */
};
/*e: struct QLock */

/*s: struct RWlock */
struct RWlock
{
    int readers;  /* number of readers */
    bool writer;   /* have a writer? */
  
    // list<ref<Proc>> (next = Proc.qnext)
    Proc  *head;    /* list of waiting processes */
    // option<ref<Proc>> (direct access to tail, queue)
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
    long ref;
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
    // option<ref<Proc>>
    Proc  *p; // sleeping process
    Lock;
};
/*e: struct Rendez */

/*s: struct Sema */
// user level semaphores, used to implement user-level lock, 
// see libc/port/lock.c
struct Sema
{
    long  *addr; // value stored in user space!
    bool waiting;
  
    //list<Sema> of Segment.sema
    Sema  *next;
    Sema  *prev;

    Rendez;
};
/*e: struct Sema */

// see also Waitq in portdat_processes.h?
/*e: portdat_concurrency.h */
