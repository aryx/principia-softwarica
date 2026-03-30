/*s: libc/fmt/print.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[libc_print]] */
int
libc_print(char *fmt, ...)
{
    int n;
    va_list args;

    va_start(args, fmt);
    n = vfprint(STDOUT, fmt, args);
    va_end(args);
    return n;
}
/*e: function [[libc_print]] */

/*s: global [[print]] */
int (*print)(char *fmt, ...) = &libc_print;
/*e: global [[print]] */
/*e: libc/fmt/print.c */
