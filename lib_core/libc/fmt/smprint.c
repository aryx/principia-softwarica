/*s: libc/fmt/smprint.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[smprint]] */
char*
smprint(char *fmt, ...)
{
    va_list args;
    char *p;

    va_start(args, fmt);
    p = vsmprint(fmt, args);
    va_end(args);
    setmalloctag(p, getcallerpc(&fmt));
    return p;
}
/*e: function [[smprint]] */
/*e: libc/fmt/smprint.c */
