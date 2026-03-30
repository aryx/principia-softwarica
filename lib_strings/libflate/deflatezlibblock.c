/*s: libflate/deflatezlibblock.c */
/*s: libflate includes */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
#include <flate.h>
/*e: libflate includes */
#include "zlib.h"

/*s: function [[deflatezlibblock]] */
int
deflatezlibblock(uchar *dst, int dsize, uchar *src, int ssize, int level, int debug)
{
    ulong adler;
    int n;

    if(dsize < 6)
        return FlateOutputFail;

    n = deflateblock(dst + 2, dsize - 6, src, ssize, level, debug);
    if(n < 0)
        return n;

    dst[0] = ZlibDeflate | ZlibWin32k;

    /* bogus zlib encoding of compression level */
    dst[1] = ((level > 2) + (level > 5) + (level > 8)) << 6;

    /* header check field */
    dst[1] |= 31 - ((dst[0] << 8) | dst[1]) % 31;

    adler = adler32(1, src, ssize);
    dst[n + 2] = adler >> 24;
    dst[n + 3] = adler >> 16;
    dst[n + 4] = adler >> 8;
    dst[n + 5] = adler;

    return n + 6;
}
/*e: function [[deflatezlibblock]] */
/*e: libflate/deflatezlibblock.c */
