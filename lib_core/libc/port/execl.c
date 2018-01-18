/*s: libc/port/execl.c */
#include <u.h>
#include <libc.h>

/*s: function [[execl]] */
int
execl(char *f, ...)
{

    return exec(f, &f+1);
}
/*e: function [[execl]] */
/*e: libc/port/execl.c */
