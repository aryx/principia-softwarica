#include <elf.h>

enum {
	Ehdr32sz	= 52,
	Phdr32sz	= 32,
	Shdr32sz	= 40,

	Ehdr64sz	= 64,
	Phdr64sz	= 56,
	Shdr64sz	= 64,
};


typedef void (*Putl)(long);

void	elf32(int mach, int bo, int addpsects, void (*putpsects)(Putl));
void	elf32phdr(void (*putl)(long), ulong type, ulong off, ulong vaddr,
	ulong paddr, ulong filesz, ulong memsz, ulong prots, ulong align);
void	elf32shdr(void (*putl)(long), ulong name, ulong type, ulong flags,
	ulong vaddr, ulong off, ulong sectsz, ulong link, ulong addnl,
	ulong align, ulong entsz);

void	elf64(int mach, int bo, int addpsects, void (*putpsects)(Putl));
void	elf64phdr(void (*putl)(long), void (*putll)(vlong), ulong type,
	uvlong off, uvlong vaddr, uvlong paddr, uvlong filesz, uvlong memsz,
	ulong prots, uvlong align);
void	elf64shdr(void (*putl)(long), void (*putll)(vlong), ulong name,
	ulong type, uvlong flags, uvlong vaddr, uvlong off, uvlong sectsz,
	ulong link, ulong addnl, uvlong align, uvlong entsz);
