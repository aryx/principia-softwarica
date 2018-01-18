/*s: machine/5i/mem.c */
/*s: basic includes */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>

#include "arm.h"
/*e: basic includes */

void*		page_of_vaddr(ulong);

/*s: function [[ifetch]] */
instruction
ifetch(uintptr addr)
{
    byte *va;

    if(addr&3) {
        Bprint(bout, "Address error (I-fetch) vaddr %.8lux\n", addr);
        longjmp(errjmp, 0);
    }

    /*s: [[ifetch()]] instruction cache handling */
    if(icache.on)
        updateicache(addr);
    /*e: [[ifetch()]] instruction cache handling */
    iprof[(addr-textbase)/PROFGRAN]++;

    va = page_of_vaddr(addr); // get page
    va += addr&(BY2PG-1); // restore offset in page

    return va[3]<<24 | va[2]<<16 | va[1]<<8 | va[0];
}
/*e: function [[ifetch]] */

/*s: function [[getmem_4]] */
ulong
getmem_4(uintptr addr)
{
    ulong val;
    int i;

    val = 0;
    for(i = 0; i < 4; i++)
        val = (val>>8) | (getmem_b(addr++)<<24);
    return val;
}
/*e: function [[getmem_4]] */

/*s: function [[getmem_2]] */
ulong
getmem_2(uintptr addr)
{
    ulong val;
    int i;

    val = 0;
    for(i = 0; i < 2; i++)
        val = (val>>8) | (getmem_b(addr++)<<16);
    return val;
}
/*e: function [[getmem_2]] */

/*s: function [[getmem_w]] */
ulong
getmem_w(uintptr addr)
{
    byte *va;
    ulong w;

    if(addr&3) {
        w = getmem_w(addr & ~3);
        while(addr & 3) {
            w = (w>>8) | (w<<24);
            addr--;
        }
        return w;
    }
    /*s: [[getmem_x()]] if membpt */
    if(membpt)
        brkchk(addr, Read);
    /*e: [[getmem_x()]] if membpt */

    va = page_of_vaddr(addr);
    va += addr&(BY2PG-1);

    return va[3]<<24 | va[2]<<16 | va[1]<<8 | va[0];
}
/*e: function [[getmem_w]] */

/*s: function [[getmem_h]] */
ushort
getmem_h(uintptr addr)
{
    byte *va;
    ulong w;

    if(addr&1) {
        w = getmem_h(addr & ~1);
        while(addr & 1) {
            w = (w>>8) | (w<<8);
            addr--;
        }
        return w;
    }
    /*s: [[getmem_x()]] if membpt */
    if(membpt)
        brkchk(addr, Read);
    /*e: [[getmem_x()]] if membpt */

    va = page_of_vaddr(addr);
    va += addr&(BY2PG-1);

    return va[1]<<8 | va[0];
}
/*e: function [[getmem_h]] */

/*s: function [[getmem_b]] */
byte
getmem_b(uintptr addr)
{
    byte *va;

    /*s: [[getmem_x()]] if membpt */
    if(membpt)
        brkchk(addr, Read);
    /*e: [[getmem_x()]] if membpt */

    va = page_of_vaddr(addr);
    va += addr&(BY2PG-1);
    return va[0];
}
/*e: function [[getmem_b]] */

/*s: function [[getmem_v]] */
uvlong
getmem_v(uintptr addr)
{
    return ((uvlong)getmem_w(addr+4) << 32) | getmem_w(addr);
}
/*e: function [[getmem_v]] */

/*s: function [[putmem_h]] */
void
putmem_h(uintptr addr, ushort data)
{
    byte *va;

    if(addr&1) {
        Bprint(bout, "Address error (Store) vaddr %.8lux\n", addr);
        longjmp(errjmp, 0);
    }

    va = page_of_vaddr(addr);
    va += addr&(BY2PG-1);

    va[1] = data>>8;
    va[0] = data;

    /*s: [[putmem_x()]] if membpt */
    if(membpt)
        brkchk(addr, Write);
    /*e: [[putmem_x()]] if membpt */
}
/*e: function [[putmem_h]] */

/*s: function [[putmem_w]] */
void
putmem_w(uintptr addr, ulong data)
{
    byte *va;

    if(addr&3) {
        Bprint(bout, "Address error (Store) vaddr %.8lux\n", addr);
        longjmp(errjmp, 0);
    }

    va = page_of_vaddr(addr);
    va += addr&(BY2PG-1);

    va[3] = data>>24;
    va[2] = data>>16;
    va[1] = data>>8;
    va[0] = data;

    /*s: [[putmem_x()]] if membpt */
    if(membpt)
        brkchk(addr, Write);
    /*e: [[putmem_x()]] if membpt */
}
/*e: function [[putmem_w]] */

/*s: function [[putmem_b]] */
void
putmem_b(uintptr addr, byte data)
{
    byte *va;

    va = page_of_vaddr(addr);
    va += addr&(BY2PG-1);
    va[0] = data;

    /*s: [[putmem_x()]] if membpt */
    if(membpt)
        brkchk(addr, Write);
    /*e: [[putmem_x()]] if membpt */
}
/*e: function [[putmem_b]] */

/*s: function [[putmem_v]] */
void
putmem_v(uintptr addr, uvlong data)
{
    putmem_w(addr, data);	/* two stages, to catch brkchk */
    putmem_w(addr+4, data>>32);
}
/*e: function [[putmem_v]] */

/*s: function [[memio]] */
char *
memio(char *mb, uintptr mem, int size, int dir)
{
    int i;
    char *buf, c;

    if(mb == nil)
        mb = emalloc(size);

    buf = mb;
    switch(dir) {
    case MemRead:
        while(size--)
            *mb++ = getmem_b(mem++);
        break;
    case MemReadstring:
        for(;;) {
            if(size-- == 0) {
                Bprint(bout, "memio: user/kernel copy too long for arm\n");
                longjmp(errjmp, 0);
            }
            c = getmem_b(mem++);
            *mb++ = c;
            if(c == '\0')
                break;
        }
        break;
    case MemWrite:
        for(i = 0; i < size; i++)
            putmem_b(mem++, *mb++);
        break;
    default:
        fatal(false, "memio");
    }
    return buf;
}
/*e: function [[memio]] */

/*s: function [[dotlb]] */
void
dotlb(uintptr vaddr)
{
    ulong *l, *e;

    vaddr &= ~(BY2PG-1);

    e = &tlb.tlbent[tlb.tlbsize];
    for(l = tlb.tlbent; l < e; l++)
        if(*l == vaddr) {
            tlb.hit++;
            return;
        }

    tlb.miss++;
    tlb.tlbent[lnrand(tlb.tlbsize)] = vaddr;
}
/*e: function [[dotlb]] */

/*s: function [[page_of_vaddr]] */
void*
page_of_vaddr(uintptr addr)
{
    Segment *s, *es;
    int off, foff, l, n;
    byte **p, *a;

    /*s: [[page_of_vaddr()]] TLB handling */
    if(tlb.on)
        dotlb(addr);
    /*e: [[page_of_vaddr()]] TLB handling */

    es = &memory.seg[Nseg];
    for(s = memory.seg; s < es; s++) {
        if(addr >= s->base && addr < s->end) {
            s->refs++;
            off = (addr - s->base)/BY2PG;
            p = &s->table[off];

            if(*p)
                return *p;

            // else page fault! no allocated memory there yet
            s->rss++;

            switch(s->type) {
            /*s: [[page_of_vaddr()]] page fault, switch segment type cases */
            case Text:
                *p = emalloc(BY2PG);
                if(seek(text, s->fileoff+(off*BY2PG), 0) < 0)
                    fatal(true, "page_of_vaddr text seek");
                if(read(text, *p, BY2PG) < 0)
                    fatal(true, "page_of_vaddr text read");
                return *p;
            /*x: [[page_of_vaddr()]] page fault, switch segment type cases */
            case Data:
                *p = emalloc(BY2PG);
                foff = s->fileoff+(off*BY2PG);
                if(seek(text, foff, 0) < 0)
                    fatal(true, "page_of_vaddr text seek");
                n = read(text, *p, BY2PG);
                if(n < 0)
                    fatal(true, "page_of_vaddr text read");
                if(foff + n > s->fileend) {
                    l = BY2PG - (s->fileend-foff);
                    a = *p+(s->fileend-foff);
                    memset(a, 0, l);
                }
                return *p;
            /*x: [[page_of_vaddr()]] page fault, switch segment type cases */
            case Bss:
            case Stack:
                *p = emalloc(BY2PG);
                return *p;
            /*e: [[page_of_vaddr()]] page fault, switch segment type cases */
            default:
                fatal(false, "page_of_vaddr");
            }
        }
    }
    // reach here if didn't find any segment with relevant range
    Bprint(bout, "User TLB miss vaddr 0x%.8lux\n", addr);
    Bflush(bout);
    longjmp(errjmp, 0);
    return nil;		/*to stop compiler whining*/
}
/*e: function [[page_of_vaddr]] */
/*e: machine/5i/mem.c */
