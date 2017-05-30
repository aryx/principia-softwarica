/*s: 9sys/werrstr.c */
#include <u.h>
#include <libc.h>

/*s: function werrstr */
void
werrstr(char *fmt, ...)
{
    va_list arg;
    char buf[ERRMAX];

    va_start(arg, fmt);
    vseprint(buf, buf+ERRMAX, fmt, arg);
    va_end(arg);
    errstr(buf, sizeof buf);
}
/*e: function werrstr */
/*e: 9sys/werrstr.c */
