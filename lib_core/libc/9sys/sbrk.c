/*s: 9sys/sbrk.c */
#include <u.h>
#include <libc.h>

extern  char    end[];

/*s: global [[bloc]] */
// starting point for the heap, after the text, data, and bss.
static  char    *bloc = { end };
/*e: global [[bloc]] */

// the syscall
extern  int brk(void*);

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
/*e: 9sys/sbrk.c */
