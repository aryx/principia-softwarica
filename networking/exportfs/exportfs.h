/*s: networking/exportfs/exportfs.h */
/*s: constant [[DEBUG]]([[(networking/exportfs/exportfs.h)]]) */
/*
 * exportfs.h - definitions for exporting file server
 */

#define DEBUG		if(dbg)fprint
/*e: constant [[DEBUG]]([[(networking/exportfs/exportfs.h)]]) */
/*s: constant [[DFD]] */
#define DFD		9
/*e: constant [[DFD]] */
/*s: macro [[fidhash]] */
#define fidhash(s)	fhash[s%FHASHSIZE]
/*e: macro [[fidhash]] */

typedef struct Fsrpc Fsrpc;
typedef struct Fid Fid;
typedef struct File File;
typedef struct Proc Proc;
typedef struct Qidtab Qidtab;

/*s: struct [[Fsrpc]] */
struct Fsrpc
{
    int	busy;		/* Work buffer has pending rpc to service */
    uintptr	pid;		/* Pid of slave process executing the rpc */
    int	canint;		/* Interrupt gate */
    int	flushtag;	/* Tag on which to reply to flush */
    Fcall	work;		/* Plan 9 incoming Fcall */
    uchar	*buf;		/* Data buffer */
};
/*e: struct [[Fsrpc]] */

/*s: struct [[Fid]]([[(networking/exportfs/exportfs.h)]]) */
struct Fid
{
    int	fid;		/* system fd for i/o */
    File	*f;		/* File attached to this fid */
    int	mode;
    int	nr;		/* fid number */
    int	mid;		/* Mount id */
    Fid	*next;		/* hash link */

    /* for preaddir -- ARRGH! */
    Dir	*dir;		/* buffer for reading directories */
    int	ndir;		/* number of entries in dir */
    int	cdir;		/* number of consumed entries in dir */
    int	gdir;		/* glue index */
    vlong	offset;		/* offset in virtual directory */
};
/*e: struct [[Fid]]([[(networking/exportfs/exportfs.h)]]) */

/*s: struct [[File]] */
struct File
{
    char	*name;
    int	ref;
    Qid	qid;
    Qidtab	*qidt;
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

/*s: struct [[Qidtab]] */
struct Qidtab
{
    int	ref;
    int	type;
    int	dev;
    vlong	path;
    vlong	uniqpath;
    Qidtab	*next;
};
/*e: struct [[Qidtab]] */

/*s: enum [[_anon_ (networking/exportfs/exportfs.h)]] */
enum
{
    MAXPROC		= 50,
    FHASHSIZE	= 64,
    Nr_workbufs 	= 50,
    Fidchunk	= 1000,
    Npsmpt		= 32,
    Nqidbits		= 5,
    Nqidtab		= (1<<Nqidbits),
};
/*e: enum [[_anon_ (networking/exportfs/exportfs.h)]] */

/*s: global [[Ebadfid]] */
char Ebadfid[];
/*e: global [[Ebadfid]] */
/*s: global [[Enotdir]]([[(networking/exportfs/exportfs.h)]]) */
char Enotdir[];
/*e: global [[Enotdir]]([[(networking/exportfs/exportfs.h)]]) */
/*s: global [[Edupfid]] */
char Edupfid[];
/*e: global [[Edupfid]] */
/*s: global [[Eopen]] */
char Eopen[];
/*e: global [[Eopen]] */
/*s: global [[Exmnt]] */
char Exmnt[];
/*e: global [[Exmnt]] */
/*s: global [[Enomem]] */
char Enomem[];
/*e: global [[Enomem]] */
/*s: global [[Emip]] */
char Emip[];
/*e: global [[Emip]] */
/*s: global [[Enopsmt]] */
char Enopsmt[];
/*e: global [[Enopsmt]] */

extern Fsrpc	*Workq;
extern int  	dbg;
extern File	*root;
extern File	*psmpt;
extern Fid	**fhash;
extern Fid	*fidfree;
extern Proc	*Proclist;
extern char	psmap[Npsmpt];
extern Qidtab	*qidtab[Nqidtab];
extern ulong	messagesize;
extern char	Enomem[];
extern int	srvfd;
extern char*	patternfile;

/* File system protocol service procedures */
void Xattach(Fsrpc*);
void Xauth(Fsrpc*);
void Xclunk(Fsrpc*); 
void Xcreate(Fsrpc*);
void Xflush(Fsrpc*); 
void Xnop(Fsrpc*);
void Xremove(Fsrpc*);
void Xstat(Fsrpc*);
void Xversion(Fsrpc*);
void Xwalk(Fsrpc*);
void Xwstat(Fsrpc*);
void slave(Fsrpc*);

void	reply(Fcall*, Fcall*, char*);
Fid 	*getfid(int);
int	freefid(int);
Fid	*newfid(int);
Fsrpc	*getsbuf(void);
void	initroot(void);
void	fatal(char*, ...);
char*	makepath(File*, char*);
File	*file(File*, char*);
void	freefile(File*);
void	slaveopen(Fsrpc*);
void	slaveread(Fsrpc*);
void	slavewrite(Fsrpc*);
void	blockingslave(void);
void	reopen(Fid *f);
void	noteproc(int, char*);
void	flushaction(void*, char*);
void	pushfcall(char*);
Qidtab* uniqueqid(Dir*);
void	freeqid(Qidtab*);
char*	estrdup(char*);
void*	emallocz(uint);
int	readmessage(int, char*, int);
void	exclusions(void);
int	excludefile(char*);
int	preaddir(Fid*, uchar*, int, vlong);
/*e: networking/exportfs/exportfs.h */
