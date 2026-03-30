/*s: libc/fmt/seprint.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[seprint]] */
char*
seprint(char *buf, char *e, char *fmt, ...)
{
    char *p;
    va_list args;

    va_start(args, fmt);
    p = vseprint(buf, e, fmt, args);
    va_end(args);
    return p;
}
/*e: function [[seprint]] */
/*e: libc/fmt/seprint.c */
