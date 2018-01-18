/*s: iostats/statfs.h */
/*s: constant [[DEBUGFILE]] */
/*
 * statfs.h - definitions for statistic gathering file server
 */

#define DEBUGFILE	"iostats.out"
/*e: constant [[DEBUGFILE]] */
/*s: constant [[DONESTR]] */
#define DONESTR		"done"
/*e: constant [[DONESTR]] */
/*s: constant [[DEBUG]] */
#define DEBUG		if(dbg)fprint
/*e: constant [[DEBUG]] */
/*s: constant [[MAXPROC]] */
#define MAXPROC		16
/*e: constant [[MAXPROC]] */
/*s: constant [[FHASHSIZE]] */
#define FHASHSIZE	64
/*e: constant [[FHASHSIZE]] */
/*s: function [[fidhash]] */
#define fidhash(s)	fhash[s%FHASHSIZE]
/*e: function [[fidhash]] */

/*s: enum [[_anon_ (iostats/statfs.h)]] */
enum{
    Maxfdata	= 8192,	/* max size of data in 9P message */
    Maxrpc		= 20000,/* number of RPCs we'll log */
};
/*e: enum [[_anon_ (iostats/statfs.h)]] */

typedef struct Fsrpc Fsrpc;
typedef struct Fid Fid;
typedef struct File File;
typedef struct Proc Proc;
typedef struct Stats Stats;
typedef struct Rpc Rpc;
typedef struct Frec Frec;

/*s: struct [[Frec]] */
struct Frec
{
    Frec	*next;
    char	*op;
    ulong	nread;
    ulong	nwrite;
    ulong	bread;
    ulong	bwrite;
    ulong	opens;
};
/*e: struct [[Frec]] */

/*s: struct [[Rpc]] */
struct Rpc
{
    char	*name;
    ulong	count;
    vlong	time;
    vlong	lo;
    vlong	hi;
    ulong	bin;
    ulong	bout;
};
/*e: struct [[Rpc]] */

/*s: struct [[Stats]] */
struct Stats
{
    ulong	totread;
    ulong	totwrite;
    ulong	nrpc;
    ulong	nproto;
    Rpc	rpc[Maxrpc];
};
/*e: struct [[Stats]] */

/*s: struct [[Fsrpc]] */
struct Fsrpc
{
    int	busy;			/* Work buffer has pending rpc to service */
    uintptr	pid;			/* Pid of slave process executing the rpc */
    int	canint;			/* Interrupt gate */
    int	flushtag;		/* Tag on which to reply to flush */
    Fcall	work;			/* Plan 9 incoming Fcall */
    uchar	buf[IOHDRSZ+Maxfdata];	/* Data buffer */
};
/*e: struct [[Fsrpc]] */

/*s: struct [[Fid]] */
struct Fid
{
    int	fid;			/* system fd for i/o */
    File	*f;			/* File attached to this fid */
    int	mode;
    int	nr;			/* fid number */
    Fid	*next;			/* hash link */
    ulong	nread;
    ulong	nwrite;
    ulong	bread;
    ulong	bwrite;
    vlong	offset;			/* for directories */
};
/*e: struct [[Fid]] */

/*s: struct [[File]] */
struct File
{
    char	*name;
    Qid	qid;
    int	inval;
    File	*parent;
    File	*child;
    File	*childlist;
};
/*e: struct [[File]] */

/*s: struct [[Proc]] */
struct Proc
{
    uintptr	pid;
    int	busy;
    Proc	*next;
};
/*e: struct [[Proc]] */

/*s: enum [[_anon_ (iostats/statfs.h)2]] */
enum
{
    Nr_workbufs 	= 40,
    Dsegpad		= 8192,
    Fidchunk	= 1000,
};
/*e: enum [[_anon_ (iostats/statfs.h)2]] */

extern Fsrpc	*Workq;
extern int  	dbg;
extern File	*root;
extern Fid	**fhash;
extern Fid	*fidfree;
extern int	qid;
extern Proc	*Proclist;
extern int	done;
extern Stats	*stats;
extern Frec	*frhead;
extern Frec	*frtail;
extern int	myiounit;

/* File system protocol service procedures */
void Xcreate(Fsrpc*), Xclunk(Fsrpc*); 
void Xversion(Fsrpc*), Xauth(Fsrpc*), Xflush(Fsrpc*); 
void Xattach(Fsrpc*), Xwalk(Fsrpc*), Xauth(Fsrpc*);
void Xremove(Fsrpc*), Xstat(Fsrpc*), Xwstat(Fsrpc*);
void slave(Fsrpc*);

void	reply(Fcall*, Fcall*, char*);
Fid 	*getfid(int);
int	freefid(int);
Fid	*newfid(int);
Fsrpc	*getsbuf(void);
void	initroot(void);
void	fatal(char*);
void	makepath(char*, File*, char*);
File	*file(File*, char*);
void	slaveopen(Fsrpc*);
void	slaveread(Fsrpc*);
void	slavewrite(Fsrpc*);
void	blockingslave(void);
void	reopen(Fid *f);
void	noteproc(int, char*);
void	flushaction(void*, char*);
void	catcher(void*, char*);
ulong	msec(void);
void	fidreport(Fid*);
/*e: iostats/statfs.h */
