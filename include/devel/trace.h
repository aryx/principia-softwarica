/*s: trace.h */
/*s: enum Tevent */
enum Tevent {
    SReady = 0,		/* runnable but not running  */
    SRun,		/* running best effort */
    SDead,		/* proc dies */
    SSleep,		/* blocked */
    SUser,		/* user event */

    /* real-time extensions */
    /*s: [[enum Tevent]] real-time scheduling events */
    SAdmit,	/* Edf admit */
    SRelease,	/* Edf release, waiting to be scheduled */
    SYield,		/* blocked waiting for release */
    SSlice,		/* slice exhausted */
    SDeadline,	/* proc's deadline */
    SExpel,		/* Edf expel */
    SInts,		/* Interrupt start */
    SInte,		/* Interrupt end */
    /*e: [[enum Tevent]] real-time scheduling events */

    Nevent, // must be last
};
/*e: enum Tevent */

/*s: struct Traceevent */
struct Traceevent {
    ulong	pid;	
    // enum<tevent>
    ulong	etype;	/* Event type */
    vlong	time;	/* time stamp  */ // dimension?
};
/*e: struct Traceevent */

typedef enum Tevent Tevent;
typedef struct Traceevent	Traceevent;
/*e: trace.h */
