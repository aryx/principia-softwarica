/*s: libc/9sys/werrstr.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[werrstr]] */
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
/*e: function [[werrstr]] */
/*e: libc/9sys/werrstr.c */
