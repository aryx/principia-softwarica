/*s: portdat_concurrency.h */

//*****************************************************************************
// Mutual exclusion
//*****************************************************************************

// tas < Lock < QLock|RWLock|Ref

// used to be in 386/ but the fields were used from port/ so must be portable!
struct Lock
{
  ulong key; // 0 when unset, 0xDEADDEAD when acquired, could be a bool
  bool_ushort isilock; // false when from lock(), true when from ilock()

  ulong sr; // saved priority level when using ilock() to restore in iunlock()
  // virt_addr?
  ulong pc; // for debugging

  // option<ref<Proc>>, None when key = 0?
  Proc  *p; // the process locking should be the same unlocking
  // option<ref<Mach>>, None when key = 0?
  Mach  *m; // not that used, only in iprintcanlock apparently

  // debugging
//#ifdef LOCKCYCLES
  long  lockcycles;
//#endif
};


// Kernel basic lock with Queue (renamed to avoid ambiguity with libc.h Qlock)
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

// Read/Write lock
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

  ulong wpc;    /* pc of writer */

  Lock  use;
};

// For reference counting shared things (e.g. a Page)
struct Ref
{
  long  ref;

  Lock;
};

//*****************************************************************************
// Synchronization
//*****************************************************************************

// defined in this directory but no functions are operating on in this dir
struct Rendez
{
  // ??
  Proc  *p;

  Lock;
};

struct Sema
{
  long  *addr;
  int waiting;

  //list<Sema> of ??
  Sema  *next;
  Sema  *prev;
  Rendez;
};

// see also Waitq in portdat_processes.h?
/*e: portdat_concurrency.h */
