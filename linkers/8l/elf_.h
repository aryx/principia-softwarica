/*s: 8l/elf_.h */
#include <exec/elf.h>

// vs include/elf.h? and libmach/elf.h?

/*s: enum ElfHeaderSizes */
enum {
    Ehdr32sz	= 52,
    Phdr32sz	= 32,
    Shdr32sz	= 40,

    Ehdr64sz	= 64,
    Phdr64sz	= 56,
    Shdr64sz	= 64,
};
/*e: enum ElfHeaderSizes */

typedef void (*Putl)(int32);

void	elf32(int mach, int bo, int addpsects, void (*putpsects)(Putl));
/*e: 8l/elf_.h */
