
// see also Lock (interdepends on Proc) in 386/

struct KQLock
{
	Lock	use;		/* to access Qlock structure */
	Proc	*head;		/* next process waiting for object */
	Proc	*tail;		/* last process waiting for object */
	int	locked;		/* flag */
	uintptr	qpc;		/* pc of the holder */
};

struct RWlock
{
	Lock	use;
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

// see also Sema, Rendez in portdat_memory.h
// see also Rgrp, Fgrp, Pgrp in portdat_processes.h?  and Egrp in files.h?
// see also Waitq in portdat_processes.h?
