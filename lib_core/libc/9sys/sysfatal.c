/*s: 9sys/sysfatal.c */
#include <u.h>
#include <libc.h>


/*s: function [[_sysfatalimpl]] */
static void
_sysfatalimpl(char *fmt, va_list arg)
{
    char buf[1024];

    vseprint(buf, buf+sizeof(buf), fmt, arg);
    if(argv0)
        fprint(2, "%s: %s\n", argv0, buf);
    else
        fprint(2, "%s\n", buf);
    exits(buf);
}
/*e: function [[_sysfatalimpl]] */

/*s: global [[_sysfatal]] */
void (*_sysfatal)(char *fmt, va_list arg) = _sysfatalimpl;
/*e: global [[_sysfatal]] */

/*s: function [[sysfatal]] */
void
sysfatal(char *fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);
    (*_sysfatal)(fmt, arg);
    va_end(arg);
}
/*e: function [[sysfatal]] */
/*e: 9sys/sysfatal.c */
