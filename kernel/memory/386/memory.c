/*s: memory/386/memory.c */
/*
 * Size memory and create the kernel page-tables on the fly while doing so.
 * Called from main(), this code should only be run by the bootstrap processor.
 *
 * MemMin is what the bootstrap code in l.s has already mapped;
 * MemMax is the limit of physical memory to scan.
 */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */
#include "io.h"
#include <ureg.h>

/*s: memory.c debugging macro(x86) */
#define MEMDEBUG    0
/*e: memory.c debugging macro(x86) */

/*s: enum [[memkind]](x86) */
enum Memkind {
    MemUPA,        /* unbacked physical address */
    MemRAM,        /* physical memory */
    MemUMB,        /* upper memory block (<16MB) */

    NMemType, // must be last
};
/*e: enum [[memkind]](x86) */

enum {
    MemMin      = 4*MB,
    MemMax      = (3*1024+768)*MB,
};
// coupling: with l.s for MemMin. 
// Why MemMax is not 4Go? because iterate by 256MB each time?

/*s: memory.c forward decl(x86) */
typedef struct Map Map;
typedef struct RMap RMap;
/*e: memory.c forward decl(x86) */
/*s: struct [[Map]](x86) */
struct Map {
    phys_addr addr;
    ulong size; // in bytes
};
/*e: struct [[Map]](x86) */

/*s: struct [[RMap]](x86) */
struct RMap {
    char*   name;
    //ref<Map> in mapram
    Map*    map;
    //ref<Map> in mapram
    Map*    mapend;
    Lock;
};
/*e: struct [[RMap]](x86) */

/* 
 * Memory allocation tracking.
 */
static Map mapupa[16];
static RMap rmapupa = {
    "unallocated unbacked physical memory",
    mapupa,
    &mapupa[nelem(mapupa)-1],
};

/*s: global [[mapram]](x86) */
static Map mapram[16];
static RMap rmapram = {
    "physical memory",
    mapram,
    &mapram[nelem(mapram)-1],
};
/*e: global [[mapram]](x86) */

static Map mapumb[64];
static RMap rmapumb = {
    "upper memory block",
    mapumb,
    &mapumb[nelem(mapumb)-1],
};

static Map mapumbrw[16];
static RMap rmapumbrw = {
    "UMB device memory",
    mapumbrw,
    &mapumbrw[nelem(mapumbrw)-1],
};

/*s: function [[mapprint]](x86) */
void
mapprint(RMap *rmap)
{
    Map *mp;

    print("%s\n", rmap->name);  
    for(mp = rmap->map; mp->size; mp++)
        print("\t%8.8luX %8.8luX (%lud)\n", mp->addr, mp->addr+mp->size, mp->size);
}
/*e: function [[mapprint]](x86) */

/*s: function [[memdebug]](x86) */
void
memdebug(void)
{
    ulong maxpa, maxpa1, maxpa2;

    maxpa = (nvramread(0x18)<<8)|nvramread(0x17);
    maxpa1 = (nvramread(0x31)<<8)|nvramread(0x30);
    maxpa2 = (nvramread(0x16)<<8)|nvramread(0x15);
    print("maxpa = %luX -> %luX, maxpa1 = %luX maxpa2 = %luX\n",
        maxpa, MB+maxpa*KB, maxpa1, maxpa2);

    mapprint(&rmapram);
    mapprint(&rmapumb);
    mapprint(&rmapumbrw);
    mapprint(&rmapupa);
}
/*e: function [[memdebug]](x86) */

/*s: function [[mapfree]](x86) */
void
mapfree(RMap* rmap, phys_addr addr, ulong size)
{
    Map *mp;
    ulong t;

    if(size <= 0)
        return;

    lock(rmap);
    for(mp = rmap->map; mp->addr <= addr && mp->size; mp++)
        ;

    if(mp > rmap->map && (mp-1)->addr+(mp-1)->size == addr){
        (mp-1)->size += size;
        if(addr+size == mp->addr){
            (mp-1)->size += mp->size;
            while(mp->size){
                mp++;
                (mp-1)->addr = mp->addr;
                (mp-1)->size = mp->size;
            }
        }
    }
    else{
        if(addr+size == mp->addr && mp->size){
            mp->addr -= size;
            mp->size += size;
        }
        else do{
            if(mp >= rmap->mapend){
                print("mapfree: %s: losing 0x%luX, %ld\n",
                    rmap->name, addr, size);
                break;
            }
            t = mp->addr;
            mp->addr = addr;
            addr = t;
            t = mp->size;
            mp->size = size;
            mp++;
        }while(size = t);
    }
    unlock(rmap);
}
/*e: function [[mapfree]](x86) */

/*s: function [[mapalloc]](x86) */
ulong
mapalloc(RMap* rmap, ulong addr, int size, int align)
{
    Map *mp;
    ulong maddr, oaddr;

    lock(rmap);
    for(mp = rmap->map; mp->size; mp++){
        maddr = mp->addr;

        if(addr){
            /*
             * A specific address range has been given:
             *   if the current map entry is greater then
             *   the address is not in the map;
             *   if the current map entry does not overlap
             *   the beginning of the requested range then
             *   continue on to the next map entry;
             *   if the current map entry does not entirely
             *   contain the requested range then the range
             *   is not in the map.
             */
            if(maddr > addr)
                break;
            if(mp->size < addr - maddr) /* maddr+mp->size < addr, but no overflow */
                continue;
            if(addr - maddr > mp->size - size)  /* addr+size > maddr+mp->size, but no overflow */
                break;
            maddr = addr;
        }

        if(align > 0)
            maddr = ((maddr+align-1)/align)*align;
        if(mp->addr+mp->size-maddr < size)
            continue;

        oaddr = mp->addr;
        mp->addr = maddr+size;
        mp->size -= maddr-oaddr+size;
        if(mp->size == 0){
            do{
                mp++;
                (mp-1)->addr = mp->addr;
            }while((mp-1)->size = mp->size);
        }

        unlock(rmap);
        if(oaddr != maddr)
            mapfree(rmap, oaddr, maddr-oaddr);

        return maddr;
    }
    unlock(rmap);
    return nilptr;
}
/*e: function [[mapalloc]](x86) */

/*s: function [[rampage]](x86) */
/*
 * Allocate from the ram map directly to make page tables.
 * Called by mmuwalk during e820scan.
 */
kern_addr3
rampage(void)
{
    phys_addr m;
    
    m = mapalloc(&rmapram, 0, BY2PG, BY2PG);
    if(m == nilptr)
        return nil;
    return KADDR(m);
}
/*e: function [[rampage]](x86) */

/*s: function [[umbscan]](x86) */
static void
umbscan(void)
{
    byte o[2], *p;

    /*
     * Scan the Upper Memory Blocks (0xA0000->0xF0000) for pieces
     * which aren't used; they can be used later for devices which
     * want to allocate some virtual address space.
     * Check for two things:
     * 1) device BIOS ROM. This should start with a two-byte header
     *    of 0x55 0xAA, followed by a byte giving the size of the ROM
     *    in 512-byte chunks. These ROM's must start on a 2KB boundary.
     * 2) device memory. This is read-write.
     * There are some assumptions: there's VGA memory at 0xA0000 and
     * the VGA BIOS ROM is at 0xC0000. Also, if there's no ROM signature
     * at 0xE0000 then the whole 64KB up to 0xF0000 is theoretically up
     * for grabs; check anyway.
     */
    p = KADDR(0xD0000);
    while(p < KADDR(0xE0000)){
        /*
         * Check for the ROM signature, skip if valid.
         */
        if(p[0] == 0x55 && p[1] == 0xAA){
            p += p[2]*512;
            continue;
        }

        /*
         * Is it writeable? If yes, then stick it in
         * the UMB device memory map. A floating bus will
         * return 0xff, so add that to the map of the
         * UMB space available for allocation.
         * If it is neither of those, ignore it.
         */
        o[0] = p[0];
        p[0] = 0xCC;
        o[1] = p[2*KB-1];
        p[2*KB-1] = 0xCC;
        if(p[0] == 0xCC && p[2*KB-1] == 0xCC){
            p[0] = o[0];
            p[2*KB-1] = o[1];
            mapfree(&rmapumbrw, PADDR(p), 2*KB);
        }
        else if(p[0] == 0xFF && p[1] == 0xFF)
            mapfree(&rmapumb, PADDR(p), 2*KB);
        p += 2*KB;
    }

    p = KADDR(0xE0000);
    if(p[0] != 0x55 || p[1] != 0xAA){
        p[0] = 0xCC;
        p[64*KB-1] = 0xCC;
        if(p[0] != 0xCC && p[64*KB-1] != 0xCC)
            mapfree(&rmapumb, PADDR(p), 64*KB);
    }
}
/*e: function [[umbscan]](x86) */

/*s: function [[sigscan]](x86) */
static void*
sigscan(uchar* addr, int len, char* signature)
{
    int sl;
    uchar *e, *p;

    e = addr+len;
    sl = strlen(signature);
    for(p = addr; p+sl < e; p += 16)
        if(memcmp(p, signature, sl) == 0)
            return p;
    return nil;
}
/*e: function [[sigscan]](x86) */

/*s: function [[sigsearch]](x86) */
void*
sigsearch(char* signature)
{
    uintptr p;
    uchar *bda;
    void *r;

    /*
     * Search for the data structure:
     * 1) within the first KiB of the Extended BIOS Data Area (EBDA), or
     * 2) within the last KiB of system base memory if the EBDA segment
     *    is undefined, or
     * 3) within the BIOS ROM address space between 0xf0000 and 0xfffff
     *    (but will actually check 0xe0000 to 0xfffff).
     */
    bda = BIOSSEG(0x40);
    if(memcmp(KADDR(0xfffd9), "EISA", 4) == 0){
        if((p = (bda[0x0f]<<8)|bda[0x0e]) != 0){
            if((r = sigscan(BIOSSEG(p), 1024, signature)) != nil)
                return r;
        }
    }

    if((p = ((bda[0x14]<<8)|bda[0x13])*1024) != 0){
        if((r = sigscan(KADDR(p-1024), 1024, signature)) != nil)
            return r;
    }
    /* hack for virtualbox: look in KiB below 0xa0000 */
    if((r = sigscan(KADDR(0xa0000-1024), 1024, signature)) != nil)
        return r;

    return sigscan(BIOSSEG(0xe000), 0x20000, signature);
}
/*e: function [[sigsearch]](x86) */

/*s: function [[lowraminit]](x86) */
static void
lowraminit(void)
{
    ulong n, pa, x;
    byte *bda;

    /*
     * Initialise the memory bank information for conventional memory
     * (i.e. less than 640KB). The base is the first location after the
     * bootstrap processor MMU information and the limit is obtained from
     * the BIOS data area.
     */
    x = PADDR(CPU0END);
    bda = (byte*)KADDR(0x400);
    n = ((bda[0x14]<<8)|bda[0x13])*KB-x;
    mapfree(&rmapram, x, n);
    memset(KADDR(x), 0, n);         /* keep us honest */

    x = PADDR(PGROUND((ulong)end));
    pa = MemMin;
    if(x > pa)
        panic("kernel too big");
    mapfree(&rmapram, x, pa-x);
    memset(KADDR(x), 0, pa-x);      /* keep us honest */
}
/*e: function [[lowraminit]](x86) */

/*s: function [[ramscan]](x86) */
static void
ramscan(phys_addr maxmem)
{
    ulong *k0, kzero, map, *pte, *table, *va, vbase, x;
    phys_addr pa, maxpa, maxkpa;

    // hash<enum<memtype>, int> nb pages per memory type
    int nvalid[NMemType];

    /*
     * The bootstrap code has created a prototype page
     * table which maps the first MemMin of physical memory to KZERO.
     * The page directory is at cpu->pdproto and the first page of
     * free memory is after the per-processor MMU information.
     */
    pa = MemMin;

    /*
     * Check if the extended memory size can be obtained from the CMOS.
     * If it's 0 then it's either not known or >= 64MB. Always check
     * at least 24MB in case there's a memory gap (up to 8MB) below 16MB;
     * in this case the memory from the gap is remapped to the top of
     * memory.
     * The value in CMOS is supposed to be the number of KB above 1MB.
     */
    if(maxmem == 0){
        x = (nvramread(0x18)<<8)|nvramread(0x17);
        if(x == 0 || x >= (63*KB))
            maxpa = MemMax;
        else
            maxpa = MB+x*KB;
        if(maxpa < 24*MB)
            maxpa = 24*MB;
    }else
        maxpa = maxmem;

    maxkpa = MAXKPA; /* 2^32 - KZERO */

    /*
     * March up memory from MemMin to maxpa 1MB at a time,
     * mapping the first page and checking the page can
     * be written and read correctly. The page tables are created here
     * on the fly, allocating from low memory as necessary.
     */
    k0 = (kern_addr2)KADDR(0);
    kzero = *k0;
    map = 0;
    x = 0x12345678;
    memset(nvalid, 0, sizeof(nvalid));
    
    /*
     * Can't map memory to KADDR(pa) when we're walking because
     * can only use KADDR for relatively low addresses.
     * Instead, map each 4MB we scan to the virtual address range
     * MemMin->MemMin+4MB while we are scanning.
     */
    vbase = MemMin;
    while(pa < maxpa){
        /*
         * Map the page. Use mapalloc(&rmapram, ...) to make
         * the page table if necessary, it will be returned to the
         * pool later if it isn't needed.  Map in a fixed range (the second 4M)
         * because high physical addresses cannot be passed to KADDR.
         */
        va = (void*)(vbase + pa%(4*MB));
        table = &cpu->pdproto[PDX(va)];
        if(pa%(4*MB) == 0){
            if(map == 0 && (map = mapalloc(&rmapram, 0, BY2PG, BY2PG)) == 0)
                break;
            memset(KADDR(map), 0, BY2PG);
            *table = map|PTEWRITE|PTEVALID;
            memset(nvalid, 0, sizeof(nvalid));
        }
        table = KADDR(PPN(*table));
        pte = &table[PTX(va)];

        *pte = pa|PTEWRITE|PTEUNCACHED|PTEVALID;
        mmuflushtlb(PADDR(cpu->pdproto));
        /*
         * Write a pattern to the page and write a different
         * pattern to a possible mirror at KZERO. If the data
         * reads back correctly the chunk is some type of RAM (possibly
         * a linearly-mapped VGA framebuffer, for instance...) and
         * can be cleared and added to the memory pool. If not, the
         * chunk is marked uncached and added to the UMB pool if <16MB
         * or is marked invalid and added to the UPA pool.
         */
        *va = x;
        *k0 = ~x;
        if(*va == x){
            nvalid[MemRAM] += MB/BY2PG;
            mapfree(&rmapram, pa, MB);

            do{
                *pte++ = pa|PTEWRITE|PTEVALID;
                pa += BY2PG;
            }while(pa % MB);
            mmuflushtlb(PADDR(cpu->pdproto));
            /* memset(va, 0, MB); so damn slow to memset all of memory */
        }
        else if(pa < 16*MB){
            nvalid[MemUMB] += MB/BY2PG;
            mapfree(&rmapumb, pa, MB);

            do{
                *pte++ = pa|PTEWRITE|PTEUNCACHED|PTEVALID;
                pa += BY2PG;
            }while(pa % MB);
        }
        else{
            nvalid[MemUPA] += MB/BY2PG;
            mapfree(&rmapupa, pa, MB);
            *pte = 0;
            pa += MB;
        }

        /*
         * Done with this 4MB chunk, review the options:
         * 1) not physical memory and >=16MB - invalidate the PD entry;
         * 2) physical memory - use the 4MB page extension if possible;
         * 3) not physical memory and <16MB - use the 4MB page extension
         *    if possible;
         * 4) mixed or no 4MB page extension - commit the already
         *    initialised space for the page table.
         */
        if(pa%(4*MB) == 0 && pa >= 32*MB && nvalid[MemUPA] == (4*MB)/BY2PG){
            /*
             * If we encounter a 4MB chunk of missing memory
             * at a sufficiently high offset, call it the end of
             * memory.  Otherwise we run the risk of thinking
             * that video memory is real RAM.
             */
            break;
        }
        if(pa <= maxkpa && pa%(4*MB) == 0){
            table = &cpu->pdproto[PDX(KADDR(pa - 4*MB))];
            if(nvalid[MemUPA] == (4*MB)/BY2PG)
                *table = 0;
            else if(nvalid[MemRAM] == (4*MB)/BY2PG && (cpu->cpuiddx & 0x08))
                *table = (pa - 4*MB)|PTESIZE|PTEWRITE|PTEVALID;
            else if(nvalid[MemUMB] == (4*MB)/BY2PG && (cpu->cpuiddx & 0x08))
                *table = (pa - 4*MB)|PTESIZE|PTEWRITE|PTEUNCACHED|PTEVALID;
            else{
                *table = map|PTEWRITE|PTEVALID;
                map = 0;
            }
        }
        mmuflushtlb(PADDR(cpu->pdproto));
        x += 0x3141526;
    }
    /*
     * If we didn't reach the end of the 4MB chunk, that part won't
     * be mapped.  Commit the already initialised space for the page table.
     */
    if(pa % (4*MB) && pa <= maxkpa){
        cpu->pdproto[PDX(KADDR(pa))] = map|PTEWRITE|PTEVALID;
        map = 0;
    }
    if(map)
        mapfree(&rmapram, map, BY2PG);

    cpu->pdproto[PDX(vbase)] = 0;
    mmuflushtlb(PADDR(cpu->pdproto));

    mapfree(&rmapupa, pa, (u32int)(-pa));
    *k0 = kzero;
}
/*e: function [[ramscan]](x86) */

/*s: function [[meminit]](x86) */
void
meminit(void)
{
    int i;
    Map *mp;
    Confmem *cm;
    phys_addr pa;
    kern_addr2 pte;
    phys_addr maxmem;
    ulong lost;
    char *p;

    if(p = getconf("*maxmem"))
        maxmem = strtoul(p, 0, 0);
    else
        maxmem = 0;

    /*
     * Set special attributes for memory between 640KB and 1MB:
     *   VGA memory is writethrough;
     *   BIOS ROM's/UMB's are uncached;
     * then scan for useful memory.
     */
    for(pa = 0xA0000; pa < 0xC0000; pa += BY2PG){
        pte = mmuwalk(cpu->pdproto, (kern_addr)KADDR(pa), 2, false);
        *pte |= PTEWT;
    }
    for(pa = 0xC0000; pa < 0x100000; pa += BY2PG){
        pte = mmuwalk(cpu->pdproto, (kern_addr)KADDR(pa), 2, false);
        *pte |= PTEUNCACHED;
    }
    mmuflushtlb(PADDR(cpu->pdproto));

    umbscan();
    lowraminit();
    ramscan(maxmem);

    /*
     * Set the conf entries describing banks of allocatable memory.
     */
    for(i=0; i<nelem(mapram) && i<nelem(conf.mem); i++){
        mp = &rmapram.map[i];
        cm = &conf.mem[i];
        cm->base = mp->addr;
        cm->npage = mp->size/BY2PG;
    }
    
    lost = 0;
    for(; i<nelem(mapram); i++)
        lost += rmapram.map[i].size;
    if(lost)
        print("meminit - lost %lud bytes\n", lost);
    if(MEMDEBUG)
        memdebug();
}
/*e: function [[meminit]](x86) */

/*s: function [[upaalloc]](x86) */
/*
 * Give out otherwise-unused physical address space
 * for use in configuring devices.  Note that unlike upamalloc
 * before it, upaalloc does not map the physical address
 * into virtual memory.  Call vmap to do that.
 */
ulong
upaalloc(int size, int align)
{
    ulong a;

    a = mapalloc(&rmapupa, 0, size, align);
    if(a == 0){
        print("out of physical address space allocating %d\n", size);
        mapprint(&rmapupa);
    }
    return a;
}
/*e: function [[upaalloc]](x86) */

/*s: function [[upareserve]](x86) */
void
upareserve(ulong pa, int size)
{
    ulong a;
    
    a = mapalloc(&rmapupa, pa, size, 0);
    if(a != pa){
        /*
         * This can happen when we're using the E820
         * map, which might have already reserved some
         * of the regions claimed by the pci devices.
         */
    //  print("upareserve: cannot reserve pa=%#.8lux size=%d\n", pa, size);
        if(a != 0)
            mapfree(&rmapupa, a, size);
    }
}
/*e: function [[upareserve]](x86) */

/*s: function [[memorysummary]](x86) */
void
arch_memorysummary(void)
{
  int i;
  int npallocpage = 0;

  print("\n");
  print("etext = 0x%luX, edata = 0x%luX, eend = 0x%luX, sizeof long = %d\n",
        etext, edata, end, sizeof(long));
  for(i=0; i<nelem(conf.mem); i++) {
       print("conf mem %d start = 0x%luX, npage = %ld\n", 
          i,
          conf.mem[i].base,
          conf.mem[i].npage
          );
  }
  for(i=0; i<nelem(palloc.mem); i++) {
       print("palloc mem %d start = 0x%luX, npage = %ld\n", 
          i,
          palloc.mem[i].base,
          palloc.mem[i].npage
          );
       npallocpage += palloc.mem[i].npage;
  }
  print("size of Page = %d, palloc size = %d\n", 
        sizeof(Page),
        npallocpage*sizeof(Page)
        );

  print("size of Proc = %d, procalloc size = %d\n",
        sizeof(Proc),
        conf.nproc*sizeof(Proc)
        );
  print("\n");
  memdebug();
}
/*e: function [[memorysummary]](x86) */

/*e: memory/386/memory.c */
