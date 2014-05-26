enum Tevent {
	SReady = 0,		/* runnable but not running  */
	SRun,		/* running best effort */
	SDead,		/* proc dies */
	SSleep,		/* blocked */
	SUser,		/* user event */

    /* real-time extensions */
	SAdmit,	/* Edf admit */
	SRelease,	/* Edf release, waiting to be scheduled */
	SYield,		/* blocked waiting for release */
	SSlice,		/* slice exhausted */
	SDeadline,	/* proc's deadline */
	SExpel,		/* Edf expel */
	SInts,		/* Interrupt start */
	SInte,		/* Interrupt end */

	Nevent, // must be last
};

struct Traceevent {
	ulong	pid;	
	ulong	etype;	/* Event type */
	vlong	time;	/* time stamp  */
};

typedef enum Tevent Tevent;
typedef struct Traceevent	Traceevent;
