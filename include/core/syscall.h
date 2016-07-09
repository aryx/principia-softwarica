// you must include libc.h instead of this file

//----------------------------------------------------------------------------
// Data structures and constants
//----------------------------------------------------------------------------

// enum Namespace_flag, mount/bind parameter
#define	MREPL	0x0000	/* mount replaces object */
#define	MBEFORE	0x0001	/* mount goes before others in union directory */
#define	MAFTER	0x0002	/* mount goes after others in union directory */
#define	MCREATE	0x0004	/* permit creation in mounted directory */
#define	MCACHE	0x0010	/* cache some data */
// bitset<Namespace_flag>
#define	MORDER	0x0003	/* mask for bits defining order of mounting */
#define	MMASK	0x0017	/* all bits on */

// enum Open_flag, open parameter
#define	OREAD	0	/* open for read */
#define	OWRITE	1	/* write */
#define	ORDWR	2	/* read and write */
#define	OEXEC	3	/* execute, == read but check execute permission */
// advanced stuff
#define	OTRUNC	16	/* or'ed in (except for exec), truncate file first */
#define	OCEXEC	32	/* or'ed in, close on exec */
#define	ORCLOSE	64	/* or'ed in, remove on close */
#define	OEXCL	0x1000	/* or'ed in, exclusive use (create only) */
// #define	OBEHIND	0x2000	/* use write behind for writes [for 9n] */

#define	AEXIST	0	/* accessible: exists */
#define	AEXEC	1	/* execute access */
#define	AWRITE	2	/* write access */
#define	AREAD	4	/* read access */

/* Segattch */
#define	SG_RONLY	0040	/* read only */
#define	SG_CEXEC	0100	/* detach on exec */

#define	NCONT	0	/* continue after note */
#define	NDFLT	1	/* terminate after note */
#define	NSAVE	2	/* clear note but hold state */
#define	NRSTR	3	/* restore saved state */

/* bits in Qid.type */
#define QTFILE		0x00		/* plain file */
#define QTDIR		0x80		/* type bit for directories */
// advanced stuff
#define QTAPPEND	0x40		/* type bit for append only files */
#define QTEXCL		0x20		/* type bit for exclusive use files */
#define QTMOUNT		0x10		/* type bit for mounted channel */
#define QTAUTH		0x08		/* type bit for authentication file */
#define QTTMP		0x04		/* type bit for not-backed-up file */

/* bits in Dir.mode */
#define DMDIR		0x80000000	/* mode bit for directories */
#define DMREAD		0x4		/* mode bit for read permission */
#define DMWRITE		0x2		/* mode bit for write permission */
#define DMEXEC		0x1		/* mode bit for execute permission */
// advanced stuff
#define DMAPPEND	0x40000000	/* mode bit for append only files */
#define DMEXCL		0x20000000	/* mode bit for exclusive use files */
#define DMMOUNT		0x10000000	/* mode bit for mounted channel */
#define DMAUTH		0x08000000	/* mode bit for authentication file */
#define DMTMP		0x04000000	/* mode bit for non-backed-up files */

/* rfork */
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
	RFNOMNT		= (1<<14)
};

#define	STATMAX	65535U	/* max length of machine-independent stat structure */
#define	DIRMAX	(sizeof(Dir)+STATMAX)	/* max length of Dir structure */
#define	ERRMAX	128	/* max length of error string */


// pad's stuff (but it is actually also in stdio.h)
enum seek_cursor {
    SEEK__START = 0,
    SEEK__CUR = 1,
    SEEK__END = 2,
};


// Qid as in uniQue id
struct Qid {
	uvlong	path;
	ulong	vers;
    // bitset<Qidtype>
	uchar	type;
};

// TODO should be renamed DirEntry really
// a similar structure is defined in the kernel!
struct Dir {
	/* system-modified data */
	ushort	type;	/* server type */
	uint	dev;	/* server subtype */

	/* file data */
	Qid	qid;	/* unique id from server */

	ulong	mode;	/* permissions */
	ulong	atime;	/* last read time */
	ulong	mtime;	/* last write time */
	vlong	length;	/* file length */
	char	*name;	/* last element of path */
	char	*uid;	/* owner name */
	char	*gid;	/* group name */
	char	*muid;	/* last modifier name */
};

/* keep /sys/src/ape/lib/ap/plan9/sys9.h in sync with this -rsc */
struct Waitmsg {
	int	pid;		/* of loved one */
	ulong	time[3];	/* of loved one & descendants */
	char	*msg;
};

// ???
struct IOchunk {
	void	*addr;
	ulong	len;
};


//----------------------------------------------------------------------------
// Functions
//----------------------------------------------------------------------------

// syscalls (or small wrappers around syscalls)

// process
extern	int		fork(void);
extern	int		rfork(int);
extern	int		exec(char*, char*[]);
extern	int		execl(char*, ...);
extern	void	_exits(char*);
extern	void	abort(void);
extern	Waitmsg*	wait(void);
extern	int		waitpid(void);
extern	int		getpid(void);
extern	int		getppid(void);

// memory
extern	void*	sbrk(ulong);

// file
extern	fdt		open(char*, int);
extern	int		close(fdt);
extern	long	read(fdt, void*, long);
extern	long	pread(fdt, void*, long, vlong);
extern	long	preadv(fdt, IOchunk*, int, vlong);
extern	long	readn(fdt, void*, long);
extern	long	readv(fdt, IOchunk*, int);
extern	long	write(fdt, void*, long);
extern	long	pwrite(fdt, void*, long, vlong);
extern	long	pwritev(fdt, IOchunk*, int, vlong);
extern	long	writev(fdt, IOchunk*, int);
extern	vlong	seek(fdt, vlong, int);
// extern	int	fdflush(int);

// directory
extern	int		create(char*, int, ulong);
extern	int		remove(char*);
extern	int		chdir(char*);
extern	int		fd2path(fdt, char*, int);
extern	int		fstat(int, uchar*, int);
extern	int		stat(char*, uchar*, int);
extern	int		fwstat(int, uchar*, int);
extern	int		wstat(char*, uchar*, int);
extern	Dir*	dirfstat(fdt);
extern	Dir*	dirstat(char*);
extern	int		dirfwstat(int, Dir*);
extern	int		dirwstat(char*, Dir*);
//
extern	long	dirread(int, Dir**);
extern	void	nulldir(Dir*);
extern	long	dirreadall(int, Dir**);
extern	int		access(char*, int); // ???

// namespace
extern	int		bind(char*, char*, int/*Mxxx*/);
extern	int		mount(fdt, int, char*, int/*Mxxx*/, char*);
extern	int		unmount(char*, char*);

// time
extern	long	alarm(ulong);
extern	int		sleep(long);

// IPC
extern	int		noted(int);
extern	int		notify(void(*)(void*, char*));

// concurrency
extern	void*	rendezvous(void*, void*);
extern	int		await(char*, int);
extern	void*	segattach(int, char*, void*, ulong);
extern	void*	segbrk(void*, void*);
extern	int		segdetach(void*);
extern	int		segflush(void*, ulong);
extern	int		segfree(void*, ulong);
extern	int		semacquire(long*, int);
extern	long	semrelease(long*, long);
extern	int		tsemacquire(long*, ulong);

// special files
extern	int		dup(int, int);
extern	int		pipe(int*);

// security
extern	int		fauth(int, char*);
extern	int		fversion(int, int, char*, int);

// error management
extern	int		errstr(char*, uint);
extern	void	werrstr(char*, ...);
#pragma	varargck	argpos	werrstr	1
extern	void	rerrstr(char*, uint);

//???
extern	char*	sysname(void);

