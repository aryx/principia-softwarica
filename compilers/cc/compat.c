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

bool
systemtype(int sys)
{
 return sys & Plan9;
}

int
pathchar(void)
{
 return '/';
}

/*
 * claude: the "fake mallocs" below (malloc->alloc, a no-op free, and a
 * no-op setmalloctag) are DISABLED via #if 0. Original Plan 9 routed
 * malloc() through the compiler's own hunk arena (alloc()) so that library
 * code shared the arena and never had to free. But that made malloc()
 * unusable *inside* the arena: gethunk() must obtain raw memory from the OS,
 * and with the wrapper in place gethunk()->malloc()->alloc()->gethunk()
 * recurses forever (an instant stack overflow on the first source file).
 * Plan 9, Linux and macOS all provide a perfectly good libc malloc/free, so
 * we now use them directly: gethunk() refills the arena with the real
 * malloc(), and the handful of malloc()/free() call sites (dcl.c, mywait
 * above) get honest heap allocation. See compilers/cc/utils.c:gethunk().
 * (The syncweb chunk markers are kept; only the code is disabled.)
 */
#ifdef NOT_DEFINED
/*s: function [[malloc]] */
/*
 * fake mallocs
 */
void*
malloc(ulong n)
{
    return alloc(n);
}
/*e: function [[malloc]] */

/*s: function [[free]] */
void
free(void*)
{
}
/*e: function [[free]] */

/*s: function [[setmalloctag]] */
//@Scheck: looks dead, but because we redefine malloc/free we must also redefine that
void setmalloctag(void*, ulong)
{
}
/*e: function [[setmalloctag]] */
#endif
/*e: cc/compat.c */
