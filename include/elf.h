/*s: include/elf.h */

/*s: enum _anon_ */
/* was in /sys/src/libmach/elf.h */
enum {
    /* Ehdr codes */
    MAG0 = 0,		/* ident[] indexes */
    MAG1 = 1,
    MAG2 = 2,
    MAG3 = 3,
    CLASS = 4,
    DATA = 5,
    VERSION = 6,

    ELFCLASSNONE = 0,	/* ident[CLASS] */
    ELFCLASS32 = 1,
    ELFCLASS64 = 2,
    ELFCLASSNUM = 3,

    ELFDATANONE = 0,	/* ident[DATA] */
    ELFDATA2LSB = 1,
    ELFDATA2MSB = 2,
    ELFDATANUM = 3,

    NOETYPE = 0,		/* type */
    REL = 1,
    EXEC = 2,
    DYN = 3,
    CORE = 4,

    NONE = 0,		/* machine */
    M32 = 1,		/* AT&T WE 32100 */
    SPARC = 2,		/* Sun SPARC */
    I386 = 3,		/* Intel 80386 */
    M68K = 4,		/* Motorola 68000 */
    M88K = 5,		/* Motorola 88000 */
    I486 = 6,		/* Intel 80486 */
    I860 = 7,		/* Intel i860 */
    MIPS = 8,		/* Mips R2000 */
    S370 = 9,		/* Amdhal	*/
    MIPSR4K = 10,	/* Mips R4000 */
    SPARC64 = 18,	/* Sun SPARC v9 */
    POWER = 20,		/* PowerPC */
    POWER64 = 21,	/* PowerPC64 */
    ARM = 40,		/* ARM */
    AMD64 = 62,		/* Amd64 */
    ARM64 = 183,	/* ARM64 */

    NO_VERSION = 0,		/* version, ident[VERSION] */
    CURRENT = 1,

    /* Phdr Codes */
    NOPTYPE = 0,		/* type */
    PT_LOAD = 1, /* also LOAD */
    DYNAMIC = 2,
    INTERP = 3,
    NOTE = 4,
    SHLIB = 5,
    PHDR = 6,

    R = 0x4,		/* flags */
    W = 0x2,
    X = 0x1,

    /* Shdr Codes */
    Progbits = 1,	/* section types */
    Strtab = 3,
    Nobits = 8,

    SwriteElf = 1,	/* section attributes (flags) */
    Salloc = 2,
    Sexec = 4,
};
/*e: enum _anon_ */


/*s: struct Ehdr */
/*
 *	Definitions needed for accessing ELF headers
 */
struct Ehdr {
    uchar	ident[16];	/* ident bytes */
    ushort	type;		/* file type */
    ushort	machine;	/* target machine */
    int	version;	/* file version */
    ulong	elfentry;	/* start address */
    ulong	phoff;		/* phdr file offset */
    ulong	shoff;		/* shdr file offset */
    int	flags;		/* file flags */
    ushort	ehsize;		/* sizeof ehdr */
    ushort	phentsize;	/* sizeof phdr */
    ushort	phnum;		/* number phdrs */
    ushort	shentsize;	/* sizeof shdr */
    ushort	shnum;		/* number shdrs */
    ushort	shstrndx;	/* shdr string index */
};
/*e: struct Ehdr */
typedef struct Ehdr Ehdr;

/*s: struct Phdr */
struct Phdr {
    int	type;		/* entry type */
    ulong	offset;		/* file offset */
    ulong	vaddr;		/* virtual address */
    ulong	paddr;		/* physical address */
    int	filesz;		/* file size */
    ulong	memsz;		/* memory size */
    int	flags;		/* entry flags */
    int	align;		/* memory/file alignment */
};
/*e: struct Phdr */
typedef struct Phdr Phdr;

/*s: struct Shdr */
struct Shdr {
    ulong	name;		/* section name */
    ulong	type;		/* SHT_... */
    ulong	flags;		/* SHF_... */
    ulong	addr;		/* virtual address */
    ulong	offset;		/* file offset */
    ulong	size;		/* section size */
    ulong	link;		/* misc info */
    ulong	info;		/* misc info */
    ulong	addralign;	/* memory alignment */
    ulong	entsize;	/* entry size if table */
};
/*e: struct Shdr */
typedef struct Shdr Shdr;

/*s: constant ELF_MAG */
#define	ELF_MAG		((0x7f<<24) | ('E'<<16) | ('L'<<8) | 'F')
/*e: constant ELF_MAG */
/*e: include/elf.h */
