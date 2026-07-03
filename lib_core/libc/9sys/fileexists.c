/*s: libc/9sys/fileexists.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[fileexists]] */
// new: used to be duplicated in linkers/ (5l and 8l)
bool
fileexists(char *s)
{
    byte dirbuf[400];

    /* it's fine if stat result doesn't fit in dirbuf, since even then the file exists */
    return stat(s, dirbuf, sizeof(dirbuf)) >= 0;
}
/*e: function [[fileexists]] */
/*e: libc/9sys/fileexists.c */
