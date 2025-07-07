/*s: rc/utils.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

// Opendir, Closedir and so on are back in plan9.c

/*s: function [[Unlink]] */
void
Unlink(char *name)
{
    remove(name);
}
/*e: function [[Unlink]] */

/*s: function [[Write]] */
long
Write(int fd, void *buf, long cnt)
{
    return write(fd, buf, cnt);
}
/*e: function [[Write]] */

/*s: function [[Read]] */
long
Read(int fd, void *buf, long cnt)
{
    return read(fd, buf, cnt);
}
/*e: function [[Read]] */

/*s: function [[Seek]] */
long
Seek(int fd, long cnt, long whence)
{
    return seek(fd, cnt, whence);
}
/*e: function [[Seek]] */

/*s: function [[Creat]] */
int
Creat(char *file)
{
    return create(file, 1, 0666L);
}
/*e: function [[Creat]] */

/*s: function [[Dup]] */
int
Dup(int a, int b)
{
    return dup(a, b);
}
/*e: function [[Dup]] */


/*s: function [[Memcpy]] */
void
Memcpy(void *a, void *b, long n)
{
    memmove(a, b, n);
}
/*e: function [[Memcpy]] */

// back in plan9.c
extern void* Malloc(ulong n);
/*s: function [[emalloc]] */
void *
emalloc(long n)
{
    void *p = Malloc(n);

    if(p==nil)
        panic("Can't malloc %d bytes", n);
    return p;
}
/*e: function [[emalloc]] */

/*s: function [[efree]] */
void
efree(void *p)
{
    if(p)
        free(p);
    else pfmt(err, "free 0\n");
}
/*e: function [[efree]] */


/*s: global [[bp]] */
char *bp;
/*e: global [[bp]] */

/*s: function [[iacvt]] */
static void
iacvt(int n)
{
    if(n<0){
        *bp++='-';
        n=-n;	/* doesn't work for n==-inf */
    }
    if(n/10)
        iacvt(n/10);
    *bp++=n%10+'0';
}
/*e: function [[iacvt]] */

/*s: function [[inttoascii]] */
void
inttoascii(char *s, long n)
{
    bp = s;
    iacvt(n);
    *bp='\0';
}
/*e: function [[inttoascii]] */

/*e: rc/utils.c */
