/*s: assemblers/aa/compat.c */
#include "aa.h"

/*s: function myaccess */
//int
//myaccess(char *f)
//{
//    return access(f, AEXIST);
//}
/*e: function myaccess */

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

/*s: function mydup */
//int
//mydup(int f1, int f2)
//{
//    return dup(f1,f2);
//}
/*e: function mydup */

/*s: function mypipe */
//int
//mypipe(int *fd)
//{
//    return pipe(fd);
//}
/*e: function mypipe */

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

/*s: function myexec */
//int
//myexec(char *path, char *argv[])
//{
//    return exec(path, argv);
//}
/*e: function myexec */

/*s: function myfork */
int
myfork(void)
{
    return fork();
}
/*e: function myfork */
/*e: assemblers/aa/compat.c */
