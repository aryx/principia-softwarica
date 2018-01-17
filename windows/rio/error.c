/*s: windows/rio/error.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>

/*s: global [[errorshouldabort]] */
bool errorshouldabort = false;
/*e: global [[errorshouldabort]] */

// could be in 9p.c too
/*s: global [[Eperm]] */
char Eperm[] = "permission denied";
/*e: global [[Eperm]] */

/*s: function [[error]] */
void
error(char *s)
{
    fprint(STDERR, "rio: %s: %r\n", s);
    if(errorshouldabort)
        abort();
    threadexitsall("error");
}
/*e: function [[error]] */

/*s: function [[derror]] */
void
derror(Display*, char *errorstr)
{
    error(errorstr);
}
/*e: function [[derror]] */
/*e: windows/rio/error.c */
