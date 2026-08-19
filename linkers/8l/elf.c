/*s: 8l/elf.c */
/*
 * emit 32- or 64-bit elf headers for any architecture.
 * this is a component of ?l.
 */
#include "l.h"

long	entryvalue(void);

// specific to each archi, defined in asm.c
void    strnput(char*, int);
void    lputl(int32);

/*s: enum _anon_ (linkers/8l/elf.c) */
enum {
    /* offsets into string table */
    Stitext		= 1,
    Stidata		= 7,
    Stistrtab	= 13,
};
/*e: enum _anon_ (linkers/8l/elf.c) */

/*s: function [[elfident]] */
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
/*e: function [[elfident]] */

/*s: function [[elfstrtab]] */
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
/*e: function [[elfstrtab]] */

/*s: function [[elf32phdr]] */
void
elf32phdr(void (*putl)(int32), uint32 type, uint32 off, uint32 vaddr, uint32 paddr,
    uint32 filesz, uint32 memsz, uint32 prots, uint32 align)
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
/*e: function [[elf32phdr]] */

/*s: function [[elf32shdr]] */
void
elf32shdr(void (*putl)(int32), uint32 name, uint32 type, uint32 flags, uint32 vaddr,
    uint32 off, uint32 sectsz, uint32 link, uint32 addnl, uint32 align,
    uint32 entsz)
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
/*e: function [[elf32shdr]] */

/*s: function [[elf32sectab]] */
static void
elf32sectab(void (*putl)(int32))
{
    seek(cout, HEADR+textsize+datsize+symsize, 0);
    elf32shdr(putl, Stitext, Progbits, Salloc|Sexec, INITTEXT,
        HEADR, textsize, 0, 0, 0x10000, 0);
    // claude: like the phdr, the data section file offset must be
    // page-aligned to match INITDAT (rnd() ported from the k variant)
    elf32shdr(putl, Stidata, Progbits, Salloc|SwriteElf, INITDAT,
        rnd(HEADR+textsize, INITRND), datsize, 0, 0, 0x10000, 0);
    elf32shdr(putl, Stistrtab, Strtab, 1 << 5, 0,
        HEADR+textsize+datsize+symsize+3*Shdr32sz, 14, 0, 0, 1, 0);
    elfstrtab();
}
/*e: function [[elf32sectab]] */

/*s: function [[elf32]] */
/* if addpsects > 0, putpsects must emit exactly that many psects. */
void
elf32(int mach, int bo, int addpsects, void (*putpsects)(Putl))
{
    uint32 phydata;
    void (*putw)(int32), (*putl)(int32);

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
    // claude: the kencc lk/elf.c switches on thechar here, but this
    // shared elf.c only sees 8l's l.h (relative include) which does
    // not declare thechar; the mach parameter is the ELF machine type
    // and the EABI flags word is a property of exactly that, so switch
    // on mach instead.
    //old: putl(0L);			/* flags */
    switch(mach) {
    case ARM:
        // version5 EABI soft-float, like the kencc 5l
        putl(0x5000200);		/* flags */
        break;
    default:
        putl(0L);			/* flags */
        break;
    }
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
    // claude: file offset must be page-aligned like INITDAT (see the
    // matching rnd() in asm.c's asmb(), which is where this is written)
    elf32phdr(putl, PT_LOAD, rnd(HEADR+textsize, INITRND), INITDAT, phydata,
        datsize, datsize+bsssize, R|W|X, INITRND); /* data */
    elf32phdr(putl, NOPTYPE, HEADR+textsize+datsize, 0, 0,
        symsize, lcsize, R, 4);			/* symbol table */
    if (addpsects > 0)
        putpsects(putl);
    cflush();

    if(debug['S'])
        elf32sectab(putl);
}
/*e: function [[elf32]] */
/*e: 8l/elf.c */
