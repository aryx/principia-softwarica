/*s: linkers/8l/elf.c */
/*
 * emit 32- or 64-bit elf headers for any architecture.
 * this is a component of ?l.
 */
#include "l.h"

/*s: enum _anon_ (linkers/8l/elf.c) */
enum {
    /* offsets into string table */
    Stitext		= 1,
    Stidata		= 7,
    Stistrtab	= 13,
};
/*e: enum _anon_ (linkers/8l/elf.c) */

/*s: function elfident */
void
elfident(int bo, int class)
{
    strnput("\177ELF", 4);		/* e_ident */
    cput(class);
    cput(bo);			/* byte order */
    cput(1);			/* version = CURRENT */
    if(debug['k']){			/* boot/embedded/standalone */
        cput(255);
        cput(0);
    }
    else{
        cput(0);		/* osabi = SYSV */
        cput(0);		/* abiversion = 3 */
    }
    strnput("", 7);
}
/*e: function elfident */

/*s: function elfstrtab */
void
elfstrtab(void)
{
    /* string table */
    cput(0);
    strnput(".text", 5);		/* +1 */
    cput(0);
    strnput(".data", 5);		/* +7 */
    cput(0);
    strnput(".strtab", 7);		/* +13 */
    cput(0);
    cput(0);
}
/*e: function elfstrtab */

/*s: function elf32phdr */
void
elf32phdr(void (*putl)(long), ulong type, ulong off, ulong vaddr, ulong paddr,
    ulong filesz, ulong memsz, ulong prots, ulong align)
{
    putl(type);
    putl(off);
    putl(vaddr);
    putl(paddr);
    putl(filesz);
    putl(memsz);
    putl(prots);
    putl(align);
}
/*e: function elf32phdr */

/*s: function elf32shdr */
void
elf32shdr(void (*putl)(long), ulong name, ulong type, ulong flags, ulong vaddr,
    ulong off, ulong sectsz, ulong link, ulong addnl, ulong align,
    ulong entsz)
{
    putl(name);
    putl(type);
    putl(flags);
    putl(vaddr);
    putl(off);
    putl(sectsz);
    putl(link);
    putl(addnl);
    putl(align);
    putl(entsz);
}
/*e: function elf32shdr */

/*s: function elf32sectab */
static void
elf32sectab(void (*putl)(long))
{
    seek(cout, HEADR+textsize+datsize+symsize, 0);
    elf32shdr(putl, Stitext, Progbits, Salloc|Sexec, INITTEXT,
        HEADR, textsize, 0, 0, 0x10000, 0);
    elf32shdr(putl, Stidata, Progbits, Salloc|SwriteElf, INITDAT,
        HEADR+textsize, datsize, 0, 0, 0x10000, 0);
    elf32shdr(putl, Stistrtab, Strtab, 1 << 5, 0,
        HEADR+textsize+datsize+symsize+3*Shdr32sz, 14, 0, 0, 1, 0);
    elfstrtab();
}
/*e: function elf32sectab */

/*s: function elf32 */
/* if addpsects > 0, putpsects must emit exactly that many psects. */
void
elf32(int mach, int bo, int addpsects, void (*putpsects)(Putl))
{
    ulong phydata;
    void (*putw)(long), (*putl)(long);

    if(bo == ELFDATA2MSB){
        putw = wput;
        putl = lput;
    }else if(bo == ELFDATA2LSB){
        putw = wputl;
        putl = lputl;
    }else{
        print("elf32 byte order is mixed-endian\n");
        errorexit();
        return;
    }

    elfident(bo, ELFCLASS32);
    putw(EXEC);
    putw(mach);
    putl(1L);			/* version = CURRENT */
    putl(entryvalue());		/* entry vaddr */
    putl(Ehdr32sz);			/* offset to first phdr */
    if(debug['S'])
        putl(HEADR+textsize+datsize+symsize); /* offset to first shdr */
    else
        putl(0);
    putl(0L);			/* flags */
    putw(Ehdr32sz);
    putw(Phdr32sz);
    putw(3 + addpsects);		/* # of Phdrs */
    putw(Shdr32sz);
    if(debug['S']){
        putw(3);		/* # of Shdrs */
        putw(2);		/* Shdr table index */
    }else{
        putw(0);
        putw(0);
    }

    /*
     * could include ELF headers in text -- 8l doesn't,
     * but in theory it aids demand loading.
     */
    elf32phdr(putl, PT_LOAD, HEADR, INITTEXT, INITTEXTP,
        textsize, textsize, R|X, INITRND);	/* text */
    /*
     * we need INITDATP, but it has to be computed.
     * assume distance between INITTEXT & INITTEXTP is also
     * correct for INITDAT and INITDATP.
     */
    phydata = INITDAT - (INITTEXT - INITTEXTP);
    elf32phdr(putl, PT_LOAD, HEADR+textsize, INITDAT, phydata,
        datsize, datsize+bsssize, R|W|X, INITRND); /* data */
    elf32phdr(putl, NOPTYPE, HEADR+textsize+datsize, 0, 0,
        symsize, lcsize, R, 4);			/* symbol table */
    if (addpsects > 0)
        putpsects(putl);
    cflush();

    if(debug['S'])
        elf32sectab(putl);
}
/*e: function elf32 */

/*s: function elf64phdr */
/*
 * elf64
 */

void
elf64phdr(void (*putl)(long), void (*putll)(vlong), ulong type, uvlong off,
    uvlong vaddr, uvlong paddr, uvlong filesz, uvlong memsz, ulong prots,
    uvlong align)
{
    putl(type);		
    putl(prots);		
    putll(off);		
    putll(vaddr);	
    putll(paddr);	
    putll(filesz);	
    putll(memsz);	
    putll(align);		
}
/*e: function elf64phdr */

/*s: function elf64shdr */
void
elf64shdr(void (*putl)(long), void (*putll)(vlong), ulong name, ulong type,
    uvlong flags, uvlong vaddr, uvlong off, uvlong sectsz, ulong link,
    ulong addnl, uvlong align, uvlong entsz)
{
    putl(name);
    putl(type);
    putll(flags);
    putll(vaddr);
    putll(off);
    putll(sectsz);
    putl(link);
    putl(addnl);
    putll(align);
    putll(entsz);
}
/*e: function elf64shdr */

/*s: function elf64sectab */
static void
elf64sectab(void (*putl)(long), void (*putll)(vlong))
{
    seek(cout, HEADR+textsize+datsize+symsize, 0);
    elf64shdr(putl, putll, Stitext, Progbits, Salloc|Sexec, INITTEXT,
        HEADR, textsize, 0, 0, 0x10000, 0);
    elf64shdr(putl, putll, Stidata, Progbits, Salloc|SwriteElf, INITDAT,
        HEADR+textsize, datsize, 0, 0, 0x10000, 0);
    elf64shdr(putl, putll, Stistrtab, Strtab, 1 << 5, 0,
        HEADR+textsize+datsize+symsize+3*Shdr64sz, 14, 0, 0, 1, 0);
    elfstrtab();
}
/*e: function elf64sectab */

/*s: function elf64 */
/* if addpsects > 0, putpsects must emit exactly that many psects. */
void
elf64(int mach, int bo, int addpsects, void (*putpsects)(Putl))
{
    uvlong phydata;
    void (*putw)(long), (*putl)(long);
    void (*putll)(vlong);

    if(bo == ELFDATA2MSB){
        putw = wput;
        putl = lput;
        putll = llput;
    }else if(bo == ELFDATA2LSB){
        putw = wputl;
        putl = lputl;
        putll = llputl;
    }else{
        print("elf64 byte order is mixed-endian\n");
        errorexit();
        return;
    }

    elfident(bo, ELFCLASS64);
    putw(EXEC);
    putw(mach);
    putl(1L);			/* version = CURRENT */
    putll(entryvalue());		/* entry vaddr */
    putll(Ehdr64sz);		/* offset to first phdr */
    if(debug['S'])
        putll(HEADR+textsize+datsize+symsize); /* offset to 1st shdr */
    else
        putll(0);
    putl(0L);			/* flags */
    putw(Ehdr64sz);
    putw(Phdr64sz);
    putw(3 + addpsects);		/* # of Phdrs */
    putw(Shdr64sz);
    if(debug['S']){
        putw(3);		/* # of Shdrs */
        putw(2);		/* Shdr table index */
    }else{
        putw(0);
        putw(0);
    }

    elf64phdr(putl, putll, PT_LOAD, HEADR, INITTEXT, INITTEXTP,
        textsize, textsize, R|X, INITRND);	/* text */
    /*
     * see 32-bit ELF case for physical data address computation.
     */
    phydata = INITDAT - (INITTEXT - INITTEXTP);
    elf64phdr(putl, putll, PT_LOAD, HEADR+textsize, INITDAT, phydata,
        datsize, datsize+bsssize, R|W, INITRND); /* data */
    elf64phdr(putl, putll, NOPTYPE, HEADR+textsize+datsize, 0, 0,
        symsize, lcsize, R, 4);			/* symbol table */
    if (addpsects > 0)
        putpsects(putl);
    cflush();

    if(debug['S'])
        elf64sectab(putl, putll);
}
/*e: function elf64 */
/*e: linkers/8l/elf.c */
