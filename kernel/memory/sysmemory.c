/*s: sysmemory.c */
/*s: kernel basic includes */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
/*e: kernel basic includes */

// see also sysbrk_ in sysproc.c

// those functions used to be in segment.c

void
pteflush(Pte *pte, int s, int e)
{
    int i;
    Page *p;

    for(i = s; i < e; i++) {
        p = pte->pages[i];
        if(pagedout(p) == 0)
            memset(p->cachectl, PG_TXTFLUSH, sizeof(p->cachectl));
    }
}

long
syssegflush(ulong *arg)
{
    Segment *s;
    ulong addr, l;
    Pte *pte;
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
        pte = s->map[ps/PTEMAPMEM];
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
/*e: sysmemory.c */
