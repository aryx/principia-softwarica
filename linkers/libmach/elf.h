#include <elf.h>

#define LOAD PT_LOAD

/*
 *	Definitions needed for accessing ELF headers
 */
typedef struct {
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
} Ehdr;

typedef struct {
	u8int	ident[16];	/* ident bytes */
	u16int	type;		/* file type */
	u16int	machine;	/* target machine */
	u32int	version;	/* file version */
	u64int	elfentry;	/* start address */
	u64int	phoff;		/* phdr file offset */
	u64int	shoff;		/* shdr file offset */
	u32int	flags;		/* file flags */
	u16int	ehsize;		/* sizeof ehdr */
	u16int	phentsize;	/* sizeof phdr */
	u16int	phnum;		/* number phdrs */
	u16int	shentsize;	/* sizeof shdr */
	u16int	shnum;		/* number shdrs */
	u16int	shstrndx;	/* shdr string index */
} E64hdr;

typedef struct {
	int	type;		/* entry type */
	ulong	offset;		/* file offset */
	ulong	vaddr;		/* virtual address */
	ulong	paddr;		/* physical address */
	int	filesz;		/* file size */
	ulong	memsz;		/* memory size */
	int	flags;		/* entry flags */
	int	align;		/* memory/file alignment */
} Phdr;

typedef struct {
	u32int	type;		/* entry type */
	u32int	flags;		/* entry flags */
	u64int	offset;		/* file offset */
	u64int	vaddr;		/* virtual address */
	u64int	paddr;		/* physical address */
	u64int	filesz;		/* file size */
	u64int	memsz;		/* memory size */
	u64int	align;		/* memory/file alignment */
} P64hdr;

typedef struct {
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
} Shdr;

typedef struct {
	u32int	name;		/* section name */
	u32int	type;		/* SHT_... */
	u64int	flags;		/* SHF_... */
	u64int	addr;		/* virtual address */
	u64int	offset;		/* file offset */
	u64int	size;		/* section size */
	u32int	link;		/* misc info */
	u32int	info;		/* misc info */
	u64int	addralign;	/* memory alignment */
	u64int	entsize;	/* entry size if table */
} S64hdr;

#define	ELF_MAG		((0x7f<<24) | ('E'<<16) | ('L'<<8) | 'F')
