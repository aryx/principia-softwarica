/*s: include/io/bio.h */
#pragma	src	"/sys/src/libbio"
#pragma	lib	"libbio.a"

typedef	struct	Biobuf	Biobuf;
typedef	struct	Biobufhdr	Biobufhdr;

//typedef	struct	Biobuf	BiobufGen; // for 5c in ocaml
typedef	struct	Biobufhdr	BiobufGen; // for regular 5c

enum
{
    /*s: constants Bxxx */
    Bsize		= 8*1024,
    Bungetsize	= UTFmax+1,		/* space for ungetc */
    /*x: constants Bxxx */
    Binactive	= 0,		/* states */
    Bractive,
    Bwactive,
    Bracteof,
    /*x: constants Bxxx */
    Bmagic		= 0x314159,
    /*x: constants Bxxx */
    Beof		= -1,
    Bbad		= -2,
    /*e: constants Bxxx */
};

/*s: struct [[Biobufhdr]] */
struct	Biobufhdr
{
    fdt	fid;		/* open file */
    // enum<Bkind>
    int	state;		/* r/w/inactive */

    vlong	offset;		/* offset of buffer in file */
    int	bsize;		/* size of buffer */

    /*s: [[Biobufhdr]] count fields */
    int	icount;		/* neg num of bytes at eob */
    int	ocount;		/* num of bytes at bob */
    /*e: [[Biobufhdr]] count fields */
    /*s: [[Biobufhdr]] buffer fields */
    // point in Biobuf.b
    uchar*	bbuf;		/* pointer to beginning of buffer */
    uchar*	ebuf;		/* pointer to end of buffer */

    uchar*	gbuf;		/* pointer to good data in buf */
    /*e: [[Biobufhdr]] buffer fields */
    /*s: [[Biobufhdr]] other fields */
    int	rdline;		/* num of bytes after rdline */
    int	runesize;	/* num of bytes of last getrune */
    /*x: [[Biobufhdr]] other fields */
    int	flag;		/* magic if malloc'ed */
    /*e: [[Biobufhdr]] other fields */
};
/*e: struct [[Biobufhdr]] */
/*s: struct [[Biobuf]] */
struct	Biobuf
{
    Biobufhdr;
    uchar	b[Bungetsize+Bsize];
};
/*e: struct [[Biobuf]] */


/* Dregs, redefined as functions for backwards compatibility */
/*s: macro [[BGETC]] */
#define	BGETC(bp)	Bgetc(bp)
/*e: macro [[BGETC]] */
/*s: macro [[BPUTC]] */
#define	BPUTC(bp,c)	Bputc(bp,c)
/*e: macro [[BPUTC]] */
/*s: macro [[BOFFSET]] */
#define	BOFFSET(bp)	Boffset(bp)
/*e: macro [[BOFFSET]] */
/*s: macro [[BLINELEN]] */
#define	BLINELEN(bp)	Blinelen(bp)
/*e: macro [[BLINELEN]] */
/*s: macro [[BFILDES]] */
#define	BFILDES(bp)	Bfildes(bp)
/*e: macro [[BFILDES]] */

/*s: signatures [[Bxxx]] functions */
int	Binit(Biobuf*, fdt, int);
Biobuf*	Bopen(char*, int);
int	Bterm(Biobufhdr*);

int	Bgetc(BiobufGen*);
int	Bungetc(Biobufhdr*);
long	Bread(Biobufhdr*, void*, long);

int	Bputc(BiobufGen*, int);
long	Bwrite(BiobufGen*, void*, long);
int	Bprint(BiobufGen*, char*, ...);

#pragma	varargck	argpos	Bprint	2

void*	Brdline(Biobufhdr*, int);
int	Blinelen(Biobufhdr*);
/*e: signatures [[Bxxx]] functions */

int	Binits(Biobufhdr*, fdt, int, uchar*, int);

int	Bflush(BiobufGen*);

vlong	Bseek(Biobufhdr*, vlong, int);

int	Bputrune(Biobufhdr*, long);
long	Bgetrune(BiobufGen*);
int	Bungetrune(BiobufGen*);

int	Bvprint(Biobufhdr*, char*, va_list);

// Helpers
int	Bbuffered(Biobufhdr*);
int	Bfildes(Biobufhdr*);

int	Bgetd(Biobufhdr*, double*);
vlong	Boffset(Biobufhdr*);
char*	Brdstr(Biobufhdr*, int, int);

/*e: include/io/bio.h */
