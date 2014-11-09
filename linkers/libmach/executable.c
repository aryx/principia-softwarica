/*s: linkers/libmach/executable.c */
#include	<u.h>
#include	<libc.h>
#include	<bio.h>

#include	<bootexec.h>
#include	<mach.h>

#include	"elf.h"

/*
 *	All a.out header types.  The dummy entry allows canonical
 *	processing of the union as a sequence of longs
 */

typedef struct {
    union{
        struct {
            Exec;		/* a.out.h */
            uvlong hdr[1];
        };
        Ehdr;			/* elf.h */
        E64hdr;
    } e;
    long dummy;			/* padding to ensure extra long */
} ExecHdr;

static	int	common(int, Fhdr*, ExecHdr*);
static	int	commonllp64(int, Fhdr*, ExecHdr*);
static	int	adotout(int, Fhdr*, ExecHdr*);
static	int	elfdotout(int, Fhdr*, ExecHdr*);
static	int	armdotout(int, Fhdr*, ExecHdr*);
static	void	setsym(Fhdr*, long, long, long, vlong);
static	void	setdata(Fhdr*, uvlong, long, vlong, long);
static	void	settext(Fhdr*, uvlong, uvlong, long, vlong);
static	void	hswal(void*, int, ulong(*)(ulong));
static	uvlong	_round(uvlong, ulong);

/*s: struct Exectable */
/*
 *	definition of per-executable file type structures
 */
typedef struct Exectable{
    long	magic;			/* big-endian magic number of file */
    char	*name;			/* executable identifier */
    char	*dlmname;		/* dynamically loadable module identifier */
    uchar	type;			/* Internal code */
    uchar	_magic;			/* _MAGIC() magic */
    Mach	*mach;			/* Per-machine data */
    long	hsize;			/* header size */
    ulong	(*swal)(ulong);		/* beswal or leswal */
    int	(*hparse)(int, Fhdr*, ExecHdr*);
} ExecTable;
/*e: struct Exectable */

//PAD: removed many archi
extern	Mach	mi386;
extern	Mach	marm;

/*s: global exectab */
ExecTable exectab[] =
{
    { I_MAGIC,			/* I386 8.out & boot image */
        "386 plan 9 executable",
        "386 plan 9 dlm",
        FI386,
        1,
        &mi386,
        sizeof(Exec),
        beswal,
        common },
    { ELF_MAG,			/* any ELF */
        "elf executable",
        nil,
        FNONE,
        0,
        &mi386,
        sizeof(Ehdr),
        nil,
        elfdotout },
    { E_MAGIC,			/* Arm 5.out and boot image */
        "arm plan 9 executable",
        "arm plan 9 dlm",
        FARM,
        1,
        &marm,
        sizeof(Exec),
        beswal,
        common },

    { 0 },
};
/*e: global exectab */

/*s: global mach */
Mach	*mach = &mi386;			/* Global current machine table */
/*e: global mach */

/*s: function crackhdr */
int
crackhdr(int fd, Fhdr *fp)
{
    ExecTable *mp;
    ExecHdr d;
    int nb, ret;
    ulong magic;

    fp->type = FNONE;
    nb = read(fd, (char *)&d.e, sizeof(d.e));
    if (nb <= 0)
        return 0;

    ret = 0;
    magic = beswal(d.e.magic);		/* big-endian */
    for (mp = exectab; mp->magic; mp++) {
        if (nb < mp->hsize)
            continue;

        /*
         * The magic number has morphed into something
         * with fields (the straw was DYN_MAGIC) so now
         * a flag is needed in Fhdr to distinguish _MAGIC()
         * magic numbers from foreign magic numbers.
         *
         * This code is creaking a bit and if it has to
         * be modified/extended much more it's probably
         * time to step back and redo it all.
         */
        if(mp->_magic){
            if(mp->magic != (magic & ~DYN_MAGIC))
                continue;

//            if(mp->magic == V_MAGIC)
//                mp = couldbe4k(mp);

            if ((magic & DYN_MAGIC) && mp->dlmname != nil)
                fp->name = mp->dlmname;
            else
                fp->name = mp->name;
        }
        else{
            if(mp->magic != magic)
                continue;
            fp->name = mp->name;
        }
        fp->type = mp->type;
        fp->hdrsz = mp->hsize;		/* will be zero on bootables */
        fp->_magic = mp->_magic;
        fp->magic = magic;

        mach = mp->mach;
        if(mp->swal != nil)
            hswal(&d, sizeof(d.e)/sizeof(ulong), mp->swal);
        ret = mp->hparse(fd, fp, &d);
        seek(fd, mp->hsize, 0);		/* seek to end of header */
        break;
    }
    if(mp->magic == 0)
        werrstr("unknown header type");
    return ret;
}
/*e: function crackhdr */

/*s: function hswal */
/*
 * Convert header to canonical form
 */
static void
hswal(void *v, int n, ulong (*swap)(ulong))
{
    ulong *ulp;

    for(ulp = v; n--; ulp++)
        *ulp = (*swap)(*ulp);
}
/*e: function hswal */

/*s: function adotout */
/*
 *	Crack a normal a.out-type header
 */
static int
adotout(int fd, Fhdr *fp, ExecHdr *hp)
{
    long pgsize;

    USED(fd);
    pgsize = mach->pgsize;
    settext(fp, hp->e.entry, pgsize+sizeof(Exec),
            hp->e.text, sizeof(Exec));
    setdata(fp, _round(pgsize+fp->txtsz+sizeof(Exec), pgsize),
        hp->e.data, fp->txtsz+sizeof(Exec), hp->e.bss);
    setsym(fp, hp->e.syms, hp->e.spsz, hp->e.pcsz, fp->datoff+fp->datsz);
    return 1;
}
/*e: function adotout */

/*s: function commonboot */
static void
commonboot(Fhdr *fp)
{
    if (!(fp->entry & mach->ktmask))
        return;

    switch(fp->type) {				/* boot image */
    case F68020:
        fp->type = F68020B;
        fp->name = "68020 plan 9 boot image";
        break;
    case FI386:
        fp->type = FI386B;
        fp->txtaddr = (u32int)fp->entry;
        fp->name = "386 plan 9 boot image";
        fp->dataddr = _round(fp->txtaddr+fp->txtsz, mach->pgsize);
        break;
    case FARM:
        fp->type = FARMB;
        fp->txtaddr = (u32int)fp->entry;
        fp->name = "ARM plan 9 boot image";
        fp->dataddr = _round(fp->txtaddr+fp->txtsz, mach->pgsize);
        return;
    case FALPHA:
        fp->type = FALPHAB;
        fp->txtaddr = (u32int)fp->entry;
        fp->name = "alpha plan 9 boot image";
        fp->dataddr = fp->txtaddr+fp->txtsz;
        break;
    case FPOWER:
        fp->type = FPOWERB;
        fp->txtaddr = (u32int)fp->entry;
        fp->name = "power plan 9 boot image";
        fp->dataddr = fp->txtaddr+fp->txtsz;
        break;
    case FAMD64:
        fp->type = FAMD64B;
        fp->txtaddr = fp->entry;
        fp->name = "amd64 plan 9 boot image";
        fp->dataddr = _round(fp->txtaddr+fp->txtsz, 4096);
        break;
    case FPOWER64:
        fp->type = FPOWER64B;
        fp->txtaddr = fp->entry;
        fp->name = "power64 plan 9 boot image";
        fp->dataddr = fp->txtaddr+fp->txtsz;
        break;
    default:
        return;
    }
    fp->hdrsz = 0;			/* header stripped */
}
/*e: function commonboot */

/*s: function common */
/*
 *	_MAGIC() style headers and
 *	alpha plan9-style bootable images for axp "headerless" boot
 *
 */
static int
common(int fd, Fhdr *fp, ExecHdr *hp)
{
    adotout(fd, fp, hp);
    if(hp->e.magic & DYN_MAGIC) {
        fp->txtaddr = 0;
        fp->dataddr = fp->txtsz;
        return 1;
    }
    commonboot(fp);
    return 1;
}
/*e: function common */

/*s: function commonllp64 */
static int
commonllp64(int, Fhdr *fp, ExecHdr *hp)
{
    long pgsize;
    uvlong entry;

    hswal(&hp->e, sizeof(Exec)/sizeof(long), beswal);
    if(!(hp->e.magic & HDR_MAGIC))
        return 0;

    /*
     * There can be more magic here if the
     * header ever needs more expansion.
     * For now just catch use of any of the
     * unused bits.
     */
    if((hp->e.magic & ~DYN_MAGIC)>>16)
        return 0;
    entry = beswav(hp->e.hdr[0]);

    pgsize = mach->pgsize;
    settext(fp, entry, pgsize+fp->hdrsz, hp->e.text, fp->hdrsz);
    setdata(fp, _round(pgsize+fp->txtsz+fp->hdrsz, pgsize),
        hp->e.data, fp->txtsz+fp->hdrsz, hp->e.bss);
    setsym(fp, hp->e.syms, hp->e.spsz, hp->e.pcsz, fp->datoff+fp->datsz);

    if(hp->e.magic & DYN_MAGIC) {
        fp->txtaddr = 0;
        fp->dataddr = fp->txtsz;
        return 1;
    }
    commonboot(fp);
    return 1;
}
/*e: function commonllp64 */

/*s: function mipsboot */
/*e: function mipsboot */

/*s: function mips4kboot */
/*e: function mips4kboot */

/*s: function sparcboot */
/*e: function sparcboot */

/*s: function nextboot */
/*e: function nextboot */

/*s: function elf64dotout */
/*
 * ELF64 binaries.
 */
static int
elf64dotout(int fd, Fhdr *fp, ExecHdr *hp)
{
    E64hdr *ep;
    P64hdr *ph;
    ushort (*swab)(ushort);
    ulong (*swal)(ulong);
    uvlong (*swav)(uvlong);
    int i, it, id, is, phsz;
    uvlong uvl;

    ep = &hp->e;
    if(ep->ident[DATA] == ELFDATA2LSB) {
        swab = leswab;
        swal = leswal;
        swav = leswav;
    } else if(ep->ident[DATA] == ELFDATA2MSB) {
        swab = beswab;
        swal = beswal;
        swav = beswav;
    } else {
        werrstr("bad ELF64 encoding - not big or little endian");
        return 0;
    }

    ep->type = swab(ep->type);
    ep->machine = swab(ep->machine);
    ep->version = swal(ep->version);
    if(ep->type != EXEC || ep->version != CURRENT)
        return 0;
    ep->elfentry = swav(ep->elfentry);
    ep->phoff = swav(ep->phoff);
    ep->shoff = swav(ep->shoff);
    ep->flags = swal(ep->flags);
    ep->ehsize = swab(ep->ehsize);
    ep->phentsize = swab(ep->phentsize);
    ep->phnum = swab(ep->phnum);
    ep->shentsize = swab(ep->shentsize);
    ep->shnum = swab(ep->shnum);
    ep->shstrndx = swab(ep->shstrndx);

    fp->magic = ELF_MAG;
    fp->hdrsz = (ep->ehsize+ep->phnum*ep->phentsize+16)&~15;
    switch(ep->machine) {
    default:
        return 0;
    }

    if(ep->phentsize != sizeof(P64hdr)) {
        werrstr("bad ELF64 header size");
        return 0;
    }
    phsz = sizeof(P64hdr)*ep->phnum;
    ph = malloc(phsz);
    if(!ph)
        return 0;
    seek(fd, ep->phoff, 0);
    if(read(fd, ph, phsz) < 0) {
        free(ph);
        return 0;
    }
    for(i = 0; i < ep->phnum; i++) {
        ph[i].type = swal(ph[i].type);
        ph[i].flags = swal(ph[i].flags);
        ph[i].offset = swav(ph[i].offset);
        ph[i].vaddr = swav(ph[i].vaddr);
        ph[i].paddr = swav(ph[i].paddr);
        ph[i].filesz = swav(ph[i].filesz);
        ph[i].memsz = swav(ph[i].memsz);
        ph[i].align = swav(ph[i].align);
    }

    /* find text, data and symbols and install them */
    it = id = is = -1;
    for(i = 0; i < ep->phnum; i++) {
        if(ph[i].type == LOAD
        && (ph[i].flags & (R|X)) == (R|X) && it == -1)
            it = i;
        else if(ph[i].type == LOAD
        && (ph[i].flags & (R|W)) == (R|W) && id == -1)
            id = i;
        else if(ph[i].type == NOPTYPE && is == -1)
            is = i;
    }
    if(it == -1 || id == -1) {
        werrstr("No ELF64 TEXT or DATA sections");
        free(ph);
        return 0;
    }

    settext(fp, ep->elfentry, ph[it].vaddr, ph[it].memsz, ph[it].offset);
    /* 8c: out of fixed registers */
    uvl = ph[id].memsz - ph[id].filesz;
    setdata(fp, ph[id].vaddr, ph[id].filesz, ph[id].offset, uvl);
    if(is != -1)
        setsym(fp, ph[is].filesz, 0, ph[is].memsz, ph[is].offset);
    free(ph);
    return 1;
}
/*e: function elf64dotout */

/*s: function elf32dotout */
/*
 * ELF32 binaries.
 */
static int
elf32dotout(int fd, Fhdr *fp, ExecHdr *hp)
{
    ulong (*swal)(ulong);
    ushort (*swab)(ushort);
    Ehdr *ep;
    Phdr *ph;
    int i, it, id, is, phsz;

    /* bitswap the header according to the DATA format */
    ep = &hp->e;
    if(ep->ident[DATA] == ELFDATA2LSB) {
        swab = leswab;
        swal = leswal;
    } else if(ep->ident[DATA] == ELFDATA2MSB) {
        swab = beswab;
        swal = beswal;
    } else {
        werrstr("bad ELF32 encoding - not big or little endian");
        return 0;
    }

    ep->type = swab(ep->type);
    ep->machine = swab(ep->machine);
    ep->version = swal(ep->version);
    ep->elfentry = swal(ep->elfentry);
    ep->phoff = swal(ep->phoff);
    ep->shoff = swal(ep->shoff);
    ep->flags = swal(ep->flags);
    ep->ehsize = swab(ep->ehsize);
    ep->phentsize = swab(ep->phentsize);
    ep->phnum = swab(ep->phnum);
    ep->shentsize = swab(ep->shentsize);
    ep->shnum = swab(ep->shnum);
    ep->shstrndx = swab(ep->shstrndx);
    if(ep->type != EXEC || ep->version != CURRENT)
        return 0;

    /* we could definitely support a lot more machines here */
    fp->magic = ELF_MAG;
    fp->hdrsz = (ep->ehsize+ep->phnum*ep->phentsize+16)&~15;
    switch(ep->machine) {
    case I386:
        mach = &mi386;
        fp->type = FI386;
        fp->name = "386 ELF32 executable";
        break;
    case ARM:
        mach = &marm;
        fp->type = FARM;
        fp->name = "arm ELF32 executable";
        break;
    default:
        return 0;
    }

    if(ep->phentsize != sizeof(Phdr)) {
        werrstr("bad ELF32 header size");
        return 0;
    }
    phsz = sizeof(Phdr)*ep->phnum;
    ph = malloc(phsz);
    if(!ph)
        return 0;
    seek(fd, ep->phoff, 0);
    if(read(fd, ph, phsz) < 0) {
        free(ph);
        return 0;
    }
    hswal(ph, phsz/sizeof(ulong), swal);

    /* find text, data and symbols and install them */
    it = id = is = -1;
    for(i = 0; i < ep->phnum; i++) {
        if(ph[i].type == LOAD
        && (ph[i].flags & (R|X)) == (R|X) && it == -1)
            it = i;
        else if(ph[i].type == LOAD
        && (ph[i].flags & (R|W)) == (R|W) && id == -1)
            id = i;
        else if(ph[i].type == NOPTYPE && is == -1)
            is = i;
    }
    if(it == -1 || id == -1) {
        /*
         * The SPARC64 boot image is something of an ELF hack.
         * Text+Data+BSS are represented by ph[0].  Symbols
         * are represented by ph[1]:
         *
         *		filesz, memsz, vaddr, paddr, off
         * ph[0] : txtsz+datsz, txtsz+datsz+bsssz, txtaddr-KZERO, datasize, txtoff
         * ph[1] : symsz, lcsz, 0, 0, symoff
         */
        if(ep->machine == SPARC64 && ep->phnum == 2) {
            ulong txtaddr, txtsz, dataddr, bsssz;

            txtaddr = ph[0].vaddr | 0x80000000;
            txtsz = ph[0].filesz - ph[0].paddr;
            dataddr = txtaddr + txtsz;
            bsssz = ph[0].memsz - ph[0].filesz;
            settext(fp, ep->elfentry | 0x80000000, txtaddr, txtsz, ph[0].offset);
            setdata(fp, dataddr, ph[0].paddr, ph[0].offset + txtsz, bsssz);
            setsym(fp, ph[1].filesz, 0, ph[1].memsz, ph[1].offset);
            free(ph);
            return 1;
        }

        werrstr("No ELF32 TEXT or DATA sections");
        free(ph);
        return 0;
    }

    settext(fp, ep->elfentry, ph[it].vaddr, ph[it].memsz, ph[it].offset);
    setdata(fp, ph[id].vaddr, ph[id].filesz, ph[id].offset, ph[id].memsz - ph[id].filesz);
    if(is != -1)
        setsym(fp, ph[is].filesz, 0, ph[is].memsz, ph[is].offset);
    free(ph);
    return 1;
}
/*e: function elf32dotout */

/*s: function elfdotout */
/*
 * Elf binaries.
 */
static int
elfdotout(int fd, Fhdr *fp, ExecHdr *hp)
{
    Ehdr *ep;

    /* bitswap the header according to the DATA format */
    ep = &hp->e;
    if(ep->ident[CLASS] == ELFCLASS32)
        return elf32dotout(fd, fp, hp);
    else if(ep->ident[CLASS] == ELFCLASS64)
        return elf64dotout(fd, fp, hp);

    werrstr("bad ELF class - not 32- nor 64-bit");
    return 0;
}
/*e: function elfdotout */

/*s: function armdotout */
/*
 * (Free|Net)BSD ARM header.
 */
static int
armdotout(int fd, Fhdr *fp, ExecHdr *hp)
{
    uvlong kbase;

    USED(fd);
    settext(fp, hp->e.entry, sizeof(Exec), hp->e.text, sizeof(Exec));
    setdata(fp, fp->txtsz, hp->e.data, fp->txtsz, hp->e.bss);
    setsym(fp, hp->e.syms, hp->e.spsz, hp->e.pcsz, fp->datoff+fp->datsz);

    kbase = 0xF0000000;
    if ((fp->entry & kbase) == kbase) {		/* Boot image */
        fp->txtaddr = kbase+sizeof(Exec);
        fp->name = "ARM *BSD boot image";
        fp->hdrsz = 0;		/* header stripped */
        fp->dataddr = kbase+fp->txtsz;
    }
    return 1;
}
/*e: function armdotout */

/*s: function settext */
static void
settext(Fhdr *fp, uvlong e, uvlong a, long s, vlong off)
{
    fp->txtaddr = a;
    fp->entry = e;
    fp->txtsz = s;
    fp->txtoff = off;
}
/*e: function settext */

/*s: function setdata */
static void
setdata(Fhdr *fp, uvlong a, long s, vlong off, long bss)
{
    fp->dataddr = a;
    fp->datsz = s;
    fp->datoff = off;
    fp->bsssz = bss;
}
/*e: function setdata */

/*s: function setsym */
static void
setsym(Fhdr *fp, long symsz, long sppcsz, long lnpcsz, vlong symoff)
{
    fp->symsz = symsz;
    fp->symoff = symoff;
    fp->sppcsz = sppcsz;
    fp->sppcoff = fp->symoff+fp->symsz;
    fp->lnpcsz = lnpcsz;
    fp->lnpcoff = fp->sppcoff+fp->sppcsz;
}
/*e: function setsym */


/*s: function _round */
static uvlong
_round(uvlong a, ulong b)
{
    uvlong w;

    w = (a/b)*b;
    if (a!=w)
        w += b;
    return(w);
}
/*e: function _round */
/*e: linkers/libmach/executable.c */
