/*s: alloc.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

#include    <pool.h>

/* everything from here down should be the same in libc, libdebugmalloc, and the kernel */
/* - except the code for malloc(), which alternately doesn't clear or does. */
/* - except the code for smalloc(), which lives only in the kernel. */

//*****************************************************************************
// Debugging support
//*****************************************************************************

/*
 * Npadlong is the number of 32-bit longs to leave at the beginning of 
 * each allocated buffer for our own bookkeeping.  We return to the callers
 * a pointer that points immediately after our bookkeeping area.  Incoming pointers
 * must be decremented by that much, and outgoing pointers incremented.
 * The malloc tag is stored at MallocOffset from the beginning of the block,
 * and the realloc tag at ReallocOffset.  The offsets are from the true beginning
 * of the block, not the beginning the caller sees.
 *
 * The extra if(Npadlong != 0) in various places is a hint for the compiler to
 * compile out function calls that would otherwise be no-ops.
 */

/* tracing */
enum {
    Npadlong    = 2,
    MallocOffset = 0,
    ReallocOffset = 1
};

/*s: alloc.c debugging macro */
/* non tracing
 *
enum {
    Npadlong    = 0,
    MallocOffset = 0,
    ReallocOffset = 0,
};
 *
 */
/*e: alloc.c debugging macro */

/*s: function setmalloctag */
void
setmalloctag(kern_addr3 v, kern_addr pc)
{
    kern_addr2 u;
    USED(v, pc); //??
    if(Npadlong <= MallocOffset || v == nil)
        return;
    u = v;
    u[-Npadlong+MallocOffset] = pc;
}
/*e: function setmalloctag */

/*s: function setrealloctag */
void
setrealloctag(kern_addr3 v, kern_addr pc)
{
    kern_addr2 u;
    USED(v, pc);
    if(Npadlong <= ReallocOffset || v == nil)
        return;
    u = v;
    u[-Npadlong+ReallocOffset] = pc;
}
/*e: function setrealloctag */

//*****************************************************************************
// Pool wrappers
//*****************************************************************************

/*s: function smalloc */
// non failing malloc! will repeat until it can
kern_addr3
smalloc(ulong size)
{
    kern_addr3 v;

    for(;;) {
        v = poolalloc(mainmem, size + Npadlong*sizeof(ulong));
        if(v != nil)
            break;
        tsleep(&up->sleepr, returnfalse, 0, 100);
    }
    if(Npadlong){
        v = (ulong*)v+Npadlong;
        setmalloctag(v, getcallerpc(&size));
    }
    memset(v, 0, size); // clear
    return v;
}
/*e: function smalloc */

/*s: function malloc */
kern_addr3
malloc(ulong size)
{
    kern_addr3 v;

    v = poolalloc(mainmem, size+Npadlong*sizeof(ulong));
    if(v == nil)
        return nil;
    if(Npadlong){
        v = (ulong*)v+Npadlong;
        setmalloctag(v, getcallerpc(&size));
        setrealloctag(v, 0);
    }
    memset(v, 0, size);
    return v;
}
/*e: function malloc */

/*s: function mallocz */
kern_addr3
mallocz(ulong size, bool clr)
{
    kern_addr3 v;

    v = poolalloc(mainmem, size+Npadlong*sizeof(ulong));
    if(Npadlong && v != nil){
        v = (ulong*)v+Npadlong;
        setmalloctag(v, getcallerpc(&size));
        setrealloctag(v, 0);
    }
    if(clr && v != nil)
        memset(v, 0, size);
    return v;
}
/*e: function mallocz */

/*s: function mallocalign */
kern_addr3
mallocalign(ulong size, ulong align, long offset, ulong span)
{
    kern_addr3 v;

    v = poolallocalign(mainmem, size+Npadlong*sizeof(ulong), align, 
                           offset-Npadlong*sizeof(ulong), span);
    if(Npadlong && v != nil){
        v = (ulong*)v+Npadlong;
        setmalloctag(v, getcallerpc(&size));
        setrealloctag(v, 0);
    }
    if(v)
        memset(v, 0, size);
    return v;
}
/*e: function mallocalign */

/*s: function free */
void
free(kern_addr3 v)
{
    if(v != nil)
        poolfree(mainmem, (ulong*)v-Npadlong);
}
/*e: function free */

/*s: function realloc */
kern_addr3
realloc(kern_addr3 v, ulong size)
{
    kern_addr3 nv;

    if(v != nil)
        v = (ulong*)v-Npadlong;
    if(Npadlong !=0 && size != 0)
        size += Npadlong*sizeof(ulong);

    if(nv = poolrealloc(mainmem, v, size)){
        nv = (ulong*)nv+Npadlong;
        setrealloctag(nv, getcallerpc(&v));
        if(v == nil)
            setmalloctag(nv, getcallerpc(&v));
    }       
    return nv;
}
/*e: function realloc */

/*s: function msize */
ulong
msize(void *v)
{
    return poolmsize(mainmem, (ulong*)v-Npadlong)-Npadlong*sizeof(ulong);
}
/*e: function msize */


//*****************************************************************************
// kstr functions
//*****************************************************************************

//pad: was in chan.c

/*s: function kstrcpy */
/*
 * Rather than strncpy, which zeros the rest of the buffer, kstrcpy
 * truncates if necessary, always zero terminates, does not zero fill,
 * and puts ... at the end of the string if it's too long.  Usually used to
 * save a string in up->genbuf;
 */
void
kstrcpy(char *s, char *t, int ns)
{
    int nt;

    nt = strlen(t);
    if(nt+1 <= ns){
        memmove(s, t, nt+1);
        return;
    }
    /* too long */
    if(ns < 4){
        /* but very short! */
        strncpy(s, t, ns);
        return;
    }
    /* truncate with ... at character boundary (very rare case) */
    memmove(s, t, ns-4);
    ns -= 4;
    s[ns] = '\0';
    /* look for first byte of UTF-8 sequence by skipping continuation bytes */
    while(ns>0 && (s[--ns]&0xC0)==0x80)
        ;
    strcpy(s+ns, "...");
}
/*e: function kstrcpy */

/*s: function kstrdup */
/*
 * Atomically replace *p with copy of s
 */
void
kstrdup(char **p, char *s)
{
    int n;
    char *t, *prev;

    n = strlen(s)+1;
    /* if it's a user, we can wait for memory; if not, something's very wrong */
    if(up){
        t = smalloc(n);
        setmalloctag(t, getcallerpc(&p));
    }else{
        t = malloc(n);
        if(t == nil)
            panic("kstrdup: no memory");
    }
    memmove(t, s, n);
    prev = *p;
    *p = t;
    free(prev);
}
/*e: function kstrdup */
/*e: alloc.c */
