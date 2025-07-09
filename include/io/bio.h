#pragma	src	"/sys/src/libbio"
#pragma	lib	"libbio.a"

typedef	struct	Biobuf	Biobuf;
typedef	struct	Biobufhdr	Biobufhdr;

//typedef	struct	Biobuf	BiobufGen; // for 5c in ocaml
typedef	struct	Biobufhdr	BiobufGen; // for regular 5c

enum
{
	Bsize		= 8*1024,
	Bungetsize	= UTFmax+1,		/* space for ungetc */
	Bmagic		= 0x314159,
	Beof		= -1,
	Bbad		= -2,

	Binactive	= 0,		/* states */
	Bractive,
	Bwactive,
	Bracteof,
};

struct	Biobufhdr
{
	int	icount;		/* neg num of bytes at eob */
	int	ocount;		/* num of bytes at bob */
	int	rdline;		/* num of bytes after rdline */
	int	runesize;	/* num of bytes of last getrune */
	int	state;		/* r/w/inactive */
	int	fid;		/* open file */
	int	flag;		/* magic if malloc'ed */
	vlong	offset;		/* offset of buffer in file */
	int	bsize;		/* size of buffer */
	uchar*	bbuf;		/* pointer to beginning of buffer */
	uchar*	ebuf;		/* pointer to end of buffer */
	uchar*	gbuf;		/* pointer to good data in buf */
};

struct	Biobuf
{
	Biobufhdr;
	uchar	b[Bungetsize+Bsize];
};

/* Dregs, redefined as functions for backwards compatibility */
#define	BGETC(bp)	Bgetc(bp)
#define	BPUTC(bp,c)	Bputc(bp,c)
#define	BOFFSET(bp)	Boffset(bp)
#define	BLINELEN(bp)	Blinelen(bp)
#define	BFILDES(bp)	Bfildes(bp)

Biobuf*	Bopen(char*, int);

int	Binit(Biobuf*, fdt, int);
int	Binits(Biobufhdr*, fdt, int, uchar*, int);

int	Bflush(BiobufGen*);

long	Bread(Biobufhdr*, void*, long);
long	Bwrite(BiobufGen*, void*, long);
vlong	Bseek(Biobufhdr*, vlong, int);

int	Bputc(BiobufGen*, int);
int	Bgetc(BiobufGen*);
int	Bungetc(Biobufhdr*);

int	Bputrune(Biobufhdr*, long);
long	Bgetrune(BiobufGen*);
int	Bungetrune(BiobufGen*);

int	Bprint(BiobufGen*, char*, ...);
int	Bvprint(Biobufhdr*, char*, va_list);



int	Bbuffered(Biobufhdr*);
int	Bfildes(Biobufhdr*);

int	Bgetd(Biobufhdr*, double*);
int	Blinelen(Biobufhdr*);
vlong	Boffset(Biobufhdr*);
void*	Brdline(Biobufhdr*, int);
char*	Brdstr(Biobufhdr*, int, int);
int	Bterm(Biobufhdr*);

#pragma	varargck	argpos	Bprint	2
