/*s: linkers/8l/elf.h */
#include <elf.h>

// vs include/elf.h? and libmach/elf.h?

/*s: enum _anon_ (linkers/8l/elf.h) */
enum {
    Ehdr32sz	= 52,
    Phdr32sz	= 32,
    Shdr32sz	= 40,

    Ehdr64sz	= 64,
    Phdr64sz	= 56,
    Shdr64sz	= 64,
};
/*e: enum _anon_ (linkers/8l/elf.h) */

typedef void (*Putl)(long);

void	elf32(int mach, int bo, int addpsects, void (*putpsects)(Putl));
/*e: linkers/8l/elf.h */
