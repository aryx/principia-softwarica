/*s: libregexp/regerror.c */
/*s: libregexp includes */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
#include "regexp.h"
/*e: libregexp includes */
/*s: function [[regerror]] */
void
regerror(char *s)
{
    char buf[132];

    strcpy(buf, "regerror: ");
    strcat(buf, s);
    strcat(buf, "\n");
    write(STDERR, buf, strlen(buf));
    exits("regerr");
}
/*e: function [[regerror]] */
/*e: libregexp/regerror.c */
