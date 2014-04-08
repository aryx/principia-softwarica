
// see also Lock (interdepends on Proc) in 386/ (but used in port)

// Lock < KQLock|RWLock|Ref

// Kernel basic lock with Queue (renamed to avoid ambiguity with libc.h Qlock)
struct KQLock
{
	bool	locked;		/* flag */

  // list<ref<Proc>> (next = Proc.qnext)
	Proc	*head;		/* next process waiting for object */
  // list<ref<Proc>> (next = Proc.??)
	Proc	*tail;		/* last process waiting for object */

	uintptr	qpc;		/* pc of the holder */ // for debugging?

	Lock	use;		/* to access Qlock structure */
};

// Read/Write lock
struct RWlock
{
	int	readers;	/* number of readers */
	int	writer;		/* number of writers */

  // list<ref<Proc>> (next = Proc.qnext)
	Proc	*head;		/* list of waiting processes */
  // list<ref<Proc>> (next = Proc.qnext)??
	Proc	*tail;
  // option<ref<Proc>>
	Proc	*wproc;		/* writing proc */

	ulong	wpc;		/* pc of writer */

	Lock	use;
};

// For reference counting shared things (e.g. a Page)
struct Ref
{
	long	ref;

	Lock;
};


// defined in this directory but no functions are operating on in this dir
struct Rendez
{
  // ??
	Proc	*p;

	Lock;
};

struct Sema
{
	long	*addr;
	int	waiting;

  //list<Sema> of ??
	Sema	*next;
	Sema	*prev;
	Rendez;
};

// see also Waitq in portdat_processes.h?
