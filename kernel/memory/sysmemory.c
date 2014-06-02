/*s: sysmemory.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

// see also sysbrk in sysproc.c

// those functions used to be in segment.c

/*s: function pteflush */
void
pteflush(Pagetable *pte, int s, int e)
{
    int i;
    Page *p;

    for(i = s; i < e; i++) {
        p = pte->pages[i];
        if(pagedout(p) == 0)
            memset(p->cachectl, PG_TXTFLUSH, sizeof(p->cachectl));
    }
}
/*e: function pteflush */

/*s: syscall segflush */
long
syssegflush(ulong *arg)
{
    Segment *s;
    ulong addr, l;
    Pagetable *pte;
    int chunk, ps, pe, len;

    addr = arg[0];
    len = arg[1];

    while(len > 0) {
        s = seg(up, addr, 1);
        if(s == 0)
            error(Ebadarg);

        s->flushme = 1;
    more:
        l = len;
        if(addr+l > s->top)
            l = s->top - addr;

        ps = addr-s->base;
        pte = s->pagedir[ps/PTEMAPMEM];
        ps &= PTEMAPMEM-1;
        pe = PTEMAPMEM;
        if(pe-ps > l){
            pe = ps + l;
            pe = (pe+BY2PG-1)&~(BY2PG-1);
        }
        if(pe == ps) {
            qunlock(&s->lk);
            error(Ebadarg);
        }

        if(pte)
            pteflush(pte, ps/BY2PG, pe/BY2PG);

        chunk = pe-ps;
        len -= chunk;
        addr += chunk;

        if(len > 0 && addr < s->top)
            goto more;

        qunlock(&s->lk);
    }
    flushmmu();
    return 0;
}
/*e: syscall segflush */
/*e: sysmemory.c */
