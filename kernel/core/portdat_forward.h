
typedef struct Alarms	Alarms;
typedef struct Block	Block;
typedef struct Chan	Chan;
typedef struct Cmdbuf	Cmdbuf;
typedef struct Cmdtab	Cmdtab;
typedef struct Confmem	Confmem;
typedef struct Dev	Dev;
typedef struct Dirtab	Dirtab;
typedef struct Edf	Edf;
typedef struct Egrp	Egrp;
typedef struct Evalue	Evalue;
typedef struct Execvals	Execvals;
typedef struct Fgrp	Fgrp;
typedef struct DevConf	DevConf;
typedef struct KImage	KImage;
typedef struct Log	Log;
typedef struct Logflag	Logflag;
typedef struct Mntcache Mntcache;
typedef struct Mount	Mount;
typedef struct Mntrpc	Mntrpc;
typedef struct Mntwalk	Mntwalk;
typedef struct Mnt	Mnt;
typedef struct Mhead	Mhead;
typedef struct Note	Note;
typedef struct Page	Page;
typedef struct Path	Path;
typedef struct Palloc	Palloc;
typedef struct Pallocmem	Pallocmem;
//now in dat.h: typedef struct Perf	Perf;
typedef struct PhysUart	PhysUart;
typedef struct Pgrp	Pgrp;
typedef struct Physseg	Physseg;
typedef struct Proc	Proc;
typedef struct Pte	Pte;
typedef struct KQLock	QLock;
typedef struct Queue	Queue;
typedef struct Ref	Ref;
typedef struct Rendez	Rendez;
typedef struct Rgrp	Rgrp;
typedef struct RWlock	RWlock;
typedef struct Sargs	Sargs;
typedef struct Schedq	Schedq;
typedef struct Segment	Segment;
typedef struct Sema	Sema;
typedef struct Timer	Timer;
typedef struct Timers	Timers;
typedef struct Uart	Uart;
typedef struct Waitq	Waitq;
typedef struct Walkqid	Walkqid;
typedef struct Watchdog	Watchdog;
typedef int    Devgen(Chan*, char*, Dirtab*, int, int, Dir*);

// was in cache.c
typedef struct Extent Extent;
typedef struct Mntcache Mntcache;

// was in edf.h
typedef struct Edf		Edf;

// was in qio.c
typedef struct Queue	Queue;


#pragma incomplete DevConf
#pragma incomplete Edf
#pragma incomplete Mntcache
#pragma incomplete Mntrpc
#pragma incomplete Queue
#pragma incomplete Timers
