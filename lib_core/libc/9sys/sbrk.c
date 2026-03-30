/*s: libc/9sys/sbrk.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */

/*s: global extern declaration [[end]] */
extern  char    end[];
/*e: global extern declaration [[end]] */
/*s: global [[bloc]] */
// starting point for the heap, after the text, data, and bss.
static  char    *bloc = { end };
/*e: global [[bloc]] */

/*s: enum [[_anon_ (9sys/sbrk.c)]] */
enum
{
    Round   = 7
};
/*e: enum [[_anon_ (9sys/sbrk.c)]] */

/*s: function [[sbrk]] */
void*
sbrk(ulong n)
{
    uintptr bl;

    bl = ((uintptr)bloc + Round) & ~Round;
    if(brk((void*)(bl+n)) < 0)
        return (void*)-1;
    bloc = (char*)bl + n;
    return (void*)bl;
}
/*e: function [[sbrk]] */
/*e: libc/9sys/sbrk.c */
