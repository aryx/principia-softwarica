#include "../port/portdat_forward.h"

#include <fcall.h>

// now have a reference to Fcall
#include "../port/portdat_core.h"

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

#include "../port/portdat_memory.h"
#include "../port/portdat_files.h"
#include "../port/portdat_processes.h"
#include "../port/portdat_buses.h"

//unused
//enum
//{
//	NSMAX	=	1000,
//	NSLOG	=	7,
//	NSCACHE	=	(1<<NSLOG),
//};

struct Execvals {
	uvlong	entry;
	ulong	textsize;
	ulong	datasize;
};

enum
{
	DELTAFD	= 20		/* incremental increase in Fgrp.fd's */
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
extern	KImage	swapimage;
extern	char*	sysname;
extern	uint	qiomaxatomic;
extern	char*	sysctab[];

enum
{
	LRESPROF	= 3,
};

//unused
/*
 *  action log
 */
//struct Log {
//	Lock;
//	int	opens;
//	char*	buf;
//	char	*end;
//	char	*rptr;
//	int	len;
//	int	nlog;
//	int	minread;
//
//	int	logmask;	/* mask of things to debug */
//
//	QLock	readq;
//	Rendez	readr;
//};
//
//struct Logflag {
//	char*	name;
//	int	mask;
//};
//
//enum
//{
//	NCMDFIELD = 128
//};

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


#define DEVDOTDOT -1

#pragma	varargck	type	"I"	uchar*
#pragma	varargck	type	"V"	uchar*
#pragma	varargck	type	"E"	uchar*
#pragma	varargck	type	"M"	uchar*
