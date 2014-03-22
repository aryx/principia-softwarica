#include "../port/portdat_forward.h"

// see also Mach and Conf in 386/ (but used in port)

#include "../port/portdat_concurrency.h"

enum
{
  // used by portdat_processes.h
	RENDLOG	=	5,
	RENDHASH =	1<<RENDLOG,	/* Hash to lookup rendezvous tags */
	MNTLOG	=	5,
	MNTHASH =	1<<MNTLOG,	/* Hash to walk mount table */
	NFD =		100,		/* per process file descriptors */
	PGHLOG  =	9,
  // used by portdat_memory.h
	PGHSIZE	=	1<<PGHLOG,	/* Page hash for image lookup */
};
#define REND(p,s)	((p)->rendhash[(s)&((1<<RENDLOG)-1)])
#define MOUNTH(p,qid)	((p)->mnthash[(qid).path&((1<<MNTLOG)-1)])

#include "../port/portdat_memory.h"
#include <fcall.h>
#include "../port/portdat_files.h"
#include "../port/portdat_processes.h"

#ifndef STAGESIZE
#define STAGESIZE 64
#endif
#include "../port/portdat_buses.h"

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
// used by devcons.c
#ifndef KMESGSIZE
#define KMESGSIZE (16*1024)
#endif
// used by 386/pci.c
#ifndef PCICONSSIZE
#define PCICONSSIZE (16*1024)
#endif
//unused: //#define MAXBY2PG BY2PG		/* rounding for UTZERO in executables */

// convenient constants
enum
{
	PRINTSIZE =	256,
  //unused:	MAXCRYPT = 	127,
	NUMSIZE	=	12,		/* size of formatted number */
	MB =		(1024*1024),
	/* READSTR was 1000, which is way too small for usb's ctl file */
	READSTR =	4000,		/* temporary buffer size for device reads */
};

#define DEVDOTDOT -1

enum
{
	DELTAFD	= 20		/* incremental increase in Fgrp.fd's */
};

enum
{
	LRESPROF	= 3,
};

#include "../port/portdat_globals.h"

#include "../port/portdat_console.h"
#include "../port/portdat_misc.h"

extern	char*	conffile;
extern	int	cpuserver;
extern	Dev*	conf_devtab[];
extern	char	hostdomain[];
extern	uchar	initcode[];
extern 	Ref	noteidalloc;
extern	char*	statename[];
extern	uint	qiomaxatomic;

// defined in syscall/systab.h
extern	char*	sysctab[];
extern	int	nsyscall;

struct Execvals {
	uvlong	entry;
	ulong	textsize;
	ulong	datasize;
};
int	(*parseboothdr)(Chan *, ulong, Execvals *);

//unused:
//enum
//{
//	NSMAX	=	1000,
//	NSLOG	=	7,
//	NSCACHE	=	(1<<NSLOG),
//};

//unused:
//enum
//{
//	NCMDFIELD = 128
//};

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

#pragma	varargck	type	"I"	uchar*
#pragma	varargck	type	"V"	uchar*
#pragma	varargck	type	"E"	uchar*
#pragma	varargck	type	"M"	uchar*
