/*s: assemblers/aa/compat.c */
#include "aa.h"


/*s: function mysbrk */
void*
mysbrk(ulong size)
{
    return sbrk(size);
}
/*e: function mysbrk */

/*s: function mycreat */
int
mycreat(char *n, int p)
{

    return create(n, 1, p);
}
/*e: function mycreat */

/*s: function mywait */
int
mywait(int *s)
{
    int p;
    Waitmsg *w;

    if((w = wait()) == nil)
        return -1;
    else{
        p = w->pid;
        *s = 0;
        if(w->msg[0])
            *s = 1;
        free(w);
        return p;
    }
}
/*e: function mywait */

/*s: function systemtype */
int
systemtype(int sys)
{
    return sys & Plan9;
}
/*e: function systemtype */

/*s: function pathchar */
int
pathchar(void)
{
    return '/';
}
/*e: function pathchar */

/*s: function mygetwd */
char*
mygetwd(char *path, int len)
{
    return getwd(path, len);
}
/*e: function mygetwd */

/*s: function myfork */
int
myfork(void)
{
    return fork();
}
/*e: function myfork */
/*e: assemblers/aa/compat.c */
