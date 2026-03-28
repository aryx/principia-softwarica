/*s: compress/flate.h */
#pragma	lib	"libflate.a"
#pragma	src	"/sys/src/libflate"

/*s: enum [[flate_error]] */
/*
 * errors from deflate, deflateinit, deflateblock,
 * inflate, inflateinit, inflateblock.
 * convertable to a string by flateerr
 */
enum
{
    FlateOk			= 0,
    FlateNoMem		= -1,
    FlateInputFail		= -2,
    FlateOutputFail		= -3,
    FlateCorrupted		= -4,
    FlateInternal		= -5,
};
/*e: enum [[flate_error]] */

// compression (make smaller)
/*s: signature [[deflate]] */
int	deflate(void *wr, int (*w)(void*, void*, int), void *rr, int (*r)(void*, void*, int), int level, int debug);
/*e: signature [[deflate]] */
int	deflateinit(void);

// decompression (make bigger)
int	inflateinit(void);
/*s: signature [[inflate]] */
int	inflate(void *wr, int (*w)(void*, void*, int), void *getr, int (*get)(void*));
/*e: signature [[inflate]] */

int	inflateblock(uchar *dst, int dsize, uchar *src, int ssize);
int	deflateblock(uchar *dst, int dsize, uchar *src, int ssize, int level, int debug);

int	deflatezlib(void *wr, int (*w)(void*, void*, int), void *rr, int (*r)(void*, void*, int), int level, int debug);
int	inflatezlib(void *wr, int (*w)(void*, void*, int), void *getr, int (*get)(void*));

int	inflatezlibblock(uchar *dst, int dsize, uchar *src, int ssize);
int	deflatezlibblock(uchar *dst, int dsize, uchar *src, int ssize, int level, int debug);

char	*flateerr(int err);

ulong	*mkcrctab(ulong);
ulong	blockcrc(ulong *tab, ulong crc, void *buf, int n);

ulong	adler32(ulong adler, void *buf, int n);
/*e: compress/flate.h */
