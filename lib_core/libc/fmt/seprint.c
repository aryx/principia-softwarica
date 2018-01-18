/*s: libc/fmt/seprint.c */
#include <u.h>
#include <libc.h>

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
