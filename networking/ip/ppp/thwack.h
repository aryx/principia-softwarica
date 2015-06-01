/*s: networking/ip/ppp/thwack.h */
typedef struct Thwack		Thwack;
typedef struct Unthwack		Unthwack;
typedef struct ThwBlock		ThwBlock;
typedef struct UnthwBlock	UnthwBlock;

/*s: enum _anon_ (networking/ip/ppp/thwack.h) */
enum
{
    ThwStats	= 8,
    ThwErrLen	= 64,		/* max length of error message from thwack or unthwack */
    ThwMaxBlock	= 1600,		/* max size of compressible block */

    HashLog		= 12,
    HashSize	= 1<<HashLog,
    HashMask	= HashSize - 1,

    MinMatch	= 3,		/* shortest match possible */

    MaxOff		= 8,
    OffBase		= 6,

    MinDecode	= 8,		/* minimum bits to decode a match or lit; >= 8 */

    CompBlocks	= 10,		/* max blocks used to encode data */
    EWinBlocks	= 64,		/* blocks held in encoder window */
    DWinBlocks	= EWinBlocks,	/* blocks held in decoder window */

    MaxSeqMask	= 8,		/* number of bits in coding block mask */
    MaxSeqStart	= 256		/* max offset of initial coding block */
};
/*e: enum _anon_ (networking/ip/ppp/thwack.h) */

/*s: struct ThwBlock */
struct ThwBlock
{
    ulong	seq;			/* sequence number for this data */
    uchar	acked;			/* ok to use this block; the decoder has it */
    ushort	begin;			/* time of first byte in hash */
    uchar	*edata;			/* last byte of valid data */
    ushort	maxoff;			/* time of last valid hash entry */
    ushort	*hash;
    uchar	*data;
};
/*e: struct ThwBlock */

/*s: struct Thwack */
struct Thwack
{
    QLock		acklock;	/* locks slot, blocks[].(acked|seq) */
    int		slot;		/* next block to use */
    ThwBlock	blocks[EWinBlocks];
    ushort		hash[EWinBlocks][HashSize];
    Block		*data[EWinBlocks];
};
/*e: struct Thwack */

/*s: struct UnthwBlock */
struct UnthwBlock
{
    ulong	seq;			/* sequence number for this data */
    ushort	maxoff;			/* valid data in each block */
    uchar	*data;
};
/*e: struct UnthwBlock */

/*s: struct Unthwack */
struct Unthwack
{
    int		slot;		/* next block to use */
    char		err[ThwErrLen];
    UnthwBlock	blocks[DWinBlocks];
    uchar		data[DWinBlocks][ThwMaxBlock];
};
/*e: struct Unthwack */

void	thwackinit(Thwack*);
void	thwackcleanup(Thwack *tw);
void	unthwackinit(Unthwack*);
int	thwack(Thwack*, int mustadd, uchar *dst, int ndst, Block *bsrc, ulong seq, ulong stats[ThwStats]);
void	thwackack(Thwack*, ulong seq, ulong mask);
int	unthwack(Unthwack*, uchar *dst, int ndst, uchar *src, int nsrc, ulong seq);
ulong	unthwackstate(Unthwack *ut, uchar *mask);
int	unthwackadd(Unthwack *ut, uchar *src, int nsrc, ulong seq);
/*e: networking/ip/ppp/thwack.h */
