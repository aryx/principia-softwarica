#include "../port/portdat_forward.h"

#include "../port/portdat_core.h"

#include <fcall.h>

#define HOWMANY(x, y)	(((x)+((y)-1))/(y))
#define ROUNDUP(x, y)	(HOWMANY((x), (y))*(y))	/* ceiling */
#define ROUNDDN(x, y)	(((x)/(y))*(y))		/* floor */
#define	ROUND(s, sz)	(((s)+(sz-1))&~(sz-1))
#define	PGROUND(s)	ROUNDUP(s, BY2PG)
#define MIN(a, b)	((a) < (b)? (a): (b))
#define MAX(a, b)	((a) > (b)? (a): (b))

/*
 * For multi-bit fields use FIELD(v, o, w) where 'v' is the value
 * of the bit-field of width 'w' with LSb at bit offset 'o'.
 */
#define FIELD(v, o, w)	(((v) & ((1<<(w))-1))<<(o))

#define FCLR(d, o, w)	((d) & ~(((1<<(w))-1)<<(o)))
#define FEXT(d, o, w)	(((d)>>(o)) & ((1<<(w))-1))
#define FINS(d, o, w, v) (FCLR((d), (o), (w))|FIELD((v), (o), (w)))
#define FSET(d, o, w)	((d)|(((1<<(w))-1)<<(o)))

#define FMASK(o, w)	(((1<<(w))-1)<<(o))

/* let each port override any of these */
#ifndef KMESGSIZE
#define KMESGSIZE (16*1024)
#endif
#ifndef PCICONSSIZE
#define PCICONSSIZE (16*1024)
#endif
#ifndef STAGESIZE
#define STAGESIZE 64
#endif
#ifndef MAXBY2PG
#define MAXBY2PG BY2PG		/* rounding for UTZERO in executables */
#endif


struct Alarms
{
	QLock;
	Proc	*head;
};


/*
 * Access types in namec & channel flags
 */
enum
{
	Aaccess,			/* as in stat, wstat */
	Abind,				/* for left-hand-side of bind */
	Atodir,				/* as in chdir */
	Aopen,				/* for i/o */
	Amount,				/* to be mounted or mounted upon */
	Acreate,			/* is to be created */
	Aremove,			/* will be removed by caller */

	COPEN	= 0x0001,		/* for i/o */
	CMSG	= 0x0002,		/* the message channel for a mount */
/*rsc	CCREATE	= 0x0004,		/* permits creation if c->mnt */
	CCEXEC	= 0x0008,		/* close on exec */
	CFREE	= 0x0010,		/* not in use */
	CRCLOSE	= 0x0020,		/* remove on close */
	CCACHE	= 0x0080,		/* client cache */
};

/* flag values */
enum
{
	BINTR	=	(1<<0),
	BFREE	=	(1<<1),
	Bipck	=	(1<<2),		/* ip checksum */
	Budpck	=	(1<<3),		/* udp checksum */
	Btcpck	=	(1<<4),		/* tcp checksum */
	Bpktck	=	(1<<5),		/* packet checksum */
};

struct Block
{
	long	ref;
	Block*	next;
	Block*	list;
	uchar*	rp;			/* first unconsumed byte */
	uchar*	wp;			/* first empty byte */
	uchar*	lim;			/* 1 past the end of the buffer */
	uchar*	base;			/* start of the buffer */
	void	(*free)(Block*);
	ushort	flag;
	ushort	checksum;		/* IP checksum of complete packet (minus media header) */
};

#define BLEN(s)	((s)->wp - (s)->rp)
#define BALLOC(s) ((s)->lim - (s)->base)

struct Dev
{
	int	dc;
	char*	name;

	void	(*reset)(void);
	void	(*init)(void);
	void	(*shutdown)(void);
	Chan*	(*attach)(char*);
	Walkqid*(*walk)(Chan*, Chan*, char**, int);
	int	(*stat)(Chan*, uchar*, int);
	Chan*	(*open)(Chan*, int);
	void	(*create)(Chan*, char*, int, ulong);
	void	(*close)(Chan*);
	long	(*read)(Chan*, void*, long, vlong);
	Block*	(*bread)(Chan*, long, ulong);
	long	(*write)(Chan*, void*, long, vlong);
	long	(*bwrite)(Chan*, Block*, ulong);
	void	(*remove)(Chan*);
	int	(*wstat)(Chan*, uchar*, int);
	void	(*power)(int);	/* power mgt: power(1) => on, power (0) => off */
	int	(*config)(int, char*, DevConf*);	/* returns nil on error */

	/* not initialised */
	int	attached;				/* debugging */
};

struct Dirtab
{
	char	name[KNAMELEN];
	Qid	qid;
	vlong	length;
	long	perm;
};

struct Walkqid
{
	Chan	*clone;
	int	nqid;
	Qid	qid[1];
};

enum
{
	NSMAX	=	1000,
	NSLOG	=	7,
	NSCACHE	=	(1<<NSLOG),
};

struct Mntwalk				/* state for /proc/#/ns */
{
	int	cddone;
	Mhead*	mh;
	Mount*	cm;
};

struct Mnt
{
	Lock;
	/* references are counted using c->ref; channels on this mount point incref(c->mchan) == Mnt.c */
	Chan	*c;		/* Channel to file service */
	Proc	*rip;		/* Reader in progress */
	Mntrpc	*queue;		/* Queue of pending requests on this channel */
	ulong	id;		/* Multiplexer id for channel check */
	Mnt	*list;		/* Free list */
	int	flags;		/* cache */
	int	msize;		/* data + IOHDRSZ */
	char	*version;	/* 9P version */
	Queue	*q;		/* input queue */
};


struct Swapalloc
{
	Lock;				/* Free map lock */
	int	free;			/* currently free swap pages */
	uchar*	swmap;			/* Base of swap map in memory */
	uchar*	alloc;			/* Round robin allocator */
	uchar*	last;			/* Speed swap allocation */
	uchar*	top;			/* Top of swap map */
	Rendez	r;			/* Pager kproc idle sleep */
	ulong	highwater;		/* Pager start threshold */
	ulong	headroom;		/* Space pager frees under highwater */
};

struct Swapalloc swapalloc;






enum
{
	DELTAFD	= 20		/* incremental increase in Fgrp.fd's */
};

struct Pallocmem
{
	ulong base;
	ulong npage;
};

struct Palloc
{
	Lock;
	Pallocmem	mem[4];
	Page	*head;			/* most recently used */
	Page	*tail;			/* least recently used */
	ulong	freecount;		/* how many pages on free list now */
	Page	*pages;			/* array of all pages */
	ulong	user;			/* how many user pages */
	Page	*hash[PGHSIZE];
	Lock	hashlock;
	Rendez	r;			/* Sleep for free mem */
	QLock	pwait;			/* Queue of procs waiting for memory */
};



enum
{
	RFNAMEG		= (1<<0),
	RFENVG		= (1<<1),
	RFFDG		= (1<<2),
	RFNOTEG		= (1<<3),
	RFPROC		= (1<<4),
	RFMEM		= (1<<5),
	RFNOWAIT	= (1<<6),
	RFCNAMEG	= (1<<10),
	RFCENVG		= (1<<11),
	RFCFDG		= (1<<12),
	RFREND		= (1<<13),
	RFNOMNT		= (1<<14),
};



struct Schedq
{
	Lock;
	Proc*	head;
	Proc*	tail;
	int	n;
};


enum
{
	PRINTSIZE =	256,
	MAXCRYPT = 	127,
	NUMSIZE	=	12,		/* size of formatted number */
	MB =		(1024*1024),
	/* READSTR was 1000, which is way too small for usb's ctl file */
	READSTR =	4000,		/* temporary buffer size for device reads */
};

struct Execvals {
	uvlong	entry;
	ulong	textsize;
	ulong	datasize;
};




extern	Conf	conf;
extern	char*	conffile;
extern	int	cpuserver;
extern	Dev**	devtab/*[]*/;
extern	Dev*	conf_devtab[];
extern	char*	eve;
extern	char	hostdomain[];
extern	uchar	initcode[];
extern	int	kbdbuttons;
extern	Queue*	kbdq;
extern	Queue*	kprintoq;
extern 	Ref	noteidalloc;
extern	int	nsyscall;
extern	Palloc	palloc;
int	(*parseboothdr)(Chan *, ulong, Execvals *);
extern	Queue*	serialoq;
extern	char*	statename[];
extern	Image	swapimage;
extern	char*	sysname;
extern	uint	qiomaxatomic;
extern	char*	sysctab[];

Watchdog* watchdog;
int	watchdogon;

enum
{
	LRESPROF	= 3,
};

/*
 *  action log
 */
struct Log {
	Lock;
	int	opens;
	char*	buf;
	char	*end;
	char	*rptr;
	int	len;
	int	nlog;
	int	minread;

	int	logmask;	/* mask of things to debug */

	QLock	readq;
	Rendez	readr;
};

struct Logflag {
	char*	name;
	int	mask;
};

enum
{
	NCMDFIELD = 128
};

struct Cmdbuf
{
	char	*buf;
	char	**f;
	int	nf;
};

struct Cmdtab
{
	int	index;	/* used by client to switch on result */
	char	*cmd;	/* command name */
	int	narg;	/* expected #args; 0 ==> variadic */
};

/*
 *  routines to access UART hardware
 */
struct PhysUart
{
	char*	name;
	Uart*	(*pnp)(void);
	void	(*enable)(Uart*, int);
	void	(*disable)(Uart*);
	void	(*kick)(Uart*);
	void	(*dobreak)(Uart*, int);
	int	(*baud)(Uart*, int);
	int	(*bits)(Uart*, int);
	int	(*stop)(Uart*, int);
	int	(*parity)(Uart*, int);
	void	(*modemctl)(Uart*, int);
	void	(*rts)(Uart*, int);
	void	(*dtr)(Uart*, int);
	long	(*status)(Uart*, void*, long, long);
	void	(*fifo)(Uart*, int);
	void	(*power)(Uart*, int);
	int	(*getc)(Uart*);	/* polling versions, for iprint, rdb */
	void	(*putc)(Uart*, int);
};

enum {
	Stagesize=	STAGESIZE
};

/*
 *  software UART
 */
struct Uart
{
	void*	regs;			/* hardware stuff */
	void*	saveregs;		/* place to put registers on power down */
	char*	name;			/* internal name */
	ulong	freq;			/* clock frequency */
	int	bits;			/* bits per character */
	int	stop;			/* stop bits */
	int	parity;			/* even, odd or no parity */
	int	baud;			/* baud rate */
	PhysUart*phys;
	int	console;		/* used as a serial console */
	int	special;		/* internal kernel device */
	Uart*	next;			/* list of allocated uarts */

	QLock;
	int	type;			/* ?? */
	int	dev;
	int	opens;

	int	enabled;
	Uart	*elist;			/* next enabled interface */

	int	perr;			/* parity errors */
	int	ferr;			/* framing errors */
	int	oerr;			/* rcvr overruns */
	int	berr;			/* no input buffers */
	int	serr;			/* input queue overflow */

	/* buffers */
	int	(*putc)(Queue*, int);
	Queue	*iq;
	Queue	*oq;

	Lock	rlock;
	uchar	istage[Stagesize];
	uchar	*iw;
	uchar	*ir;
	uchar	*ie;

	Lock	tlock;			/* transmit */
	uchar	ostage[Stagesize];
	uchar	*op;
	uchar	*oe;
	int	drain;

	int	modem;			/* hardware flow control on */
	int	xonoff;			/* software flow control on */
	int	blocked;
	int	cts, dsr, dcd;	/* keep track of modem status */ 
	int	ctsbackoff;
	int	hup_dsr, hup_dcd;	/* send hangup upstream? */
	int	dohup;

	Rendez	r;
};

extern	Uart*	consuart;

void (*lprint)(char *, int);


struct Watchdog
{
	void	(*enable)(void);	/* watchdog enable */
	void	(*disable)(void);	/* watchdog disable */
	void	(*restart)(void);	/* watchdog restart */
	void	(*stat)(char*, char*);	/* watchdog statistics */
};


/* queue state bits,  Qmsg, Qcoalesce, and Qkick can be set in qopen */
enum
{
	/* Queue.state */
	Qstarve		= (1<<0),	/* consumer starved */
	Qmsg		= (1<<1),	/* message stream */
	Qclosed		= (1<<2),	/* queue has been closed/hungup */
	Qflow		= (1<<3),	/* producer flow controlled */
	Qcoalesce	= (1<<4),	/* coalesce packets on read */
	Qkick		= (1<<5),	/* always call the kick routine after qwrite */
};

#define DEVDOTDOT -1

#pragma	varargck	type	"I"	uchar*
#pragma	varargck	type	"V"	uchar*
#pragma	varargck	type	"E"	uchar*
#pragma	varargck	type	"M"	uchar*
