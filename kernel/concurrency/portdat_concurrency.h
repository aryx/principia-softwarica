
// see also Lock (interdepends on Proc) in 386/ (but used in port)

struct KQLock
{
	Lock	use;		/* to access Qlock structure */

  // see also Proc.qnext for getting the full chain from head to tail.
	Proc	*head;		/* next process waiting for object */
	Proc	*tail;		/* last process waiting for object */

	bool	locked;		/* flag */
	uintptr	qpc;		/* pc of the holder */
};

struct RWlock
{
	Lock	use;

  // see also Proc.qnext for getting the full chain from head to tail.
	Proc	*head;		/* list of waiting processes */
	Proc	*tail;

	ulong	wpc;		/* pc of writer */
	Proc	*wproc;		/* writing proc */
	int	readers;	/* number of readers */
	int	writer;		/* number of writers */
};

struct Ref
{
	Lock;
	long	ref;
};


// defined in this directory but no functions are operating on in this dir
struct Rendez
{
	Lock;
	Proc	*p;
};

struct Sema
{
	Rendez;
	long	*addr;
	int	waiting;

	Sema	*next;
	Sema	*prev;
};

// see also Waitq in portdat_processes.h?
// see also Rgrp, Fgrp, Pgrp in portdat_processes.h?  and Egrp in files.h?
