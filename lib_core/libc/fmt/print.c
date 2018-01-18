/*s: fmt/print.c */
#include    <u.h>
#include    <libc.h>

/*s: function [[libc_print]] */
int
libc_print(char *fmt, ...)
{
    int n;
    va_list args;

    va_start(args, fmt);
    n = vfprint(1, fmt, args);
    va_end(args);
    return n;
}
/*e: function [[libc_print]] */

/*s: global [[print]] */
int (*print)(char *fmt, ...) = &libc_print;
/*e: global [[print]] */
/*e: fmt/print.c */
