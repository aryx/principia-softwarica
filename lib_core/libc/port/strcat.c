/*s: libc/port/strcat.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[strcat]] */
char*
strcat(char *s1, char *s2)
{

    strcpy(strchr(s1, '\0'), s2);
    return s1;
}
/*e: function [[strcat]] */
/*e: libc/port/strcat.c */
