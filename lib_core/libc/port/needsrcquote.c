/*s: libc/port/needsrcquote.c */
#include <u.h>
#include <libc.h>

/*s: function [[needsrcquote]] */
int
needsrcquote(int c)
{
    if(c <= ' ')
        return 1;
    if(utfrune("`^#*[]=|\\?${}()'<>&;", c))
        return 1;
    return 0;
}
/*e: function [[needsrcquote]] */
/*e: libc/port/needsrcquote.c */
