/*s: xalloc.c */
// Memory allocator for long lived allocated structures
// e.g. array or Proc (procalloc), array of Pages (palloc.pages), etc
/*s: kernel basic includes */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
/*e: kernel basic includes */

/*s: xalloc.c forward decl */
void        xhole(ulong, ulong);
/*e: xalloc.c forward decl */

//*****************************************************************************
// The global
//*****************************************************************************

/*s: global xlists */
static Xalloc   xlists;
/*e: global xlists */

//*****************************************************************************
// Initialization
//*****************************************************************************

/*s: function xinit */
void
xinit(void)
{
    int i, n, upages, kpages;
    ulong maxpages;
    Confmem *m;
    Pallocmem *pm;
    Hole *h, *eh;

    eh = &xlists.hole[Nhole-1];
    for(h = xlists.hole; h < eh; h++)
        h->link = h+1;

    xlists.flist = xlists.hole;

    upages = conf.upages;
    kpages = conf.npage - upages;
    pm = palloc.mem;
    for(i=0; i<nelem(conf.mem); i++){
        m = &conf.mem[i];
        n = m->npage;
        if(n > kpages)
            n = kpages;
        /* don't try to use non-KADDR-able memory for kernel */
        maxpages = cankaddr(m->base)/BY2PG;
        if(n > maxpages)
            n = maxpages;
        /* first give to kernel */
        if(n > 0){
            m->kbase = (ulong)KADDR(m->base);
            m->klimit = (ulong)KADDR(m->base+n*BY2PG);
            xhole(m->base, n*BY2PG);
            kpages -= n;
        }
        /* if anything left over, give to user */
        if(n < m->npage){
            if(pm >= palloc.mem+nelem(palloc.mem)){
                print("xinit: losing %lud pages\n", m->npage-n);
                continue;
            }
            pm->base = m->base+n*BY2PG;
            pm->npage = m->npage - n;
            pm++;
        }
    }
//  xsummary();         /* call it from main if desired */
}
/*e: function xinit */

//*****************************************************************************
// Functions
//*****************************************************************************

/*s: function xspanalloc */
void*
xspanalloc(ulong size, int align, ulong span)
{
    ulong a, v, t;
    a = (ulong)xalloc(size+align+span);
    if(a == 0)
        panic("xspanalloc: %lud %d %lux", size, align, span);

    if(span > 2) {
        v = (a + span) & ~(span-1);
        t = v - a;
        if(t > 0)
            xhole(PADDR(a), t);
        t = a + span - v;
        if(t > 0)
            xhole(PADDR(v+size+align), t);
    }
    else
        v = a;

    if(align > 1)
        v = (v + align) & ~(align-1);

    return (void*)v;
}
/*e: function xspanalloc */

/*s: function xallocz */
void*
xallocz(ulong size, bool zero)
{
    Xhdr *p;
    Hole *h, **l;

    /* add room for magix & size overhead, round up to nearest vlong */
    size += BY2V + offsetof(Xhdr, data[0]);
    size &= ~(BY2V-1);

    ilock(&xlists);
    l = &xlists.table;
    for(h = *l; h; h = h->link) {
        if(h->size >= size) {
            p = (Xhdr*)KADDR(h->addr);
            h->addr += size;
            h->size -= size;
            if(h->size == 0) {
                *l = h->link;
                h->link = xlists.flist;
                xlists.flist = h;
            }
            iunlock(&xlists);
            if(zero)
                memset(p, 0, size);
            p->magix = Magichole;
            p->size = size;
            return p->data;
        }
        l = &h->link;
    }
    iunlock(&xlists);
    return nil;
}
/*e: function xallocz */

/*s: function xalloc */
void*
xalloc(ulong size)
{
    return xallocz(size, true);
}
/*e: function xalloc */

/*s: function xfree */
void
xfree(void *p)
{
    Xhdr *x;

    x = (Xhdr*)((ulong)p - offsetof(Xhdr, data[0]));
    if(x->magix != Magichole) {
        xsummary();
        panic("xfree(%#p) %#ux != %#lux", p, Magichole, x->magix);
    }
    xhole(PADDR((uintptr)x), x->size);
}
/*e: function xfree */

/*s: function xmerge */
int
xmerge(void *vp, void *vq)
{
    Xhdr *p, *q;

    p = (Xhdr*)(((ulong)vp - offsetof(Xhdr, data[0])));
    q = (Xhdr*)(((ulong)vq - offsetof(Xhdr, data[0])));
    if(p->magix != Magichole || q->magix != Magichole) {
        int i;
        ulong *wd;
        void *badp;

        xsummary();
        badp = (p->magix != Magichole? p: q);
        wd = (ulong *)badp - 12;
        for (i = 24; i-- > 0; ) {
            print("%#p: %lux", wd, *wd);
            if (wd == badp)
                print(" <-");
            print("\n");
            wd++;
        }
        panic("xmerge(%#p, %#p) bad magic %#lux, %#lux",
            vp, vq, p->magix, q->magix);
    }
    if((uchar*)p+p->size == (uchar*)q) {
        p->size += q->size;
        return 1;
    }
    return 0;
}
/*e: function xmerge */

/*s: function xhole */
void
xhole(ulong addr, ulong size)
{
    ulong top;
    Hole *h, *c, **l;

    if(size == 0)
        return;

    top = addr + size;
    ilock(&xlists);
    l = &xlists.table;
    for(h = *l; h; h = h->link) {
        if(h->top == addr) {
            h->size += size;
            h->top = h->addr+h->size;
            c = h->link;
            if(c && h->top == c->addr) {
                h->top += c->size;
                h->size += c->size;
                h->link = c->link;
                c->link = xlists.flist;
                xlists.flist = c;
            }
            iunlock(&xlists);
            return;
        }
        if(h->addr > addr)
            break;
        l = &h->link;
    }
    if(h && top == h->addr) {
        h->addr -= size;
        h->size += size;
        iunlock(&xlists);
        return;
    }

    if(xlists.flist == nil) {
        iunlock(&xlists);
        print("xfree: no free holes, leaked %lud bytes\n", size);
        return;
    }

    h = xlists.flist;
    xlists.flist = h->link;
    h->addr = addr;
    h->top = top;
    h->size = size;
    h->link = *l;
    *l = h;
    iunlock(&xlists);
}
/*e: function xhole */

//*****************************************************************************
// Debugging
//*****************************************************************************

/*s: function xsummary */
void
xsummary(void)
{
    int i;
    Hole *h;

    i = 0;
    for(h = xlists.flist; h; h = h->link)
        i++;

    print("%d holes free", i);
    i = 0;
    for(h = xlists.table; h; h = h->link) {
        if (0) {
            print("addr %#.8lux top %#.8lux size %lud\n",
                h->addr, h->top, h->size);
            delay(10);
        }
        i += h->size;
        if (h == h->link) {
            print("xsummary: infinite loop broken\n");
            break;
        }
    }
    print(" %d bytes free\n", i);
}
/*e: function xsummary */
/*e: xalloc.c */
