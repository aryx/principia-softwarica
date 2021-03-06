/*s: libc/port/mktemp.c */
#include <u.h>
#include <libc.h>

/*s: function [[mktemp]] */
char*
mktemp(char *as)
{
    char *s;
    unsigned pid;
    int i;
    char err[ERRMAX];

    pid = getpid();
    s = as;
    while(*s++)
        ;
    s--;
    while(*--s == 'X') {
        *s = pid % 10 + '0';
        pid = pid/10;
    }
    s++;
    i = 'a';
    while(access(as, 0) != -1) {
        if (i == 'z')
            return "/";
        *s = i++;
    }
    err[0] = '\0';
    errstr(err, sizeof err);    /* clear the error */
    return as;
}
/*e: function [[mktemp]] */
/*e: libc/port/mktemp.c */
