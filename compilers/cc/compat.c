/*s: cc/compat.c */
#include	"cc.h"

//#include	"compat"
//TODO copy paste with aa/, maybe could factorize with another lib?
//

int
myaccess(char *f)
{
 return access(f, AEXIST);
}


int
mycreat(char *n, int p)
{

 return create(n, 1, p);
}

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

int
mydup(int f1, int f2)
{
 return dup(f1,f2);
}

int
mypipe(int *fd)
{
 return pipe(fd);
}

int
systemtype(int sys)
{
 return sys & Plan9;
}

int
pathchar(void)
{
 return '/';
}

char*
mygetwd(char *path, int len)
{
 return getwd(path, len);
}

int
myexec(char *path, char *argv[])
{
 return exec(path, argv);
}

int
myfork(void)
{
 return fork();
}



/*s: function malloc */
/*
 * fake mallocs
 */
void*
malloc(ulong n)
{
    return alloc(n);
}
/*e: function malloc */

/*s: function free */
void
free(void*)
{
}
/*e: function free */

/*s: function setmalloctag */
//@Scheck: looks dead, but because we redefine malloc/free we must also redefine that
void setmalloctag(void*, ulong)
{
}
/*e: function setmalloctag */
/*e: cc/compat.c */
