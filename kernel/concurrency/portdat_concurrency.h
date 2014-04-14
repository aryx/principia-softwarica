
// There are different code "regions": 
// - user code, 
// - kernel init code
// - kernel code of kernel processes (e.g. the alarm kernel process)
// - kernel code of syscalls (soft interrupt), 
// - kernel code of interrupt handlers (hard interrupt).
//
// There is no mutual exclusion need between user and kernel code. 
// Same for init code as only one processor is used during the
// uninterrupted sequential initialization.
//
// For the kernel code one wants mutual exclusion because of possible race 
// on shared data structures between the syscalls themselves when run
// on different processors (or even when run on one processor as one syscall
// can be interrupted causing a scheduling that will then lead later to
// another syscall), but also between the syscalls and interrupts.
// The flow of control on one processor can be 
//  - User -> Syscall, 
//  - User -> Interrupt,
//  - or even User -> Syscall -> Interrupt. 
//  - one can even have User -> Syscall -> Interrupt -> Interrupt!!
// This is on one processor. Multiple processors lead to more combinations
// where 2 processors can run at the same time 2 interrupt handlers for instance.
// 
// One must take care when using locks inside interrupts as one can deadlock
// if the same lock was used in the enclosing syscall (hence ilock/iunlock)

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
