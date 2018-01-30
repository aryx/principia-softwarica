/*s: rc/utils.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

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


/*s: constant [[NFD]] */
#define	NFD	50
/*e: constant [[NFD]] */

/*s: struct [[DirEntryWrapper]] */
struct DirEntryWrapper {
    Dir	*dbuf;
    int	i;
    int	n;
};
/*e: struct [[DirEntryWrapper]] */
/*s: global [[dir]] */
struct DirEntryWrapper dir[NFD];
/*e: global [[dir]] */

/*s: function [[Opendir]] */
int
Opendir(char *name)
{
    Dir *db;
    int f;
    f = open(name, 0);
    if(f==-1)
        return f;
    db = dirfstat(f);
    if(db!=nil && (db->mode&DMDIR)){
        if(f<NFD){
            dir[f].i = 0;
            dir[f].n = 0;
        }
        free(db);
        return f;
    }
    free(db);
    close(f);
    return -1;
}
/*e: function [[Opendir]] */

/*s: function [[trimdirs]] */
static int
trimdirs(Dir *d, int nd)
{
    int r, w;

    for(r=w=0; r<nd; r++)
        if(d[r].mode&DMDIR)
            d[w++] = d[r];
    return w;
}
/*e: function [[trimdirs]] */

/*s: function [[Readdir]] */
/*
 * onlydirs is advisory -- it means you only
 * need to return the directories.  it's okay to
 * return files too (e.g., on unix where you can't
 * tell during the readdir), but that just makes 
 * the globber work harder.
 */
int
Readdir(int f, void *p, int onlydirs)
{
    int n;

    if(f<0 || f>=NFD)
        return 0;
Again:
    if(dir[f].i==dir[f].n){	/* read */
        free(dir[f].dbuf);
        dir[f].dbuf = 0;
        n = dirread(f, &dir[f].dbuf);
        if(n>0){
            if(onlydirs){
                n = trimdirs(dir[f].dbuf, n);
                if(n == 0)
                    goto Again;
            }	
            dir[f].n = n;
        }else
            dir[f].n = 0;
        dir[f].i = 0;
    }
    if(dir[f].i == dir[f].n)
        return 0;
    strcpy(p, dir[f].dbuf[dir[f].i].name);
    dir[f].i++;
    return 1;
}
/*e: function [[Readdir]] */

/*s: function [[Closedir]] */
void
Closedir(int f)
{
    if(f>=0 && f<NFD){
        free(dir[f].dbuf);
        dir[f].i = 0;
        dir[f].n = 0;
        dir[f].dbuf = 0;
    }
    close(f);
}
/*e: function [[Closedir]] */



/*s: function [[Memcpy]] */
void
Memcpy(void *a, void *b, long n)
{
    memmove(a, b, n);
}
/*e: function [[Memcpy]] */

/*s: function [[Malloc]] */
void*
Malloc(ulong n)
{
    return mallocz(n, 1);
}
/*e: function [[Malloc]] */


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
